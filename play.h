#include <libaiff/libaiff.h>    // for parsing AIFF

/* LIBAIFF related functions */
/* Clean song parameters after playing the file */
int cleanSong(int *soundSamples, AIFF_Ref *song);
/* Using libaiff, get waveform data and song metadata from common chunk */
int getAudioData(const char *file, AIFF_Ref *song, int **sampleArray, uint64_t *nSamples);

