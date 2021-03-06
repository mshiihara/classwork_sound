#pragma once
#include <Windows.h>
#include <stdio.h>
#include <guiddef.h>
#include <mmreg.h>
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

class WaveFile {
public:
    FILE* fp;
    WAVEFMT waveFmt;
    WAVEFILEINFO waveInfo;

    void open(const char* filename);
    long read(void* pData, int bufferSize);
private:
    bool checkRIFFHeader();
    void readHeader();
    void readFMT_(RIFFChunk& riffChunk);
};