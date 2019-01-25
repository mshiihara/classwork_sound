#include "WaveFile.h"

void WaveFile::open(const char * filename) {
    fopen_s(&fp, filename, "rb");
    // ヘッダ情報を読み込み
    if (fp) { 
        readHeader(); 
    }
    else {
        printf("ファイルを開くことに失敗しました。\n");
    }
}

long WaveFile::read(void * pData, int bufferSize) {
    //
    // 読み込もうと考えているサイズがファイルに残っているか？
    //
    unsigned long ulOffset = ftell(fp);
    if ((ulOffset - waveInfo.waveChunkPos + bufferSize) > waveInfo.waveSize) {
        bufferSize = waveInfo.waveSize - (ulOffset - waveInfo.waveChunkPos);
    }
    // ファイルからデータを読み取り 
    return fread(pData, 1, bufferSize, fp);
}

bool WaveFile::checkRIFFHeader() {
    // ヘッダを読み取り
    RIFFHeader riffHeader;
    fread(&riffHeader, 1, sizeof(RIFFHeader), fp);
    // 読み取ったヘッダがRIFFであるか確認
    if (_strnicmp(riffHeader.tag, "RIFF", 4) == 0) {
        return true;
    }
    return false;
}

void WaveFile::readHeader() {
    //RIFFファイルかチェック
    if (checkRIFFHeader()) {        
        printf("RIFFヘッダを読み取りました\n");
        RIFFChunk riffChunk;
        while (fread(&riffChunk, 1, sizeof(RIFFChunk), fp) == sizeof(RIFFChunk)) {
            // 読み取ったチャンクがfmt であるか確認
            if (_strnicmp(riffChunk.tag, "fmt ", 4) == 0) {
                readFMT_(riffChunk);
            }
            else if (_strnicmp(riffChunk.tag, "data", 4) == 0) {
                printf("dataチャンク発見\n");
                //waveデータのサイズを取得
                waveInfo.waveSize = riffChunk.size;
                //後でwaveデータを読み込む際のセーブポイント
                waveInfo.waveChunkPos = ftell(fp);
            }
            else {
                // 次のチャンクへ移動
                fseek(fp, riffChunk.size, SEEK_CUR);
            }
        }
    }
    else {
        printf("ヘッダがRIFFではありませんでした\n");
    }
}

void WaveFile::readFMT_(RIFFChunk & riffChunk) {
    printf("fmt チャンクを発見\n");
    if (riffChunk.size <= sizeof(WAVEFMT)) {
        //フォーマット情報を読み取り
        fread(&waveFmt, 1, riffChunk.size, fp);
        printf("usFormatTag:%d\nusChannels:%d\nulSamplesPerSec:%d\nulAvgBytesPerSec:%d\nusBlockAlign:%d\nusBitsPerSample:%d\nusSize:%d\nusReserved:%d\nulChannelMask:%d\nguidSubFormat:%d\n",
            waveFmt.usFormatTag,
            waveFmt.usChannels,
            waveFmt.ulSamplesPerSec,
            waveFmt.ulAvgBytesPerSec,
            waveFmt.usBlockAlign,
            waveFmt.usBitsPerSample,
            waveFmt.usSize,
            waveFmt.usReserved,
            waveFmt.ulChannelMask,
            waveFmt.guidSubFormat);
        // 一般的なのwaveファイルか？
        if (waveFmt.usFormatTag == WAVE_FORMAT_PCM) {
            waveInfo.wfType = WF_EX;
            memcpy(&waveInfo.wfEXT.Format, &waveFmt, sizeof(PCMWAVEFORMAT));
        }
        // 3チャンネル以上の特別なwaveファイルか？
        else if (waveFmt.usFormatTag == WAVE_FORMAT_EXTENSIBLE) {
            waveInfo.wfType = WF_EXT;
            memcpy(&waveInfo.wfEXT, &waveFmt, sizeof(WAVEFORMATEXTENSIBLE));
        }
    }
    else {
        // 次のチャンクへ移動
        fseek(fp, riffChunk.size, SEEK_CUR);
    }
}
