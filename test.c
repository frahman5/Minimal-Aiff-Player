#include <assert.h>

/* A simple test suite to test AIFF handling files */

int test_open_file() {
    const char *testFile1 = "/Users/faiyamrahman/programming/C/playAIFF/AIFFs/
ByYourSidePt1.aiff"

    /* test 1 */
    AIFF_Ref_Basic *song = openAIFF_File(const char *testFile1);
    assert ( !(song->f) );
    assert (song->format == 0x46464941);                // FFIA 
    assert (song->segmentSize == 2);
    assert (song->bitDepth == 16);
    assert (song->numChannels == 2);
    assert (song->sampleRate == 44100);
    assert (song->numSampleFrames == 3789844);
    assert (song->audioFormat == 0x454e4f4e);           // ENON

}