/* == A minimal AIFF player that does NOT use an external library to 
      parse the AIFF file, and uses libportaudio to stream the file
      to audio devices. It does not play AIFFs. It is designed only to
      play the accompanying AIFF files ByYourSidePt1.aiff, 
      ByYourSidePt2.aiff on a little-endian architecture== */
#include <assert.h>
#include <stdlib.h>         // malloc
#include <math.h>
#include "playIND.h"

#define FRAMES_PER_BUFFER       1024

/* play the AIFF referenced by argv[1]
     Usage: play musicFile.aiff */
int main(int argc, char *argv[]) {
    PaError err;
    AIFF_Ref_Basic *songRef;
    
    /* Get song data */
    int *soundSamples;
    uint64_t numSamples;
    const char *songFile = argv[1];
    if (getAudioDataIND(songFile, songRef, &soundSamples, &numSamples) < 0)
    {
        fprintf(stderr, "Could not get audio data from %s\n", songFile);
        abort();
    }

    /* Stream song to default audio device */
    if ((err = streamSong(soundSamples, numSamples)) < 0) handlePaError(err);

    printf("Finished streaming song\n");

    /* Clean up AIFF references and soundSamples */
    cleanSongIND(soundSamples, songRef);
    
    return 1;
}

AIFF_Ref_Basic *openAIFF_File(const char *file) {
    AIFF_Ref_Basic *songRef;     // reference data structure
    IFFHeader hdr;               // create a struct that holds header info 
                                     // (header id, file id, length of header)

    // ask for space on the heap for a song reference
    songRef = malloc(sizeof(AIFF_Ref_Basic));   
    if (!songRef) {
        printf("Allocation of songRef on heap failed\n");
        free(songRef);
        return NULL;
    }

    // open the file and have the reference point to it
    songRef->f = fopen(file, "rb");     // open the file in binary mode
    if (songRef->f == NULL) {
        printf("Opening song file %s failed on heap failed\n", file);
        free(songRef);
        return NULL;
    }

    // read in header id
    if (fread(&hdr, 1, 4, songRef->f) < 4) {
        printf("Reading in header id failed\n");
        fclose(songRef->f);
        free(songRef);
        return NULL;
    }

    // If the header id is FORM, keep calm and carry on. Else return NULL
    switch (hdr.hid) {
        case IFF_TYPE_FORM:                 // case FORM, keep reading.
            // read in file length and file id
            if (fread(&(hdr.len), 1, 8, songRef->f) < 8) {
                printf("Reading in length and file id failed\n");
                fclose(songRef->f);
                free(songRef);
                return NULL;
            }  
            // if song length is 0, fuck it all
            if (hdr.len == 0) {
                printf("Song length is 0. Fuck it\n");
                fclose(songRef->f);
                free(songRef);
                return NULL;
            }

            // Check the format type (make sure its AIFF or AIFC);
            songRef->format = hdr.fid;        // storm file format type in reference
            switch (songRef->format) {
                case AIFF_TYPE_AIFF:   
                case AIFF_TYPE_AIFC:
                    break;
                default:                        // not good! abort!
                    printf("song ref format %08x is not AIFF or AIFC\n", 
                        songRef->format);
                    fclose(songRef->f);
                    free(songRef);
                    return NULL;
            }

            // Store the common chunk info into the songRef. Figure out file format (LPCM? Other?)
            if (init_ref_from_common_chunk(songRef) < 1) {     
                printf("Reading in common chunk failed\n");    
                fclose(songRef->f);
                free(songRef);
                return NULL;
            }
            break;
        default:                               // case LIST or CAT or other
            printf("Header id %08x is not FORM\n", hdr.hid);
            fclose(songRef->f);
            free(songRef);
            return NULL;
        
    }

    return songRef;
}

/*
 * Read IEEE Extended Precision Numbers
 */
double ieee754_read_extendedIND(uint8_t* in) {
    int sgn, exp;
    uint32_t low, high;
    double out;

    /* Extract the components from the big endian buffer */
    sgn = (int) (in[0] >> 7);
    exp = ((int) (in[0] & 0x7F) << 8) | ((int) in[1]);
    low = (((uint32_t) in[2]) << 24)
        | (((uint32_t) in[3]) << 16)
        | (((uint32_t) in[4]) << 8) | (uint32_t) in[5];
    high = (((uint32_t) in[6]) << 24)
        | (((uint32_t) in[7]) << 16)
        | (((uint32_t) in[8]) << 8) | (uint32_t) in[9];

    if (exp == 0 && low == 0 && high == 0)
        return (sgn ? -0.0 : 0.0);

    switch (exp) {
    case 32767:
        if (low == 0 && high == 0)
            return (sgn ? -INFINITE_VALUE : INFINITE_VALUE);
        else
            return (sgn ? -NAN_VALUE : NAN_VALUE);
    default:
        exp -= 16383;   /* unbias exponent */

    }

    out = ldexp((double) low, -31 + exp);
    out += ldexp((double) high, -63 + exp);

    return (sgn ? -out : out);
}

