#include <stdio.h>
#include <al.h>
#include <alc.h>

int main(void) {
    // OpenALを開く
    ALCdevice* device = alcOpenDevice(nullptr);
    ALCcontext* context = nullptr;
    if (device) {
        context = alcCreateContext(device, nullptr);
        alcMakeContextCurrent(context);
    }

    // OpenALを閉じる
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(context);
    alcCloseDevice(device);
    return 0;
}