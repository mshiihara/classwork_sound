#pragma once

#include <al.h>
#include <alc.h>
#include <string.h>


#include "WaveFile.h"

// ストリームに使用するバッファの数
#define NUMBUFFERS 4

#define MAX_NUM_WAVEID 1024

class OpenAL {
    ALCdevice* device;
    ALCcontext* context;
    WaveFile waveFile;
public:
    void init();
    void play(const char* filename);
    void clear();
};