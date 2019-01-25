#include "OpenAL.h"

void OpenAL::init()  {
    // OpenALを開く
    device = alcOpenDevice(nullptr);
    context = nullptr;
    if (device) {
        context = alcCreateContext(device, nullptr);
        alcMakeContextCurrent(context);
    }
}

void OpenAL::play(const char* filename) {
    // バッファの作成
    ALuint buffers[NUMBUFFERS];
    alGenBuffers(NUMBUFFERS, buffers);

    // ソースの作成
    ALuint source;
    alGenSources(1, &source);

    ALuint uiBuffer;

    RIFFChunk  riffChunk;
    WAVEFMT waveFmt;
    WAVEFILEINFO waveInfo;

    WAVEFILEINFO* m_WaveIDs[MAX_NUM_WAVEID];
    // 中身をゼロで初期化
    memset(&m_WaveIDs, 0, sizeof(m_WaveIDs));

    int waveId;
    unsigned long	ulFrequency = 0;
    unsigned long	ulFormat = 0;
    unsigned long	ulBufferSize;

    WAVEFORMATEX wfe;

    void* pData = NULL;

    ALint iBuffersProcessed;
    ALint iTotalBuffersProcessed;
    ALint iQueuedBuffers;
    
    // ファイルを開く
    FILE* fp = nullptr;
    fopen_s(&fp, filename, "rb");

    // ヘッダ情報を読み込み
    if (fp) { 
        readHeader(fp, &waveFmt, &waveInfo); 
    }
    else {
        printf("ファイルを開くことに失敗しました。\n");
    }

    long lLoop = 0;
    for (lLoop = 0; lLoop < MAX_NUM_WAVEID; lLoop++) {
        if (!m_WaveIDs[lLoop]){
		    m_WaveIDs[lLoop] = &waveInfo;
		    waveId = lLoop;
		    break;
	    }
    }
    ulFrequency = m_WaveIDs[waveId]->wfEXT.Format.nSamplesPerSec;
    
    //
    // フォーマットを取得
    //
    
    // 普通のwaveファイル
    if (m_WaveIDs[waveId]->wfType == WF_EX) {
        // 1チャンネル モノラル
        if (m_WaveIDs[waveId]->wfEXT.Format.nChannels == 1) {
            // 量子化ビット数で分岐
            switch (m_WaveIDs[waveId]->wfEXT.Format.wBitsPerSample) {
                case 4:
                    ulFormat = alGetEnumValue("AL_FORMAT_MONO_IMA4");
                    break;
                case 8:
                    ulFormat = alGetEnumValue("AL_FORMAT_MONO8");
                    break;
                case 16:
                    ulFormat = alGetEnumValue("AL_FORMAT_MONO16");
                    break;
            }
        }
        // 2チャンネル ステレオ
        else if (m_WaveIDs[waveId]->wfEXT.Format.nChannels == 2) {
            // 量子化ビット数で分岐
            switch (m_WaveIDs[waveId]->wfEXT.Format.wBitsPerSample) {
                case 4:
                    ulFormat = alGetEnumValue("AL_FORMAT_STEREO_IMA4");
                    break;
                case 8:
                    ulFormat = alGetEnumValue("AL_FORMAT_STEREO8");
                    break;
                case 16:
                    ulFormat = alGetEnumValue("AL_FORMAT_STEREO16");
                    break;
            }
        }
        // 4チャンネル
        else if ((m_WaveIDs[waveId]->wfEXT.Format.nChannels == 4) 
            && (m_WaveIDs[waveId]->wfEXT.Format.wBitsPerSample == 16)) {
            ulFormat = alGetEnumValue("AL_FORMAT_QUAD16");
        }
    }
    // 拡張されたwaveファイル
    else if (m_WaveIDs[waveId]->wfType == WF_EXT) {
        //todo: 後で実装する。サンプルデータも今無いし・・・
        printf("未実装\n");
    }

    //WAVEFORMATEXを取得
    memcpy(&wfe, &(m_WaveIDs[waveId]->wfEXT.Format), 
        sizeof(WAVEFORMATEX));
    
    //
    // 250mmに近いブロックアライメントの倍数をもとめる
    //

    // 1s/4 = 250mm
    ulBufferSize = wfe.nAvgBytesPerSec >> 2;
    // ブロックアライメントの倍数からはみ出している部分を引く
    ulBufferSize -= (ulBufferSize % wfe.nBlockAlign);
    
    // バッファを確保
    if (ulFormat != 0) {
        pData = malloc(ulBufferSize);
        // dataチャンクに移動
        fseek(fp, m_WaveIDs[waveId]->waveChunkPos, SEEK_SET);

        // バッファにデータを読み込み
        for (int i = 0; i < NUMBUFFERS; i++) {
            // ファイルからデータを読み取り 
            long len = readWaveFile(fp, *m_WaveIDs[waveId], pData, ulBufferSize);
            alBufferData(buffers[i], ulFormat, pData, len, ulFrequency);
		    alSourceQueueBuffers(source, 1, &buffers[i]);
        }
    }

    alSourcePlay(source);

    while (1) {
        // ESCで処理終了
        if (GetAsyncKeyState(VK_ESCAPE)) { break; }
        // 再生済みのバッファ数を求める
        iBuffersProcessed = 0;
	    alGetSourcei(source, AL_BUFFERS_PROCESSED, &iBuffersProcessed);
        // 再生済みのバッファがあった
        while (iBuffersProcessed) {
            // キューから使用済みのバッファを１つ取り出す（削除）
            // uiBufferniには削除されたバッファの名前（識別する為の値)が格納される
            uiBuffer = 0;
		    alSourceUnqueueBuffers(source, 1, &uiBuffer);

            // ファイルからデータを読み取り 
            long len = readWaveFile(fp, *m_WaveIDs[waveId], pData, ulBufferSize);
            alBufferData(uiBuffer, ulFormat, pData, len, ulFrequency);
		    alSourceQueueBuffers(source, 1, &uiBuffer);
            // 使用済みバッファの数を一つ減らす
            iBuffersProcessed--;
        }
        // 現在の状態を取得
        ALint iState;
        alGetSourcei(source, AL_SOURCE_STATE, &iState);
        // 再生していなければ処理を終了
        if (iState != AL_PLAYING) {
            break;
        }
    }
    fclose(fp);
}

