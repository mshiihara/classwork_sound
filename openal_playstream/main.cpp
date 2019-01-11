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


int main(void) {
    // OpenALを開く
    ALCdevice* device = alcOpenDevice(nullptr);
    ALCcontext* context = nullptr;
    if (device) {
        context = alcCreateContext(device, nullptr);
        alcMakeContextCurrent(context);
    }

    // バッファの作成
    ALuint buffers[NUMBUFFERS];
    alGenBuffers(NUMBUFFERS, buffers);

    // ソースの作成
    ALuint source;
    alGenSources(1, &source);

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

    fclose(fp);
    // OpenALを閉じる
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(context);
    alcCloseDevice(device);

    return 0;
}