/*
Copyright (c) 2018 Mhage
Released under the MIT license
https://github.com/MhageGH/esp32_SoundRecorder/blob/master/LICENSE.txt
*/

#include <Arduino.h>

// 16bit, monoral, 44100Hz,  linear PCM
void CreateWavHeader(byte* header, int waveDataSize);  // size of header is 44
