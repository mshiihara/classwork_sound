#include "OpenAL.h"

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