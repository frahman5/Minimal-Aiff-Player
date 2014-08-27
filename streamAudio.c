/* == Holds functions that sends a stream of audio samples to the output
      devices == */

int streamSong(int *soundSamples, int numSamples) {
    printf("numSampleFrames: %d\n", numSamples);
    PaError err;
    PaStream *stream;
    int buffer[FRAMES_PER_BUFFER][2];           // stereo output buffer

    /* Open a stream to play the song */
    if ((err = Pa_Initialize()) != paNoError ) return err;
    err = Pa_OpenDefaultStream(
            &stream,                            // stream
            0,                                  // numInputChannels. 0->output only stream
            2,                                  // numOutputChannels
            paInt32,                            // sample format (song is read in as 32 bit ints)
            44100,                              // sample rate in hertz,
            0,                                  // preferred granularity for blocking read/write stream. 0->let portaudio optimize this 
            NULL,                               // pointer to asynchronous callback function for filling output buffer,
            NULL);                              // pointer passed to callback function
    if (err != paNoError) return err;      

    /* Play the song */
    int loopOffset;
    int frameOffset;
    int loopLimit = numSamples / FRAMES_PER_BUFFER;     // on each loop, we go through FRAMES_PER_BUFFER "samples" as defined by AIFF
    int samplesLeft = numSamples * 2;                   // recall an AIFF sample is actually a frame
    int numFrames;                                      // number of frames to write to stream
    if ((err = Pa_StartStream(stream)) != paNoError) return err;

    int counter = 0;
    while (samplesLeft != 0) 
    {
        numFrames = (samplesLeft > FRAMES_PER_BUFFER) ? FRAMES_PER_BUFFER : (samplesLeft / 2);
        loopOffset = (counter++ * 2 * numFrames); 
        samplesLeft -= (numFrames * 2);

        for (int j = 0; j < numFrames; j++) // Each iteration, load another frame into the buffer
        {
            frameOffset = 2 * j;            // On jth iteration, we've already read frameOffset frames
            buffer[j][0] = *(soundSamples + loopOffset + frameOffset);
            buffer[j][1] = *(soundSamples + loopOffset + frameOffset + 1);
        }
        err = Pa_WriteStream(stream, buffer, numFrames);
        if (err != paNoError) return err;

    }
    
    /* Clean up shop */
    if ( ((err = Pa_StopStream(stream)) != paNoError)  ||    // Stop the stream
         ((err = Pa_CloseStream(stream)) != paNoError) ||    // Close the stream
         ((err = Pa_Terminate()) != paNoError) )             // Terminate port audio
        return err; 

    return 0;

}

int handlePaError(PaError err) {
    fprintf(stderr, "An Error occured while using the portaudio stream\n");
    fprintf(stderr, "Error Number: %d\n", err);
    fprintf(stderr, "Error message: %s\n", Pa_GetErrorText( err ));
    if (err == paUnanticipatedHostError) 
    {
        const PaHostErrorInfo *hostErrorInfo = Pa_GetLastHostErrorInfo();
        fprintf(stderr, "Host API error = #%ld, hostApiType = %d\n", hostErrorInfo->errorCode, hostErrorInfo->hostApiType);
        fprintf(stderr, "Host API error = %s\n", hostErrorInfo->errorText);
    }
    Pa_Terminate();
    abort();
}