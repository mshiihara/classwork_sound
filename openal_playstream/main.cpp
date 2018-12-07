#include <stdio.h>
#include <al.h>
#include <alc.h>

// ストリームに使用するバッファの数
#define NUMBUFFERS 4

int main(void) {
    // OpenAL Open
    ALCdevice* device = alcOpenDevice(nullptr);
    ALCcontext* context = nullptr;
    if (device) {
        context = alcCreateContext(device, nullptr);
        alcMakeContextCurrent(context);
    }

    // Create Buffer
    ALuint buffers[NUMBUFFERS];
    alGenBuffers( NUMBUFFERS, buffers);

    // Create Source
    ALuint source;
    alGenSources(1, &source);

    // OpenAL Close
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(context);
    alcCloseDevice(device);
    return 0;
}