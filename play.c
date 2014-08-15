#define LIBAIFF_NOCOMPAT 1      // do not use LibAiff 2 API compatability
#include <libaiff/libaiff.h>    // for parsing AIFF
#include "portaudio.h"          // for sending sound samples to sound card
#include <math.h>               // pow

#define FRAMES_PER_BUFFER       1024

/* handle errors produced by port audio functions */
int handlePaError(PaError err); 

/* play the AIFF referenced by argv[1]
     Usage: play musicFile.aiff */
int main(int argc, char *argv[]) {

    /* portaudio and libaiff parameters */
    PaError err;
    PaStream *stream;
    int buffer[FRAMES_PER_BUFFER][2];       // stereo output buffer
    AIFF_Ref song;

    /* Open the songfile */
    const char *songFile = argv[1];
    song = AIFF_OpenFile(songFile, F_RDONLY);
    if (!song) 
    {
        fprintf( stderr, "Failed to open file %s\n", songFile);
        AIFF_CloseFile(song);
        abort();
    }
    printf("Opened file succesfully\n");

    /* Get sound format info */
    uint64_t seconds = 0;
    uint64_t nSamples = 0;
    double samplingRate = 0;
    int channels = 0;
    int bitsPerSample = 0;
    int segmentSize = 0;
    if (AIFF_GetAudioFormat(song, &nSamples, &channels, &samplingRate, &bitsPerSample, &segmentSize) < 1) 
    {
        fprintf( stderr, "Could not get audio format information\n");
        AIFF_CloseFile(song);
        abort();
    }

    /* Open a stream to play the song */
    err = Pa_Initialize();
    if ( err != paNoError ) { handlePaError(err); }
    err = Pa_OpenDefaultStream(
            &stream,                            // stream
            0,                                  // numInputChannels. 0->output only stream
            2,                                  // numOutputChannels
            paInt32,                            // sample format (song is read in as 32 bit ints)
            44100,                              // sample rate in hertz,
            0,                                  // preferred granularity for blocking read/write stream. 0->let portaudio optimize this 
            NULL,                               // pointer to asynchronous callback function for filling output buffer,
            NULL);                              // pointer passed to callback function
    if ( err != paNoError ) { handlePaError(err); }
    printf("Opened audio stream successfully\n");        
    

    /* Extract audio samples */
    int numBytes = (2 * nSamples * 4);          // each sample is 4 bytes, AIFF calls a frame a "sample", so we mult by 2
    int *soundSamples = (int *) malloc(2 * numBytes);
    if (!soundSamples) 
    {
        fprintf( stderr, "soundSamples could not be allocated\n"); abort();
    }
    AIFF_ReadSamples32Bit(song, soundSamples, 2 * nSamples);

    /* Play the song */
    int loopOffset;
    int frameOffset;
    int loopLimit = nSamples / FRAMES_PER_BUFFER;   // on each loop, we go through FRAMES_PER_BUFFER "samples" as defined by AIFF
    err = Pa_StartStream(stream);
    if ( err != paNoError ) { handlePaError(err); }

    for (int i = 0; i < loopLimit; i++) {
        // On the ith iteration, we've already played loopOffset samples
        loopOffset = (i * 2 * FRAMES_PER_BUFFER); 
        for (int j = 0; j < FRAMES_PER_BUFFER; j++) // Each iteration, load another frame into the buffer
        {
            frameOffset = 2 * j;                    // On jth iteration, we've alreayd read frameOffset frames
            buffer[j][0] = *(soundSamples + loopOffset + frameOffset);
            buffer[j][1] = *(soundSamples + loopOffset + frameOffset + 1);
        }
        err = Pa_WriteStream(stream, buffer, FRAMES_PER_BUFFER);
        if ( err != paNoError ) { handlePaError(err); }
    }
    /* Clean up (Stop stream, close stream, free soundSamples) */
    free(soundSamples);                                 // Free soundSamples
    AIFF_CloseFile(song);                               // Close song file
    if ((err = Pa_StopStream(stream)) != paNoError)     // Stop the stream
        handlePaError(err); 
    if ( (err = Pa_CloseStream(stream)) != paNoError )  // Close the stream
        handlePaError(err);
    if ( (err = Pa_Terminate()) != paNoError )          // Terminate port audio
        handlePaError(err);
    
    /* Print song info */
    seconds = nSamples / samplingRate;
    printf("Song Length: %lu:%02lu:%02lu \n", (unsigned long) seconds/360, 
               (unsigned long) (seconds/60)%60, (unsigned long) seconds%60 );
    printf("Sampling Rate: %f\n", samplingRate);
    printf("Number of Channels: %d\n", channels);
    printf("bit depth: %i\n", bitsPerSample);
    printf("Num samples: %llu\n", nSamples);
    printf("FRAMES_PER_BUFFER: %d\n", FRAMES_PER_BUFFER);
    printf("loopLimit: %d\n", loopLimit);
    printf("loopLimit as float: %f\n", nSamples / (double) FRAMES_PER_BUFFER);
    
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