#include <assert.h>
#include "playIND.h"
#include "play.h"

/* A simple test suite to test AIFF handling files */

int test_open_file() {
    const char *testFile = "/Users/faiyamrahman/programming/C/playAIFF/AIFFs/\
ByYourSidePt1.aiff";

    /* test 1 */
    AIFF_Ref_Basic *song = openAIFF_File(testFile);
    assert (song);
    assert (song->f);
    assert (song->format == 0x46464941);                    // FFIA 
    assert (song->bitDepth == 16);
    assert (song->numChannels == 2);
    assert (song->numSampleFrames == 3789844);
    assert (song->segmentSize == 2);
    assert (song->sampleRate == 44100);

    return 1;

}

int test_get_audio_data() {
    const char *testFile = "/Users/faiyamrahman/programming/C/playAIFF/AIFFs/\
ByYourSidePt2.aiff";

    /* Get sound data using libAIFF */
    AIFF_Ref song;
    int *soundSamples;
    uint64_t numSamples;
    
    if (getAudioData(testFile, &song, &soundSamples, &numSamples) < 0)
    {
        fprintf(stderr, "Could not get audio data from %s\n", testFile);
        abort();
    }

    /* Get sound data using independent functions */
    AIFF_Ref_Basic *songTest;
    int *soundSamplesTest;
    uint64_t numSamplesTest;
    if (getAudioDataIND(testFile, songTest, &soundSamplesTest, &numSamplesTest) < 0) {
        fprintf(stderr, "Could not get audio data (test) from %s\n", testFile);
        abort();
    }

    /* Make sure it's all right */
    assert (numSamples == numSamplesTest);
    for (int i = 0; i < numSamples; i++) {
        // printf("%d: standard--test: %08x--%08x\n", i, soundSamples[i], soundSamplesTest[i]);
        assert (soundSamples[i] == soundSamplesTest[i]);
    }

    return 1;
}
int main() {
    test_open_file();
    test_get_audio_data();
    return 1;
}