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



class OpenAL {
    ALCdevice* device;
    ALCcontext* context;
public:
    void init() {
        // OpenAL���J��
        device = alcOpenDevice(nullptr);
        context = nullptr;
        if (device) {
            context = alcCreateContext(device, nullptr);
            alcMakeContextCurrent(context);
        }
    }
    
    void play() {
        // �o�b�t�@�̍쐬
        ALuint buffers[NUMBUFFERS];
        alGenBuffers(NUMBUFFERS, buffers);

        // �\�[�X�̍쐬
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
        // ���g���[���ŏ�����
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

        ulDataSize  = m_WaveIDs[waveId]->waveSize;
        ulFrequency = m_WaveIDs[waveId]->wfEXT.Format.nSamplesPerSec;
    
        //
        // �t�H�[�}�b�g���擾
        //
    
        // ���ʂ�wave�t�@�C��
        if (m_WaveIDs[waveId]->wfType == WF_EX) {
            // 1�`�����l�� ���m����
            if (m_WaveIDs[waveId]->wfEXT.Format.nChannels == 1) {
                // �ʎq���r�b�g���ŕ���
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
            // 2�`�����l�� �X�e���I
            else if (m_WaveIDs[waveId]->wfEXT.Format.nChannels == 2) {
                // �ʎq���r�b�g���ŕ���
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
            // 4�`�����l��
            else if ((m_WaveIDs[waveId]->wfEXT.Format.nChannels == 4) 
                && (m_WaveIDs[waveId]->wfEXT.Format.wBitsPerSample == 16)) {
                ulFormat = alGetEnumValue("AL_FORMAT_QUAD16");
            }
        }
        // �g�����ꂽwave�t�@�C��
        else if (m_WaveIDs[waveId]->wfType == WF_EXT) {
            //todo: ��Ŏ�������B�T���v���f�[�^�����������E�E�E
            printf("������\n");
        }

        //WAVEFORMATEX���擾
        memcpy(&wfe, &(m_WaveIDs[waveId]->wfEXT.Format), 
            sizeof(WAVEFORMATEX));
    
        //
        // 250mm�ɋ߂��u���b�N�A���C�����g�̔{�������Ƃ߂�
        //

        // 1s/4 = 250mm
        ulBufferSize = wfe.nAvgBytesPerSec >> 2;
        // �u���b�N�A���C�����g�̔{������͂ݏo���Ă��镔��������
        ulBufferSize -= (ulBufferSize % wfe.nBlockAlign);
    
        // �o�b�t�@���m��
        if (ulFormat != 0) {
            pData = malloc(ulBufferSize);
            // data�`�����N�Ɉړ�
            fseek(fp, m_WaveIDs[waveId]->waveChunkPos, SEEK_SET);

            // �o�b�t�@�Ƀf�[�^��ǂݍ���
            for (int i = 0; i < NUMBUFFERS; i++) {
                //
                // �ǂݍ������ƍl���Ă���T�C�Y���t�@�C���Ɏc���Ă��邩�H
                //
                unsigned long ulOffset = ftell(fp);
                if ((ulOffset - m_WaveIDs[waveId]->waveChunkPos + ulBufferSize) > m_WaveIDs[waveId]->waveSize) {
                    ulBufferSize = m_WaveIDs[waveId]->waveSize - (ulOffset - m_WaveIDs[waveId]->waveChunkPos);
                }
                // �t�@�C������f�[�^��ǂݎ�� 
                ulBytesWritten = (unsigned long)fread(pData, 1, ulBufferSize, fp);
                alBufferData(buffers[i], ulFormat, pData, ulBytesWritten, ulFrequency);
			    alSourceQueueBuffers(source, 1, &buffers[i]);
            }
        }

        alSourcePlay(source);

        while (1) {
            // ESC�ŏ����I��
            if (GetAsyncKeyState(VK_ESCAPE)) { break; }
            // �Đ��ς݂̃o�b�t�@�������߂�
            iBuffersProcessed = 0;
	        alGetSourcei(source, AL_BUFFERS_PROCESSED, &iBuffersProcessed);
            // �Đ��ς݂̃o�b�t�@��������
            while (iBuffersProcessed) {
                // �L���[����g�p�ς݂̃o�b�t�@���P���o���i�폜�j
                // uiBufferni�ɂ͍폜���ꂽ�o�b�t�@�̖��O�i���ʂ���ׂ̒l)���i�[�����
                uiBuffer = 0;
			    alSourceUnqueueBuffers(source, 1, &uiBuffer);


                //
                // �ǂݍ������ƍl���Ă���T�C�Y���t�@�C���Ɏc���Ă��邩�H
                //
                unsigned long ulOffset = ftell(fp);
                if ((ulOffset - m_WaveIDs[waveId]->waveChunkPos + ulBufferSize) > m_WaveIDs[waveId]->waveSize) {
                    ulBufferSize = m_WaveIDs[waveId]->waveSize - (ulOffset - m_WaveIDs[waveId]->waveChunkPos);
                }
                // �t�@�C������f�[�^��ǂݎ�� 
                ulBytesWritten = (unsigned long)fread(pData, 1, ulBufferSize, fp);
                alBufferData(uiBuffer, ulFormat, pData, ulBytesWritten, ulFrequency);
			    alSourceQueueBuffers(source, 1, &uiBuffer);
                // �g�p�ς݃o�b�t�@�̐�������炷
                iBuffersProcessed--;
            }
            // ���݂̏�Ԃ��擾
            ALint iState;
            alGetSourcei(source, AL_SOURCE_STATE, &iState);
            // �Đ����Ă��Ȃ���Ώ������I��
            if (iState != AL_PLAYING) {
                break;
            }
        }
        fclose(fp);
    }

    void clear() {
        // OpenAL�����
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(context);
        alcCloseDevice(device);
    }
};

int main(void) {
    OpenAL openAL;
    // OpenAL������
    openAL.init();
    // �Đ�
    openAL.play();
    // OpenAL�I��
    openAL.clear();

    return 0;
}