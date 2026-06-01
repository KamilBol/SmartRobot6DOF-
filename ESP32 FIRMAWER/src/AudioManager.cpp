#include "AudioManager.h"
#include <SPI.h>
#include <SD.h>

AudioManager::AudioManager() {
    source = nullptr;
    wav = nullptr;
    mp3 = nullptr;
    out = nullptr;
    isPlayingWav = false;
    isPlayingMp3 = false;
}

// ==============================================================================
// INICJALIZACJA MAGISTRALI I SPRZĘTU AUDIO
// ==============================================================================
bool AudioManager::init() {
    Serial.println("[AUDIO] Inicjalizacja podsystemu (SPI & I2S)...");

    // 1. Uruchomienie karty SD z niestandardowymi pinami na magistrali VSPI
    SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
    
    // Prędkość 40MHz dla szybkiego czytania MP3/WAV
    if (!SD.begin(SD_CS, SPI, 40000000)) {
        Serial.println("[AUDIO_ERR] Brak karty SD lub błąd odczytu magistrali SPI!");
        return false;
    }
    Serial.println("[AUDIO] Karta MicroSD (FAT32) zamontowana poprawnie.");

    // 2. Inicjalizacja wyjścia cyfrowego I2S do MAX98357A
    // Parametr (0, 1) oznacza użycie wbudowanych przetworników wewnętrznych procesora, szyna I2S_NUM_1
    oout = new AudioOutputI2S(0, 1); 
    out->SetPinout(SPK_BCLK, SPK_LRC, SPK_DIN);
    
    // W pełni sprzętowe wzmocnienie na module, mikrokontroler podaje czysty sygnał 1.0
    out->SetGain(1.0);
    
    Serial.println("[AUDIO] Wzmacniacz akustyczny I2S gotowy.");
    return true;
}

// ==============================================================================
// URUCHOMIENIE ODTWARZANIA DŹWIĘKU (PARSER)
// ==============================================================================
void AudioManager::playFile(const char* path) {
    stopAll(); // Czyszczenie starych buforów

    Serial.printf("[AUDIO] Żądanie odtworzenia pliku: %s\n", path);

    if (!SD.exists(path)) {
        Serial.println("[AUDIO_ERR] Plik nie istnieje na karcie MicroSD!");
        return;
    }

    source = new AudioFileSourceSD(path);
    String pathStr = String(path);
    pathStr.toLowerCase();

    // Dynamiczny routing dekodera
    if (pathStr.endsWith(".wav")) {
        wav = new AudioGeneratorWAV();
        wav->begin(source, out);
        isPlayingWav = true;
    } else if (pathStr.endsWith(".mp3")) {
        mp3 = new AudioGeneratorMP3();
        mp3->begin(source, out);
        isPlayingMp3 = true;
    } else {
        Serial.println("[AUDIO_ERR] Zły format pliku! Obsługiwane tylko .mp3 i .wav");
        stopAll();
    }
}

// ==============================================================================
// ASYNCHRONICZNY SILNIK ODTWARZACZA (ZERO-BLOCKING)
// ==============================================================================
void AudioManager::process() {
    if (isPlayingWav && wav) {
        if (wav->isRunning()) {
            if (!wav->loop()) { // Wrzuca kolejną paczkę bajtów do I2S
                wav->stop();
                isPlayingWav = false;
                Serial.println("[AUDIO] Zakończono plik WAV.");
            }
        }
    }
    
    if (isPlayingMp3 && mp3) {
        if (mp3->isRunning()) {
            if (!mp3->loop()) {
                mp3->stop();
                isPlayingMp3 = false;
                Serial.println("[AUDIO] Zakończono plik MP3.");
            }
        }
    }
}

// ==============================================================================
// ZATRZYMANIE I ZWOLNIENIE PAMIĘCI RAM (CZYSZCZENIE)
// ==============================================================================
void AudioManager::stopAll() {
    if (wav) {
        if (wav->isRunning()) wav->stop();
        delete wav;
        wav = nullptr;
    }
    if (mp3) {
        if (mp3->isRunning()) mp3->stop();
        delete mp3;
        mp3 = nullptr;
    }
    if (source) {
        source->close();
        delete source;
        source = nullptr;
    }
    isPlayingWav = false;
    isPlayingMp3 = false;
}

void AudioManager::setVolume(float vol) {
    if (out) {
        out->SetGain(vol);
    }
}