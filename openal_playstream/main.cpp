#include <stdio.h>
#include <al.h>
#include <alc.h>

int main(void) {
    // OpenAL���J��
    ALCdevice* device = alcOpenDevice(nullptr);
    ALCcontext* context = nullptr;
    if (device) {
        context = alcCreateContext(device, nullptr);
        alcMakeContextCurrent(context);
    }

    // OpenAL�����
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(context);
    alcCloseDevice(device);
    return 0;
}