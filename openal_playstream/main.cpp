#include "OpenAL.h"

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