int init_ref_from_common_chunk(AIFF_Ref_Basic *songRef) {

    uint32_t len, numSampleFrames;
    uint16_t bitDepth, numChannels;
    uint8_t sampleRateBuffer[10];

    /* Find the COMM chunk, store length in len and seek r->fd to the given chunk */
    if (!find_comm_or_ssnd_chunk(AIFF_TYPE_COMM, songRef, &len)){
        printf("Could not find common chunk\n");
        return -1;
    }    

    /* Length should be >= 18 */
    assert (len >= 18);

    /* Put common chunk metadata into songRef */
    if (fread(&numChannels, 1, 2, songRef->f) < 2 ||
        fread(&numSampleFrames, 1, 4, songRef->f) < 4 || 
        fread(&bitDepth, 1, 2, songRef->f) < 2 ||
        fread(sampleRateBuffer, 1, 10, songRef->f) < 10) {
        printf("could not read in common chunk attributes\n");
        return -1;
    }

    songRef->numChannels = (int) ARRANGE_LE16(numChannels);
    songRef->numSampleFrames = ARRANGE_LE32(numSampleFrames);
    songRef->bitDepth = (int) ARRANGE_LE16(bitDepth);
    songRef->segmentSize = (int) ceil((songRef->bitDepth)/8);
    songRef->sampleRate = ieee754_read_extendedIND(sampleRateBuffer);

    return 1;

}

/* Dumb Version */
int find_comm_or_ssnd_chunk(IFFType chunk, AIFF_Ref_Basic *songRef, uint32_t *length) {

    IFFChunk data;                   

    /* Make sure chunk is comm or ssnd */
    assert (chunk == AIFF_TYPE_COMM | chunk == AIFF_TYPE_SSND);

    /* Scroll the file back to the beginning */
    if (fseek(songRef->f, 0, SEEK_SET) != 0) { return 0; }

    /* Scroll through file reading 8 bytes at a time looking for chunk ID */
    while (1) {
        if (fread(&data, 1, 8, songRef->f) < 8) { return 0; }
        fseek(songRef->f, -6, SEEK_CUR);        // Gotta read starting every 2 bytes
        if (data.id == chunk) {
            fseek(songRef->f, 6, SEEK_CUR);
            *length = ARRANGE_LE32(data.len);
            return 1;
        }
    }

    /* Not found */
    return 0;

}
int getAudioDataIND(const char *file, AIFF_Ref_Basic *songRef, 
                    int **sampleArray, uint64_t *nSampleFrames) {

    /* Open file, store file pointer into song, as well as metadata about song, 
       (common chunk info and header info) */
    songRef = openAIFF_File(file);
    if (!songRef) 
    {
        fprintf(stderr, "Failed to open file %s\n", file);
        closeAIFF_File(songRef);
        return -1;
    }

    /* Extract audio samples */
       // each frame has numChannels samples; each sample is 4 bytes long
    int numBytes = (songRef->numSampleFrames * songRef->numChannels * 4); 
    *sampleArray = (int *) malloc(numBytes);
    if (!*sampleArray) {
        fprintf( stderr, "soundSamples could not be allocated\n"); 
        return -1;
    }
    readSamples32Bit(songRef, sampleArray, songRef->numSampleFrames * songRef->numChannels);

    /* Let the caller know how many sample frames we have */
    *nSampleFrames = songRef->numSampleFrames;

    return 1;

}

int cleanSongIND(int *soundSamples, AIFF_Ref_Basic *songRef) {
    closeAIFF_File(songRef);
    printf("closed AIFF file succesffuly\n");
    free(soundSamples);
    return 1;
}

int closeAIFF_File(AIFF_Ref_Basic *songRef) {
    printf("entered closeAIFF_File\n");
    if (songRef->f) {
        printf("Attempting to close songRef\n");
        fclose(songRef->f);
    }
    printf("closed song file succesffuly\n");
    free(songRef);
    printf("freed songRef space successfully\n");
    return 1;
}

int readSamples32Bit(AIFF_Ref_Basic *songRef, int **sampleArray, int numSamples) {
    uint32_t len;
    uint32_t offset, blockSize;

    /* Locate SSND chunk */
    if (!find_comm_or_ssnd_chunk(AIFF_TYPE_SSND, songRef, &len)){
        printf("Could not find sound data chunk\n");
        return -1;
    }    

    /* Read block alignment stuff and act appropriately */
    if (fread(&offset, 1, 4, songRef->f) < 4 ||
        fread(&blockSize, 1, 4, songRef->f) < 4) {
        printf("Could not read offset or blockSize\n");
        return -1;
    }
    assert (offset == 0);
    assert (blockSize == 0);

    /* store samples in sampleArray */
    uint32_t byteDepth = (songRef->bitDepth) / 8;

    for (int i = 0; i < numSamples; i++) {
        // printf("Iter %d\n", i);
        if (fread(&(*sampleArray)[i], 1, byteDepth, songRef->f) < byteDepth) {
            printf("Could not read a soundWave data at sample %d\n", i);
            return -1;
        }
        (*sampleArray)[i] = ARRANGE_LE32((int) (*sampleArray)[i]);
    }
    return 1;
}