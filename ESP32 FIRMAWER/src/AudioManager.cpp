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

    // 1. Uruchomienie karty SD (BEZPIECZNA PRĘDKOŚĆ Z TWOJEGO TESTU)
    SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
    if (!SD.begin(SD_CS, SPI)) {
        Serial.println("[AUDIO_ERR] Brak karty SD lub błąd odczytu magistrali SPI!");
        return false;
    }
    Serial.println("[AUDIO] Karta MicroSD zamontowana poprawnie.");

    // 2. TWORZYMY DEKODERY TYLKO RAZ (Dokładnie tak, jak w Twoim testowym kodzie)
    wav = new AudioGeneratorWAV();
    mp3 = new AudioGeneratorMP3();
    
    // 3. Inicjalizacja I2S z dokładnie tą samą flagą EXTERNAL_I2S
    out = new AudioOutputI2S(1, AudioOutputI2S::EXTERNAL_I2S); 
    out->SetPinout(SPK_BCLK, SPK_LRC, SPK_DIN);
    out->SetGain(1.0); // Wzmacniacz sprzętowy na maxa, procek podaje czysty dźwięk
    
    Serial.println("[AUDIO] Wzmacniacz akustyczny I2S gotowy (Port 1).");
    return true;
}

// ==============================================================================
// URUCHOMIENIE ODTWARZANIA DŹWIĘKU
// ==============================================================================
void AudioManager::playFile(const char* path) {
    stopAll(); // Zatrzymuje aktualny dźwięk i uwalnia sprzętowy bufor czytnika

    Serial.printf("[AUDIO] Żądanie odtworzenia pliku: %s\n", path);

    if (!SD.exists(path)) {
        Serial.println("[AUDIO_ERR] Plik nie istnieje na karcie MicroSD!");
        return;
    }

    // Ładujemy plik ze stabilnej szyny SPI
    source = new AudioFileSourceSD(path);
    String pathStr = String(path);
    pathStr.toLowerCase();

    // Rzutowanie na odpowiedni dekoder
    if (pathStr.endsWith(".wav")) {
        if (!wav->begin(source, out)) {
            Serial.println("[AUDIO_ERR] Błąd dekodera WAV! I2S odrzucił parametry pliku.");
        } else {
            isPlayingWav = true;
        }
    } else if (pathStr.endsWith(".mp3")) {
        if (!mp3->begin(source, out)) {
            Serial.println("[AUDIO_ERR] Błąd dekodera MP3!");
        } else {
            isPlayingMp3 = true;
        }
    } else {
        Serial.println("[AUDIO_ERR] Zły format pliku! Obsługiwane tylko .mp3 i .wav");
        stopAll();
    }
}

// ==============================================================================
// ASYNCHRONICZNY SILNIK ODTWARZACZA (ZERO-BLOCKING)
// ==============================================================================
void AudioManager::process() {
    if (isPlayingWav && wav->isRunning()) {
        if (!wav->loop()) {
            wav->stop(); // Zatrzymuje I2S
            isPlayingWav = false;
            Serial.println("[AUDIO] Zakończono odtwarzanie pliku WAV.");
        }
    }
    
    if (isPlayingMp3 && mp3->isRunning()) {
        if (!mp3->loop()) {
            mp3->stop();
            isPlayingMp3 = false;
            Serial.println("[AUDIO] Zakończono odtwarzanie pliku MP3.");
        }
    }
}

// ==============================================================================
// ZATRZYMANIE I ZWOLNIENIE PAMIĘCI RAM
// ==============================================================================
void AudioManager::stopAll() {
    // Delikatne zatrzymanie dekoderów
    if (wav && wav->isRunning()) wav->stop();
    if (mp3 && mp3->isRunning()) mp3->stop();
    
    // Zwalnianie uchwytu pliku z pamięci RAM (żeby zapobiec wyciekom pamięci)
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