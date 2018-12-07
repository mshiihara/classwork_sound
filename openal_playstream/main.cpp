#include <stdio.h>
#include <al.h>
#include <alc.h>

// �X�g���[���Ɏg�p����o�b�t�@�̐�
#define NUMBUFFERS 4
#define WAVE_FILE_NAME "sample.wav"

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

    // �t�@�C�����J���̂ɐ���
    if (fp) {

    }
    // �t�@�C�����J�����ƂɎ��s
    else {

    }

    if (fp) {
        fclose(fp);
    }
    // OpenAL�����
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(context);
    alcCloseDevice(device);

    return 0;
}