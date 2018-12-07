#include <stdio.h>
#include <al.h>
#include <alc.h>

// ストリームに使用するバッファの数
#define NUMBUFFERS 4
#define WAVE_FILE_NAME "sample.wav"

int main(void) {
    // OpenALを開く
    ALCdevice* device = alcOpenDevice(nullptr);
    ALCcontext* context = nullptr;
    if (device) {
        context = alcCreateContext(device, nullptr);
        alcMakeContextCurrent(context);
    }

    // バッファの作成
    ALuint buffers[NUMBUFFERS];
    alGenBuffers( NUMBUFFERS, buffers);

    // ソースの作成
    ALuint source;
    alGenSources(1, &source);
    
    FILE* fp = nullptr;
    fopen_s(&fp, WAVE_FILE_NAME, "rb");

    // ファイルを開くのに成功
    if (fp) {

    }
    // ファイルを開くことに失敗
    else {

    }

    if (fp) {
        fclose(fp);
    }
    // OpenALを閉じる
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(context);
    alcCloseDevice(device);

    return 0;
}