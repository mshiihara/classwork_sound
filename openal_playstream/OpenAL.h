#pragma once
#include <Windows.h>
#include <stdio.h>
#include <al.h>
#include <alc.h>
#include <string.h>
#include <guiddef.h>
#include <mmreg.h>

// ストリームに使用するバッファの数
#define NUMBUFFERS 4

#define MAX_NUM_WAVEID 1024

// RIFFチャンクを格納する為の構造体
struct RIFFHeader {
    char          tag[4];
    unsigned long size;
    char          format[4];
};

struct RIFFChunk {
    char          tag[4];
    unsigned long size;
};

struct WAVEFMT {
	unsigned short	usFormatTag;
	unsigned short	usChannels;
	unsigned long	ulSamplesPerSec;
	unsigned long	ulAvgBytesPerSec;
	unsigned short	usBlockAlign;
	unsigned short	usBitsPerSample;
	unsigned short	usSize;
	unsigned short  usReserved;
	unsigned long	ulChannelMask;
    GUID            guidSubFormat;
};
enum WAVEFILETYPE
{
	WF_EX  = 1,
	WF_EXT = 2
};

struct WAVEFILEINFO {
    WAVEFILETYPE         wfType; // PCMなのかEXTENSIBLEなのかを区別する為の情報
    WAVEFORMATEXTENSIBLE wfEXT;  // フォーマット情報
    unsigned long waveSize;      // waveデータの大きさ
    unsigned long waveChunkPos;  // waveチャンクのファイルポインタ
};



class OpenAL {
    ALCdevice* device;
    ALCcontext* context;
public:
    void init();
    void play(const char* filename);
    void clear();
private:
    bool checkRIFFHeader(FILE* fp);
    void readFMT_(FILE* fp, RIFFChunk& riffChunk, WAVEFMT* waveFmt,
        WAVEFILEINFO* waveInfo);
};