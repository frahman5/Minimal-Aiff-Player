/* == Typedefs == */
typedef uint32_t IFFType;       // represents 4 ASCII digits

/* Struct for AIFF reading */
struct AIFF_Ref_Basic {
    FILE *f;                    // pointer to AIFF file
    int segmentSize;            // number of 8 bit segments in a sample (1-4)
    int bitDepth;               // number of bits in a sample
    int numChannels;            // number of channels (1-8)
    double sampleRate;          // rate at which to play AIFF (in hertz)
    uint64_t numSampleFrames;   // number of sample frames in AIFF
    IFFType format;             // IFF format of file
    IFFType audioFormat;        // format of audio in file (expected: LPCM)
};
typedef struct AIFF_Ref_Basic AIFF_Ref_Basic;

/* Holds header info for an IFF file */
struct IFFHeader {
    IFFType hid;                // FORM, LIST, CAT 
    uint32_t len;
    IFFType fid;                // AIFF, etc. 
};

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
/* Open file (for reading) and store common chunk metadata */
AIFF_Ref_Basic openAIFF_File(const char *file);
/* Without libaiff, get waveform data and song metadata from common chunk */
int getAudioDataIND(const char *file, AIFF_Ref_Basic *song, 
                    int **sampleArray, uint64_t *nSampleFrames);
/* Using libaiff, get waveform data and song metadata from common chunk */
int getAudioData(const char *file, AIFF_Ref *song, int **sampleArray, uint64_t *nSamples);