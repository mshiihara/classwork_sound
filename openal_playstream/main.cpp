#include <Windows.h>
#include <stdio.h>
#include <al.h>
#include <alc.h>
#include <string.h>
#include <guiddef.h>
#include <mmreg.h>

// �X�g���[���Ɏg�p����o�b�t�@�̐�
#define NUMBUFFERS 4
#define WAVE_FILE_NAME "sample.wav"

#define MAX_NUM_WAVEID 1024

// RIFF�`�����N���i�[����ׂ̍\����
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
    WAVEFILETYPE         wfType; // PCM�Ȃ̂�EXTENSIBLE�Ȃ̂�����ʂ���ׂ̏��
    WAVEFORMATEXTENSIBLE wfEXT;  // �t�H�[�}�b�g���
    unsigned long waveSize;      // wave�f�[�^�̑傫��
    unsigned long waveChunkPos;  // wave�`�����N�̃t�@�C���|�C���^
};


int main(void) {
    // OpenAL���J��
    ALCdevice* device = alcOpenDevice(nullptr);
    ALCcontext* context = nullptr;
    if (device) {
        context = alcCreateContext(device, nullptr);
        alcMakeContextCurrent(context);
    }

    // �o�b�t�@�̍쐬
    ALuint buffers[NUMBUFFERS];
    alGenBuffers(NUMBUFFERS, buffers);

    // �\�[�X�̍쐬
    ALuint source;
    alGenSources(1, &source);

    FILE* fp = nullptr;
    fopen_s(&fp, WAVE_FILE_NAME, "rb");

    RIFFHeader riffHeader;
    RIFFChunk  riffChunk;
    WAVEFMT waveFmt;
    WAVEFILEINFO waveInfo;

    WAVEFILEINFO* m_WaveIDs[MAX_NUM_WAVEID];
    // ���g���[���ŏ�����
    memset(&m_WaveIDs, 0, sizeof(m_WaveIDs));

    int waveId;

    // �t�@�C�����J���̂ɐ���
    if (fp) {
        // �w�b�_��ǂݎ��
        fread(&riffHeader, 1, sizeof(RIFFHeader), fp);
        // �ǂݎ�����w�b�_��RIFF�ł��邩�m�F
        if (_strnicmp(riffHeader.tag, "RIFF", 4) == 0) {
            printf("RIFF�w�b�_��ǂݎ��܂���\n");
            while (fread(&riffChunk, 1, sizeof(RIFFChunk), fp) == sizeof(RIFFChunk)) {
                // �ǂݎ�����`�����N��fmt �ł��邩�m�F
                if (_strnicmp(riffChunk.tag, "fmt ", 4) == 0) {
                    printf("fmt �`�����N�𔭌�\n");
                    if (riffChunk.size <= sizeof(WAVEFMT)) {
                        //�t�H�[�}�b�g����ǂݎ��
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
                        // ��ʓI�Ȃ�wave�t�@�C�����H
                        if (waveFmt.usFormatTag == WAVE_FORMAT_PCM) {
                            waveInfo.wfType = WF_EX;
                            memcpy(&waveInfo.wfEXT.Format, &waveFmt, sizeof(PCMWAVEFORMAT));
                        }
                        // 3�`�����l���ȏ�̓��ʂ�wave�t�@�C�����H
                        else if (waveFmt.usFormatTag == WAVE_FORMAT_EXTENSIBLE) {
                            waveInfo.wfType = WF_EXT;
                            memcpy(&waveInfo.wfEXT, &waveFmt, sizeof(WAVEFORMATEXTENSIBLE));
                        }
                    }
                    else {
                        // ���̃`�����N�ֈړ�
                        fseek(fp, riffChunk.size, SEEK_CUR);
                    }
                }
                else if (_strnicmp(riffChunk.tag, "data", 4) == 0) {
                    printf("data�`�����N����\n");
                    //wave�f�[�^�̃T�C�Y���擾
                    waveInfo.waveSize = riffChunk.size;
                    //���wave�f�[�^��ǂݍ��ލۂ̃Z�[�u�|�C���g
                    waveInfo.waveChunkPos = ftell(fp);
                }
                else {
                    // ���̃`�����N�ֈړ�
                    fseek(fp, riffChunk.size, SEEK_CUR);
                }
            }
        }
        else {
            printf("�w�b�_��RIFF�ł͂���܂���ł���\n");
        }
    }
    // �t�@�C�����J�����ƂɎ��s
    else {
        printf("�t�@�C�����J�����ƂɎ��s���܂����B\n");
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
    // OpenAL�����
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(context);
    alcCloseDevice(device);

    return 0;
}