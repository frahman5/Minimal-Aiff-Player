/* == A minimal AIFF player that uses libAIFF to parse the AIFF file, 
      and uses libportaudio to stream the file to audio devices == */

#define LIBAIFF_NOCOMPAT 1      // do not use LibAiff 2 API compatability
#include <math.h>               // pow
#include "play.h"


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


int cleanSong(int *soundSamples, AIFF_Ref *song) {
    free(soundSamples);                                 // Free soundSamples
    AIFF_CloseFile(*song);                              // Close song file  
    return 0;  
}