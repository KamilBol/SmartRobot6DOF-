#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <Arduino.h>
#include "AudioFileSourceSD.h"
#include "AudioGeneratorWAV.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"
#include "config.h"

// ==============================================================================
// KLASA ZARZĄDZAJĄCA DEKODERAMI AUDIO (Zero-Blocking DMA)
// ==============================================================================
class AudioManager {
private:
    AudioFileSourceSD* source;
    AudioGeneratorWAV* wav;
    AudioGeneratorMP3* mp3;
    AudioOutputI2S* out;
    
    bool isPlayingWav;
    bool isPlayingMp3;

public:
    AudioManager();
    
    // Inicjalizuje magistralę SPI (SD) oraz I2S1 (Głośnik)
    bool init();
    
    // Asynchronicznie rozpoczyna odtwarzanie pliku
    void playFile(const char* path);
    
    // Musi być taktowane cyklicznie w pętli loop!
    void process(); 
    
    void setVolume(float vol);
    void stopAll();
};

#endif