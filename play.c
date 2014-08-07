#define LIBAIFF_NOCOMPAT 1      // do not use LibAiff 2 API compatability
#include <libaiff/libaiff.h>    // for parsing AIFF
#include "portaudio.h"          // for sending sound samples to sound card

/* play the AIFF referenced by r 
     Usage: play musicFile.aif */
int main(int argc, char *argv[]) {
    const char *songName = argv[1];

    AIFF_Ref song;

    /* Attempt to open the file */
    song = AIFF_OpenFile(songName, F_RDONLY);
    if (!song) {
        printf("Failed to open file %s\n", songName);
        AIFF_CloseFile(song);
        abort();
    }
    printf("Opened file succesfully\n");

    /* Print sound format info */
    uint64_t seconds, nSamples;
    int channels;
    double samplingRate;
    int bitsPerSample;
    int segmentSize;

    if (AIFF_GetAudioFormat(song, &nSamples, &channels, &samplingRate, &bitsPerSample, &segmentSize) < 1) {
        printf("Could not get audio format information");
        AIFF_CloseFile(song);
    }
    seconds = nSamples / samplingRate;
    printf("Song Length: %lu:%02lu:%02lu \n", (unsigned long) seconds/360, 
               (unsigned long) (seconds/60)%60, (unsigned long) seconds%60 );
    printf("Sampling Rate: %f\n", samplingRate);
    printf("bit depth: %i\n", bitsPerSample);

    /* Play file */
    int numBytes = (nSamples * 4);  // ReadSamples returns samples in 32 bit ints
    int *soundSamples = (int *) malloc(numBytes);
    AIFF_ReadSamples32Bit(song, soundSamples, nSamples);
    for (int i = 0; i < nSamples; i++) {
        printf("Next Sample: %i\n", soundSamples[i]);
    }


    /* Close song file */
    AIFF_CloseFile(song);
    return 0;
}