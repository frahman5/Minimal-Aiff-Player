#include <stdint.h>             // uint types
#include <stdio.h>              // FILE

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
typedef struct IFFHeader IFFHeader;


/* == IFF codes == */
#define IFF_TYPE_FORM 0x4D524F46    // little endian ASCII "FORM"

/* == AIFF types == */
#define AIFF_TYPE_AIFF 0x46464941  // little endian ASCII "AIFF"
#define AIFF_TYPE_AIFC 0x43464941  // big endian ASCII "AIFC"

                /* Function prototypes */
/* Open file (for reading) and store common chunk metadata */
AIFF_Ref_Basic *openAIFF_File(const char *file);
/* Without libaiff, get waveform data and song metadata from common chunk */
int getAudioDataIND(const char *file, AIFF_Ref_Basic *song, 
                    int **sampleArray, uint64_t *nSampleFrames);