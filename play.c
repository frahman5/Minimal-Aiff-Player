#define LIBAIFF_NOCOMPAT 1      // do not use LibAiff 2 API compatability
#include <libaiff/libaiff.h>    // for parsing AIFF
#include "portaudio.h"          // for sending sound samples to sound card
#include <math.h>               // pow

#define FRAMES_PER_BUFFER       1024

/* handle errors produced by port audio functions */
int handlePaError(PaError err); 

/* play the AIFF referenced by r 
     Usage: play musicFile.aif */
int main(int argc, char *argv[]) {
    /* portaudio and libaiff parameters */
    PaError err;
    PaStream *stream;
    int buffer[FRAMES_PER_BUFFER][2];       // stereo output buffer
    AIFF_Ref song;

    /* Get songFile */
    const char *songFile = argv[1];
    
    /* Attempt to open the song file */
    song = AIFF_OpenFile(songFile, F_RDONLY);
    if (!song) {
        printf("Failed to open file %s\n", songFile);
        AIFF_CloseFile(song);
        abort();
    }
    printf("Opened file succesfully\n");

    /* Print sound format info */
    uint64_t seconds = 0;
    uint64_t nSamples = 0;
    int channels = 0;
    double samplingRate = 0;
    int bitsPerSample = 0;
    int segmentSize = 0;

    if (AIFF_GetAudioFormat(song, &nSamples, &channels, &samplingRate, &bitsPerSample, &segmentSize) < 1) {
        printf("Could not get audio format information");
        AIFF_CloseFile(song);
    }

    /* Fuck around with PortAudio */
    // int error;                          // for capturing Pa Errors
    // printf("Your machine has %d audio APIs\n", Pa_GetHostApiCount());
    // const struct PaHostApiInfo *ha = Pa_GetHostApiInfo( Pa_GetDefaultHostApi() );
    // const struct PaDeviceInfo *d = Pa_GetDeviceInfo( Pa_GetDefaultOutputDevice() );
    // printf("Your default input device is %s\n", d->name);
    // printf("Device info: \n");
    // printf("    max Input Channels: %d\n", d->maxInputChannels);
    // printf("    max Output Channels: %d\n", d->maxOutputChannels);
    // printf("    default Sample Rate %f\n", d->defaultSampleRate);

    /* Open a stream to play the song */
    err = Pa_Initialize();
    if ( err != paNoError ) { handlePaError(err); }

    err = Pa_OpenDefaultStream(
                         &stream,           // stream
                         0,                 // numInputChannels. 0->output only stream
                         2,                 // numOutputChannels
                         paInt32,           // sample format (song has 16 bit depth but gets read in as 32 bit ints)
                         44100,             // sample rate in hertz,
                         0,                 // preferred granularity for blocking read/write stream. 0->let portaudio optimize this 
                         NULL,              // pointer to asynchronous callback function for filling output buffer,
                         NULL);             // pointer passed to callback function
    if ( err != paNoError ) { handlePaError(err); }
    printf("Opened audio stream successfully\n");        
    
    /* Play the song */
    err = Pa_StartStream( stream );
    if ( err != paNoError ) { handlePaError(err); }
    printf("Playing song\n");

    int numBytes = (nSamples * 4);          // ReadSamples returns samples in 32 bit ints
    printf("Length of soundSamples in bytes: %d\n", numBytes);
    printf("num Samples: %llu\n", nSamples);
    int *soundSamples = (int *) malloc(2 * numBytes);
    if (!soundSamples) {
        fprintf( stderr, "soundSamples could not be allocated\n");
        abort();
    }
    AIFF_ReadSamples32Bit(song, soundSamples, 2 * nSamples);

    int bufferOffset;
    int loopLimit = nSamples / (FRAMES_PER_BUFFER);
    for (int i = 0; i < loopLimit; i++) {
        printf("we've gone through 1024 frames %d of %d times\n", i, loopLimit);
        // On the ith iteration, we've already played bufferOffset samples
        bufferOffset = (i * 2 * FRAMES_PER_BUFFER);  
        printf("buffer offset: %d\n", bufferOffset);
        for (int j = 0; j < FRAMES_PER_BUFFER; ) {
            // printf("j: %d\n", j);
            buffer[j][0] = *(soundSamples + bufferOffset + (2 * j) );
            buffer[j][1] = *(soundSamples + bufferOffset + (2 * j) + 1);
            j++;
        }
        err = Pa_WriteStream( stream, buffer, FRAMES_PER_BUFFER );
        if ( err != paNoError ) { handlePaError(err); }
    }

    err = Pa_StopStream( stream );
    if ( err != paNoError ) { handlePaError(err); }

    /* Clean up the soundSamples array */
    free(soundSamples);

    seconds = nSamples / samplingRate;
    printf("Song Length: %lu:%02lu:%02lu \n", (unsigned long) seconds/360, 
               (unsigned long) (seconds/60)%60, (unsigned long) seconds%60 );
    printf("Sampling Rate: %f\n", samplingRate);
    printf("Number of Channels: %d\n", channels);
    printf("bit depth: %i\n", bitsPerSample);
    printf("Num samples: %llu\n", nSamples);
    printf("FRAMES_PER_BUFFER: %d\n", FRAMES_PER_BUFFER);
    printf("loopLimit: %d\n", loopLimit);
    printf("loopLimit as float: %f\n", nSamples / (double) (2 * FRAMES_PER_BUFFER));

    /* Close the stream once we're done */
    err = Pa_CloseStream(stream);        
    if ( err != paNoError ) { handlePaError(err); } 
    printf("Successfully closed stream\n");

    /* Clean up shop */
    err = Pa_Terminate();
    if ( err != paNoError ) { handlePaError(err); }
    AIFF_CloseFile(song);               // Close song file
    
    return 0;
}

int handlePaError(PaError err) {
    fprintf( stderr, "An Error occured while using the portaudio stream\n");
    fprintf( stderr, "Error Number: %d\n", err);
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ));
        // Print more info on the error
    if (err == paUnanticipatedHostError) {
        const PaHostErrorInfo *hostErrorInfo = Pa_GetLastHostErrorInfo();
        fprintf( stderr, "Host API error = #%ld, hostApiType = %d\n", hostErrorInfo->errorCode, hostErrorInfo->hostApiType);
        fprintf( stderr, "Host API error = %s\n", hostErrorInfo->errorText);
    }
    Pa_Terminate();
    abort();
}