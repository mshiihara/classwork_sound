#include "OpenAL.h"

int main(void) {
    OpenAL openAL;
    // OpenAL������
    openAL.init();
    // �Đ�
    openAL.play("sample.wav");
    // OpenAL�I��
    openAL.clear();

    return 0;
}