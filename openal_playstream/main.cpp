#include <stdio.h>
#include <al.h>
#include <alc.h>
#include <string.h>

// �X�g���[���Ɏg�p����o�b�t�@�̐�
#define NUMBUFFERS 4
#define WAVE_FILE_NAME "sample.wav"

// RIFF�`�����N���i�[����ׂ̍\����
struct RIFFHeader {
    char          tag[4];
    unsigned long size;
    char          format[4];
};

struct RIFFChunk{
	char          tag[4];
    unsigned long size;
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

    RIFFHeader riffChunk;
    // �t�@�C�����J���̂ɐ���
    if (fp) {
        // �w�b�_��ǂݎ��
        fread(&riffChunk, 1, sizeof(RIFFHeader), fp);
        // �ǂݎ�����w�b�_��RIFF�ł��邩�m�F
        if (_strnicmp(riffChunk.tag, "RIFF", 4) == 0) {
            printf("RIFF�w�b�_��ǂݎ��܂���\n");
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