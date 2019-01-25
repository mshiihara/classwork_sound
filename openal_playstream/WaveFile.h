#pragma once

class WaveFile {
public:
    FILE* fp;
    WAVEFMT* waveFmt;
    WAVEFILEINFO* waveInfo;

    void open(const char* filename);
    long read(void* pData, int bufferSize);
private:
    bool checkRIFFHeader();
    void readHeader();
    readFMT_(RIFFChunk& riffChunk);
};