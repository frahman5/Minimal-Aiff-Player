/* == A minimal AIFF player that does NOT use an external library to 
      parse the AIFF file, and uses libportaudio to stream the file
      to audio devices == */

#define LIBAIFF_NOCOMPAT 1      // do not use LibAiff 2 API compatability
#include <math.h>               // pow
#include "play.h"

#define FRAMES_PER_BUFFER       1024

/* play the AIFF referenced by argv[1]
     Usage: play musicFile.aiff */
int main(int argc, char *argv[]) {
    /* portaudio and AIFF parameters */
    PaError err;
    AIFF_Ref song;
    
    /* Get song data */
    int *soundSamples;
    uint64_t numSamples;
    const char *songFile = argv[1];
    if (getAudioData(songFile, &song, &soundSamples, &numSamples) < 0)
    {
        fprintf(stderr, "Could not get audio data from %s\n", songFile);
        abort();
    }
    printf("We got audio data succesfully\n");

    /* Stream song to default audio device */
    if ((err = streamSong(soundSamples, numSamples)) < 0) handlePaError(err);

    /* Clean up AIFF references and soundSamples */
    cleanSong(soundSamples, &song);
    
    return 0;
}

AIFF_Ref_Basic *openAIFF_File(const char *file) {
    AIFF_Ref_Basic *songRef;     // reference data structure
    IFFHeader hdr;               // create a struct that holds header info 
                                     // (header id, file id, length of header)

    // ask for space on the heap for a song reference
    songRef = malloc(sizeof(AIFF_Ref_Basic));   
    if (!songRef) {
        free(songRef);
        return NULL;
    }

    // open the file and have the reference point to it
    songRef->f = fopen(file, "rb");     // open the file in binary mode
    if (songRef->f == NULL) {
        free(songRef);
        return NULL;
    }

    // read in header id
    if (fread(&hdr, 1, 4, songRef->f) < 4) {
        fclose(songRef->f);
        free(songRef);
        return NULL;
    }

    // If the header id is FORM, keep calm and carry on. Else return NULL
    switch (hdr.hid) {
        case 0x464F524D:                // "FORM", keep reading
            // read in file length and file id
            if (fread(&(hdr.len), 1, 8, songRef->f) < 8) {
                fclose(songRef->f);
                free(songRef);
                return NULL;
            }  
            // if song length is 0, fuck it all
            if (hdr.len == 0) {
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
                    fclose(songRef->f);
                    free(songRef);
                    return NULL;
            }

            // Store the common chunk info into the songRef
            /* TO DO */
            break;
        default:
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
int getAudioData(const char *file, AIFF_Ref *song, int **sampleArray, uint64_t *nSamples) {
    /* Open file, store file pointer into song, as well as matadata about song, including
       common chunk info and header info */
    *song = AIFF_OpenFile(file, F_RDONLY);
    if (!*song) 
    {
        fprintf( stderr, "Failed to open file %s\n", file);
        AIFF_CloseFile(*song);
        return -1;
    }
    printf("Song format: %08x\n", (*song)->format);
    printf("Song audio format: %08x\n", (*song)->audioFormat);

    /* Get sound format info */
    uint64_t seconds;
    double samplingRate;
    int channels, bitsPerSample, segmentSize;
    if (AIFF_GetAudioFormat(*song, nSamples, &channels, &samplingRate, 
                            &bitsPerSample, &segmentSize) < 1) 
    {
        fprintf( stderr, "Could not get audio format information\n");
        AIFF_CloseFile(*song);
        return -1;
    }

    /* Extract audio samples */
    int numBytes = (*nSamples * 4 * 2);          // each sample is 4 bytes, AIFF calls a frame a "sample", so we mult by 2
    *sampleArray = (int *) malloc(numBytes);
    if (!*sampleArray) {
        fprintf( stderr, "soundSamples could not be allocated\n"); 
        return -1;
    }
    AIFF_ReadSamples32Bit(*song, *sampleArray, *nSamples * 2);

    return 0;
}

int streamSong(int *soundSamples, int numSamples) {
    printf("numSampleFrames: %d\n", numSamples);
    PaError err;
    PaStream *stream;
    int buffer[FRAMES_PER_BUFFER][2];           // stereo output buffer

    /* Open a stream to play the song */
    if ((err = Pa_Initialize()) != paNoError ) return err;
    err = Pa_OpenDefaultStream(
            &stream,                            // stream
            0,                                  // numInputChannels. 0->output only stream
            2,                                  // numOutputChannels
            paInt32,                            // sample format (song is read in as 32 bit ints)
            44100,                              // sample rate in hertz,
            0,                                  // preferred granularity for blocking read/write stream. 0->let portaudio optimize this 
            NULL,                               // pointer to asynchronous callback function for filling output buffer,
            NULL);                              // pointer passed to callback function
    if (err != paNoError) return err;      

    /* Play the song */
    int loopOffset;
    int frameOffset;
    int loopLimit = numSamples / FRAMES_PER_BUFFER;     // on each loop, we go through FRAMES_PER_BUFFER "samples" as defined by AIFF
    int samplesLeft = numSamples * 2;                   // recall an AIFF sample is actually a frame
    int numFrames;                                      // number of frames to write to stream
    if ((err = Pa_StartStream(stream)) != paNoError) return err;

    int counter = 0;
    while (samplesLeft != 0) 
    {
        numFrames = (samplesLeft > FRAMES_PER_BUFFER) ? FRAMES_PER_BUFFER : (samplesLeft / 2);
        loopOffset = (counter++ * 2 * numFrames); 
        samplesLeft -= (numFrames * 2);

        for (int j = 0; j < numFrames; j++) // Each iteration, load another frame into the buffer
        {
            frameOffset = 2 * j;            // On jth iteration, we've already read frameOffset frames
            buffer[j][0] = *(soundSamples + loopOffset + frameOffset);
            buffer[j][1] = *(soundSamples + loopOffset + frameOffset + 1);
        }
        err = Pa_WriteStream(stream, buffer, numFrames);
        if (err != paNoError) return err;

    }
    
    /* Clean up shop */
    if ( ((err = Pa_StopStream(stream)) != paNoError)  ||    // Stop the stream
         ((err = Pa_CloseStream(stream)) != paNoError) ||    // Close the stream
         ((err = Pa_Terminate()) != paNoError) )             // Terminate port audio
        return err; 

    return 0;

}
int cleanSong(int *soundSamples, AIFF_Ref *song) {
    free(soundSamples);                                 // Free soundSamples
    AIFF_CloseFile(*song);                              // Close song file  
    return 0;  
}

int handlePaError(PaError err) {
    fprintf(stderr, "An Error occured while using the portaudio stream\n");
    fprintf(stderr, "Error Number: %d\n", err);
    fprintf(stderr, "Error message: %s\n", Pa_GetErrorText( err ));
    if (err == paUnanticipatedHostError) 
    {
        const PaHostErrorInfo *hostErrorInfo = Pa_GetLastHostErrorInfo();
        fprintf(stderr, "Host API error = #%ld, hostApiType = %d\n", hostErrorInfo->errorCode, hostErrorInfo->hostApiType);
        fprintf(stderr, "Host API error = %s\n", hostErrorInfo->errorText);
    }
    Pa_Terminate();
    abort();
}