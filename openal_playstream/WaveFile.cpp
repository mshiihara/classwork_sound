#include "WaveFile.h"

void WaveFile::open(const char * filename) {
    fopen_s(&fp, filename, "rb");
    readHeader();
}

long WaveFile::read(void * pData, int bufferSize) {
    return 0;
}

bool WaveFile::checkRIFFHeader() {
    return false;
}

void WaveFile::readHeader() {
}

void WaveFile::readFMT_(RIFFChunk & riffChunk) {
}
