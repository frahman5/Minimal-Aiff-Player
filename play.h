/* Function prototypes */
    // functions for playing waveform data
/* handle errors produced by port audio functions */
int handlePaError(PaError err); 
/* Extract song data into an array */
/* Clean song parameters after playing the file */
int cleanSong(int *soundSamples, AIFF_Ref *song);
/* Using portaudio, stream song to default audio device */
int streamSong(int *soundSamples, int numSamples);
    // functions for obtaining waveform data
/* Without libaiff, get waveform data and song metadata from common chunk */
int getAudioDataIND(const char *file, AIFF_Ref_basic *song, 
                    int **sampleArray, uint64_t *nSampleFrames)
/* Using libaiff, get waveform data and song metadata from common chunk */
int getAudioData(const char *file, AIFF_Ref *song, int **sampleArray, uint64_t *nSamples);