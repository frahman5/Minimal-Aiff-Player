/* == A minimal AIFF player that does NOT use an external library to 
      parse the AIFF file, and uses libportaudio to stream the file
      to audio devices == */
#include <stdlib.h>         // malloc
#include "playIND.h"

#define FRAMES_PER_BUFFER       1024

/* play the AIFF referenced by argv[1]
     Usage: play musicFile.aiff */
// int main(int argc, char *argv[]) {
//     /* portaudio and AIFF parameters */
//     PaError err;
//     AIFF_Ref song;
    
//     /* Get song data */
//     int *soundSamples;
//     uint64_t numSamples;
//     const char *songFile = argv[1];
//     if (getAudioData(songFile, &song, &soundSamples, &numSamples) < 0)
//     {
//         fprintf(stderr, "Could not get audio data from %s\n", songFile);
//         abort();
//     }
//     printf("We got audio data succesfully\n");

//     /* Stream song to default audio device */
//     if ((err = streamSong(soundSamples, numSamples)) < 0) handlePaError(err);

//     /* Clean up AIFF references and soundSamples */
//     cleanSong(soundSamples, &song);
    
//     return 0;
// }

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
        case IFF_TYPE_FORM:
        // case 0x464F524D:                // "FORM", keep reading
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

            // Store the common chunk info into the songRef
            /* TO DO */
            break;
        default:
            printf("Header id %08x is not FORM\n", hdr.hid);
            fclose(songRef->f);
            free(songRef);
            return NULL;
        // case FORM
        // case LIST or CAT or other
    }

    return songRef;

}
/* Get song metadata into song, waveform data into *sampleArray, and the number
   of sample frames into nSampleFrames. Do so without using libaiff */
int getAudioDataIND(const char *file, AIFF_Ref_Basic *song, 
                    int **sampleArray, uint64_t *nSampleFrames) {
    return 0;

}