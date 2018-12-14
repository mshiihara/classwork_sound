#include <stdio.h>
#include <al.h>
#include <alc.h>
#include <string.h>
#include <guiddef.h>
#include <mmreg.h>

// �X�g���[���Ɏg�p����o�b�t�@�̐�
#define NUMBUFFERS 4
#define WAVE_FILE_NAME "sample.wav"

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
    WAVEFILETYPE	     wfType; // PCM�Ȃ̂�EXTENSIBLE�Ȃ̂�����ʂ���ׂ̏��
    WAVEFORMATEXTENSIBLE wfEXT;  // �t�H�[�}�b�g���
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
    alGenBuffers( NUMBUFFERS, buffers);

    // �\�[�X�̍쐬
    ALuint source;
    alGenSources(1, &source);
    
    FILE* fp = nullptr;
    fopen_s(&fp, WAVE_FILE_NAME, "rb");

    RIFFHeader riffHeader;
    RIFFChunk  riffChunk;
    WAVEFMT waveFmt;
    WAVEFILEINFO waveInfo;

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

                        }
                        // 3�`�����l���ȏ�̓��ʂ�wave�t�@�C�����H
                        else if (waveFmt.usFormatTag == WAVE_FORMAT_EXTENSIBLE) {

                        }
                    }
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
        fclose(fp);
    }
    // �t�@�C�����J�����ƂɎ��s
    else {

    }

    // OpenAL�����
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(context);
    alcCloseDevice(device);

    return 0;
}