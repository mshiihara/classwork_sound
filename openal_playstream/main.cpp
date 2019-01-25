#include "OpenAL.h"

int main(void) {
    OpenAL openAL;
    // OpenAL‰Šú‰»
    openAL.init();
    // Ä¶
    openAL.play("sample.wav");
    // OpenALI—¹
    openAL.clear();

    return 0;
}