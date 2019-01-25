#include "WaveFile.h"

void WaveFile::open(const char * filename) {
    fopen_s(&fp, filename, "rb");
    // �w�b�_����ǂݍ���
    if (fp) { 
        readHeader(); 
    }
    else {
        printf("�t�@�C�����J�����ƂɎ��s���܂����B\n");
    }
}

long WaveFile::read(void * pData, int bufferSize) {
    //
    // �ǂݍ������ƍl���Ă���T�C�Y���t�@�C���Ɏc���Ă��邩�H
    //
    unsigned long ulOffset = ftell(fp);
    if ((ulOffset - waveInfo.waveChunkPos + bufferSize) > waveInfo.waveSize) {
        bufferSize = waveInfo.waveSize - (ulOffset - waveInfo.waveChunkPos);
    }
    // �t�@�C������f�[�^��ǂݎ�� 
    return fread(pData, 1, bufferSize, fp);
}

bool WaveFile::checkRIFFHeader() {
    // �w�b�_��ǂݎ��
    RIFFHeader riffHeader;
    fread(&riffHeader, 1, sizeof(RIFFHeader), fp);
    // �ǂݎ�����w�b�_��RIFF�ł��邩�m�F
    if (_strnicmp(riffHeader.tag, "RIFF", 4) == 0) {
        return true;
    }
    return false;
}

void WaveFile::readHeader() {
    //RIFF�t�@�C�����`�F�b�N
    if (checkRIFFHeader()) {        
        printf("RIFF�w�b�_��ǂݎ��܂���\n");
        RIFFChunk riffChunk;
        while (fread(&riffChunk, 1, sizeof(RIFFChunk), fp) == sizeof(RIFFChunk)) {
            // �ǂݎ�����`�����N��fmt �ł��邩�m�F
            if (_strnicmp(riffChunk.tag, "fmt ", 4) == 0) {
                readFMT_(riffChunk);
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

void WaveFile::readFMT_(RIFFChunk & riffChunk) {
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
