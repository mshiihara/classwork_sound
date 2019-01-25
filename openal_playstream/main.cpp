#include <Windows.h>
#include <stdio.h>
#include <al.h>
#include <alc.h>
#include <string.h>
#include <guiddef.h>
#include <mmreg.h>

// ストリームに使用するバッファの数
#define NUMBUFFERS 4
#define WAVE_FILE_NAME "sample.wav"

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
    void init() {
        // OpenALを開く
        device = alcOpenDevice(nullptr);
        context = nullptr;
        if (device) {
            context = alcCreateContext(device, nullptr);
            alcMakeContextCurrent(context);
        }
    }
    
    void play() {
        // バッファの作成
        ALuint buffers[NUMBUFFERS];
        alGenBuffers(NUMBUFFERS, buffers);

        // ソースの作成
        ALuint source;
        alGenSources(1, &source);

        ALuint uiBuffer;

        FILE* fp = nullptr;
        fopen_s(&fp, WAVE_FILE_NAME, "rb");

        RIFFHeader riffHeader;
        RIFFChunk  riffChunk;
        WAVEFMT waveFmt;
        WAVEFILEINFO waveInfo;

        WAVEFILEINFO* m_WaveIDs[MAX_NUM_WAVEID];
        // 中身をゼロで初期化
        memset(&m_WaveIDs, 0, sizeof(m_WaveIDs));

        int waveId;
        unsigned long	ulDataSize = 0;
	    unsigned long	ulFrequency = 0;
	    unsigned long	ulFormat = 0;
        unsigned long	ulBufferSize;
        unsigned long	ulBytesWritten;

        WAVEFORMATEX wfe;

        void* pData = NULL;

        ALint iBuffersProcessed;
        ALint iTotalBuffersProcessed;
        ALint iQueuedBuffers;
    

        // ファイルを開くのに成功
        if (fp) {
            // ヘッダを読み取り
            fread(&riffHeader, 1, sizeof(RIFFHeader), fp);
            // 読み取ったヘッダがRIFFであるか確認
            if (_strnicmp(riffHeader.tag, "RIFF", 4) == 0) {
                printf("RIFFヘッダを読み取りました\n");
                while (fread(&riffChunk, 1, sizeof(RIFFChunk), fp) == sizeof(RIFFChunk)) {
                    // 読み取ったチャンクがfmt であるか確認
                    if (_strnicmp(riffChunk.tag, "fmt ", 4) == 0) {
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
        // ファイルを開くことに失敗
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

        ulDataSize  = m_WaveIDs[waveId]->waveSize;
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
                //
                // 読み込もうと考えているサイズがファイルに残っているか？
                //
                unsigned long ulOffset = ftell(fp);
                if ((ulOffset - m_WaveIDs[waveId]->waveChunkPos + ulBufferSize) > m_WaveIDs[waveId]->waveSize) {
                    ulBufferSize = m_WaveIDs[waveId]->waveSize - (ulOffset - m_WaveIDs[waveId]->waveChunkPos);
                }
                // ファイルからデータを読み取り 
                ulBytesWritten = (unsigned long)fread(pData, 1, ulBufferSize, fp);
                alBufferData(buffers[i], ulFormat, pData, ulBytesWritten, ulFrequency);
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


                //
                // 読み込もうと考えているサイズがファイルに残っているか？
                //
                unsigned long ulOffset = ftell(fp);
                if ((ulOffset - m_WaveIDs[waveId]->waveChunkPos + ulBufferSize) > m_WaveIDs[waveId]->waveSize) {
                    ulBufferSize = m_WaveIDs[waveId]->waveSize - (ulOffset - m_WaveIDs[waveId]->waveChunkPos);
                }
                // ファイルからデータを読み取り 
                ulBytesWritten = (unsigned long)fread(pData, 1, ulBufferSize, fp);
                alBufferData(uiBuffer, ulFormat, pData, ulBytesWritten, ulFrequency);
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

    void clear() {
        // OpenALを閉じる
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(context);
        alcCloseDevice(device);
    }
};

int main(void) {
    OpenAL openAL;
    // OpenAL初期化
    openAL.init();
    // 再生
    openAL.play();
    // OpenAL終了
    openAL.clear();

    return 0;
}