//
// 引数のファイルポインタからRIFFヘッダを読み込み
// 正常であればtrueを返す関数
//
bool OpenAL::checkRIFFHeader(FILE* fp) {
    // ヘッダを読み取り
    RIFFHeader riffHeader;
    fread(&riffHeader, 1, sizeof(RIFFHeader), fp);
    // 読み取ったヘッダがRIFFであるか確認
    if (_strnicmp(riffHeader.tag, "RIFF", 4) == 0) {
        return true;
    }
    return false;
}

void OpenAL::readHeader(FILE* fp, WAVEFMT* waveFmt, WAVEFILEINFO* waveInfo) {
    //RIFFファイルかチェック
    if (checkRIFFHeader(fp)) {        
        printf("RIFFヘッダを読み取りました\n");
        RIFFChunk riffChunk;
        while (fread(&riffChunk, 1, sizeof(RIFFChunk), fp) == sizeof(RIFFChunk)) {
            // 読み取ったチャンクがfmt であるか確認
            if (_strnicmp(riffChunk.tag, "fmt ", 4) == 0) {
                readFMT_(fp, riffChunk, waveFmt, waveInfo);
            }
            else if (_strnicmp(riffChunk.tag, "data", 4) == 0) {
                printf("dataチャンク発見\n");
                //waveデータのサイズを取得
                waveInfo->waveSize = riffChunk.size;
                //後でwaveデータを読み込む際のセーブポイント
                waveInfo->waveChunkPos = ftell(fp);
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

//
// fmt チャンクを読み取る為の関数
//
void OpenAL::readFMT_(FILE* fp, RIFFChunk& riffChunk,WAVEFMT* waveFmt, 
    WAVEFILEINFO* waveInfo) {
    printf("fmt チャンクを発見\n");
    if (riffChunk.size <= sizeof(WAVEFMT)) {
        //フォーマット情報を読み取り
        fread(waveFmt, 1, riffChunk.size, fp);
        printf("usFormatTag:%d\nusChannels:%d\nulSamplesPerSec:%d\nulAvgBytesPerSec:%d\nusBlockAlign:%d\nusBitsPerSample:%d\nusSize:%d\nusReserved:%d\nulChannelMask:%d\nguidSubFormat:%d\n",
            waveFmt->usFormatTag,
            waveFmt->usChannels,
            waveFmt->ulSamplesPerSec,
            waveFmt->ulAvgBytesPerSec,
            waveFmt->usBlockAlign,
            waveFmt->usBitsPerSample,
            waveFmt->usSize,
            waveFmt->usReserved,
            waveFmt->ulChannelMask,
            waveFmt->guidSubFormat);
        // 一般的なのwaveファイルか？
        if (waveFmt->usFormatTag == WAVE_FORMAT_PCM) {
            waveInfo->wfType = WF_EX;
            memcpy(&waveInfo->wfEXT.Format, waveFmt, sizeof(PCMWAVEFORMAT));
        }
        // 3チャンネル以上の特別なwaveファイルか？
        else if (waveFmt->usFormatTag == WAVE_FORMAT_EXTENSIBLE) {
            waveInfo->wfType = WF_EXT;
            memcpy(&waveInfo->wfEXT, waveFmt, sizeof(WAVEFORMATEXTENSIBLE));
        }
    }
    else {
        // 次のチャンクへ移動
        fseek(fp, riffChunk.size, SEEK_CUR);
    }
}

long OpenAL::readWaveFile(FILE * fp, WAVEFILEINFO& waveInfo, void* pData, int bufferSize) {
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

void OpenAL::clear() {
    // OpenALを閉じる
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(context);
    alcCloseDevice(device);
}