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

    // waveファイルを開く
    waveFile.open("sample.wav");

    long lLoop = 0;
    for (lLoop = 0; lLoop < MAX_NUM_WAVEID; lLoop++) {
        if (!m_WaveIDs[lLoop]){
		    m_WaveIDs[lLoop] = &waveFile.waveInfo;
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
        fseek(waveFile.fp, m_WaveIDs[waveId]->waveChunkPos, SEEK_SET);

        // バッファにデータを読み込み
        for (int i = 0; i < NUMBUFFERS; i++) {
            // ファイルからデータを読み取り 
            long len = waveFile.read(pData, ulBufferSize);
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
            long len = waveFile.read(pData, ulBufferSize);
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
}

void OpenAL::clear() {
    // OpenALを閉じる
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(context);
    alcCloseDevice(device);
}