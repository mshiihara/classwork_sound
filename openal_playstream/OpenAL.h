#pragma once
#include <Windows.h>
#include <al.h>
#include <alc.h>
#include <string.h>
#include <guiddef.h>
#include <mmreg.h>
#include "WaveFile.h"

// ストリームに使用するバッファの数
#define NUMBUFFERS 4

#define MAX_NUM_WAVEID 1024

class OpenAL {
    ALCdevice* device;
    ALCcontext* context;
public:
    void init();
    void play(const char* filename);
    void clear();
private:
    bool checkRIFFHeader(FILE* fp);
    void readHeader(FILE* fp, WAVEFMT* waveFmt, WAVEFILEINFO* waveInfo);
    void readFMT_(FILE* fp, RIFFChunk& riffChunk, WAVEFMT* waveFmt,
        WAVEFILEINFO* waveInfo);
    long readWaveFile(FILE* fp, WAVEFILEINFO& waveInfo, void* pData, int bufferSize);
};