#include "portaudio.h"          // for sending sound samples to sound card

#define FRAMES_PER_BUFFER   1024        // Defines how many audio samples are
                                        // sent to ouput at a time

/* Function prototypes */
int streamSong(int *soundSamples, int numSamples);
/* handle errors produced by port audio functions */
int handlePaError(PaError err); 