#include <stdint.h>             // uint types
#include <stdio.h>              // FILE
#include "endian.h"             // handles endian issues
#include "streamAudio.h"

/*
 * Infinite & NAN values
 * for non-IEEE systems
 * Used for IEEE 754 digit handling
 */
#ifndef HUGE_VAL
#ifdef HUGE
#define INFINITE_VALUE  HUGE
#define NAN_VALUE   HUGE
#endif
#else
#define INFINITE_VALUE  HUGE_VAL
#define NAN_VALUE   HUGE_VAL
#endif

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

/* Holds metadata about an IFF Chunk */
struct IFFChunk {
    IFFType id;
    uint32_t len;
};
typedef struct IFFChunk IFFChunk;


/* == IFF codes == */
#define IFF_TYPE_FORM 0x4D524F46    // little endian ASCII "FORM"

/* == AIFF types == */
#define AIFF_TYPE_AIFF 0x46464941  // little endian ASCII "AIFF"
#define AIFF_TYPE_AIFC 0x41494643  // little endian ASCII "AIFC"
#define AIFF_TYPE_COMM 0x4d4d4f43  // little endian ASCII "COMM"
#define AIFF_TYPE_SSND 0x444e5353  // little endian ASCII "SSND"

                /* Function prototypes */
/* Open file (for reading) and store common chunk metadata */
AIFF_Ref_Basic *openAIFF_File(const char *file);
/* Close file and free space for AIFF_REF_BASIC */
int closeAIFF_File(AIFF_Ref_Basic *songRef);
/* Clean up the song reference and the waveform data array */
int cleanSongIND(int *soundSamples, AIFF_Ref_Basic *songRef);
/* Without libaiff, get waveform data and song metadata from common chunk */
int getAudioDataIND(const char *file, AIFF_Ref_Basic *song, 
                    int **sampleArray, uint64_t *nSampleFrames);
/* Store info from the common chunk. Figure uut file format (LPCM? Other?) */
int init_ref_from_common_chunk(AIFF_Ref_Basic *songRef);
/*
 * Find the required chunks of an AIFF file--Common and Sound Data.
 * Return 1 (found) or 0 (not found / error). If found, update 'length' 
 * to be the chunk length.
 */
int find_comm_or_ssnd_chunk(IFFType chunk, AIFF_Ref_Basic *songRef, uint32_t *length);
/* Read in sound waveform data */
int readSamples32Bit(AIFF_Ref_Basic *songRef, int **sampleArray, int numSamples);