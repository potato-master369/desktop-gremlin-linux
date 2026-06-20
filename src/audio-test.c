#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include <stdio.h>

    ma_result result;
    ma_engine engine;
    
int main() {


    // Initialize the engine
    result = ma_engine_init(NULL, &engine);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize audio engine.\n");
        return -1;
    }

    // Play a sound file asynchronously
    ma_engine_play_sound(&engine, "background_music.mp3", NULL);

    printf("Press Enter to quit...\n");
    getchar();

    // Clean up resources
    ma_engine_uninit(&engine);
    return 0;
}

