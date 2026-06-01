#include <Arduino.h>
#include "config.h"
#include "NvsManager.h"
#include "ServoManager.h"
#include "NetworkManager.h"
#include "AudioManager.h"

// ==============================================================================
// DEFINICJA MASZYNY STANÓW (FINITE STATE MACHINE)
// ==============================================================================
enum class RobotState : uint8_t {
    BOOT,   
    IDLE,
    SPEAKING 
};

// ==============================================================================
// INSTANCJE GLOBALNE MODUŁÓW
// ==============================================================================
RobotState currentState = RobotState::BOOT;
NvsManager nvs;
ServoManager servo;
NetworkManager net; 
AudioManager audio; // <--- Menedżer dźwięku

// ==============================================================================
// FUNKCJA BEZPIECZNEJ ZMIANY STANÓW
// ==============================================================================
void transitionTo(RobotState newState) {
    if (currentState == newState) return;

    Serial.printf("[STATE] Zmiana stanu systemu: %d -> %d\n", 
                  static_cast<uint8_t>(currentState), static_cast<uint8_t>(newState));
                  
    currentState = newState;
}

// ==============================================================================
// GŁÓWNA FUNKCJA STARTOWA (BOOT SEQUENCE)
// ==============================================================================
void setup() {
    Serial.begin(115200);
    delay(1000); 
    
    Serial.println("\n==============================================");
    Serial.println("  ROBOT 6DOF MASTER FIRMWARE — INICJALIZACJA  ");
    Serial.println("==============================================");

    // 1. Pamięć Flash NVS
    if (!nvs.init()) {
        Serial.println("[BŁĄD KRYTYCZNY] Awaria pamięci Flash NVS!");
    }

    // 2. Sterownik PCA9685
    if (!servo.init()) {
        Serial.println("[BŁĄD KRYTYCZNY] Nie wykryto sterownika PCA9685 na I2C!");
    }

    // 3. Ładowanie twardych limitów
    Serial.println("[SYSTEM] Pobieranie profili kinematycznych...");
    for (uint8_t i = 0; i < 6; i++) {
        ServoBounds bounds = nvs.loadServoLimits(i);
        servo.setSoftwareLimits(i, bounds);
    }

    // 4. Sprzętowe zabezpieczenie struktury i Test Kinematyki
    Serial.println("[SYSTEM] Ustawianie w pionie i przygotowanie do testu ruchu...");
    servo.moveHomeAll();
    
    // Asynchroniczne oczekiwanie (1 sekunda), pozwalające serwom na dojście do celu
    unsigned long startTest = millis();
    while(millis() - startTest < 1000) { 
        servo.updateInterpolation(); 
    } 
    
    // Odpalenie sekwencyjnego tańca diagnostycznego
    servo.executeCalibrationDance();

    // 5. Inicjalizacja Podsystemu Audio (I2S1, Karta SD)
    if (!audio.init()) {
        Serial.println("[ERR] Podsystem audio zawiódł, uruchamiam bez dźwięku.");
    }

    // 6. Start Infrastruktury Sieciowej
    net.startSystem(&nvs);
    
    // Głosowy test sprawności układu! Zrób plik test.mp3 na karcie SD, by usłyszeć bota po starcie!
    audio.playFile("/test.mp3");

    // 6. Maszyna przechodzi w tryb nasłuchu
    transitionTo(RobotState::IDLE);
    Serial.println("[SYSTEM] Robot gotowy do pracy, maszyna stanów aktywna.");
}

// ==============================================================================
// ASYNCHRONICZNA PĘTLA GŁÓWNA (ZERO-BLOCKING ROUTINE)
// ==============================================================================
void loop() {
    // BEZWARUNKOWE, ASYNCHRONICZNE ODŚWIEŻANIE DEKODERA AUDIO (I2S DMA)
    audio.process();

    // BEZWARUNKOWE TAKTOWANIE RUCHU I USYPIANIA (SERVO SWEEPING & AUTO-SLEEP)
    servo.updateInterpolation();

    // ASYNCHRONICZNE PRZETWARZANIE PAKIETÓW SIECIOWYCH
    net.process();

    // ROUTING ZDARZEŃ W ZALEŻNOŚCI OD OBECNEGO STANU
    switch (currentState) {
        case RobotState::IDLE:
            break;
            
        default:
            break;
    }
}