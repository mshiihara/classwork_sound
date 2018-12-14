#include <stdio.h>
#include <al.h>
#include <alc.h>
#include <string.h>

// ストリームに使用するバッファの数
#define NUMBUFFERS 4
#define WAVE_FILE_NAME "sample.wav"

// RIFFチャンクを格納する為の構造体
struct RIFFHeader {
    char          tag[4];
    unsigned long size;
    char          format[4];
};

struct RIFFChunk{
	char          tag[4];
    unsigned long size;
};

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

    RIFFHeader riffChunk;
    // ファイルを開くのに成功
    if (fp) {
        // ヘッダを読み取り
        fread(&riffChunk, 1, sizeof(RIFFHeader), fp);
        // 読み取ったヘッダがRIFFであるか確認
        if (_strnicmp(riffChunk.tag, "RIFF", 4) == 0) {
            printf("RIFFヘッダを読み取りました\n");
        }
        else {
            printf("ヘッダがRIFFではありませんでした\n");
        }
        fclose(fp);
    }
    // ファイルを開くことに失敗
    else {

    }

    // OpenALを閉じる
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(context);
    alcCloseDevice(device);

    return 0;
}