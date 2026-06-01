#include <Arduino.h>
#include "config.h"
#include "NvsManager.h"
#include "ServoManager.h"

// ==============================================================================
// DEFINICJA MASZYNY STANÓW (FINITE STATE MACHINE)
// ==============================================================================
// Silnie typowana klasa wyliczeniowa. Zapobiega to przypadkowemu przypisaniu 
// złej wartości do stanu robota. Będziemy ją rozbudowywać w kolejnych etapach.
enum class RobotState : uint8_t {
    BOOT,   // Stan początkowy: uruchamianie podzespołów i wczytywanie pamięci
    IDLE    // Stan bezczynności: oczekiwanie na rozkazy lub odczyty z sensorów
};

// ==============================================================================
// INSTANCJE GLOBALNE MODUŁÓW
// ==============================================================================
RobotState currentState = RobotState::BOOT;
NvsManager nvs;
ServoManager servo;

// ==============================================================================
// FUNKCJA BEZPIECZNEJ ZMIANY STANÓW (ROUTER LOGIKI)
// ==============================================================================
void transitionTo(RobotState newState) {
    if (currentState == newState) return; // Zabezpieczenie przed podwójnym przełączeniem

    Serial.printf("[STATE] Zmiana stanu systemu: %d -> %d\n", 
                  static_cast<uint8_t>(currentState), static_cast<uint8_t>(newState));
                  
    currentState = newState;
}

// ==============================================================================
// GŁÓWNA FUNKCJA STARTOWA (BOOT SEQUENCE)
// ==============================================================================
void setup() {
    // Inicjalizacja portu szeregowego do debugowania
    Serial.begin(115200);
    delay(1000); // Krótka pauza na ustabilizowanie napięcia zasilacza przed startem
    
    Serial.println("\n==============================================");
    Serial.println("  ROBOT 6DOF MASTER FIRMWARE — INICJALIZACJA  ");
    Serial.println("==============================================");

    // 1. Inicjalizacja Pamięci Nieulotnej (NVS)
    if (!nvs.init()) {
        Serial.println("[BŁĄD KRYTYCZNY] Awaria pamięci Flash NVS! System może zachowywać się niestabilnie.");
    }

    // 2. Inicjalizacja Magistrali I2C i układu PCA9685
    if (!servo.init()) {
        Serial.println("[BŁĄD KRYTYCZNY] Nie wykryto sterownika PCA9685 na magistrali I2C!");
    }

    // 3. Wstrzykiwanie limitów bezpieczeństwa (Sanity Limits)
    Serial.println("[SYSTEM] Pobieranie profili kinematycznych...");
    for (uint8_t i = 0; i < 6; i++) {
        // Dla każdego z 6 stawów ładujemy jego unikalne, twarde limity z pamięci Flash
        ServoBounds bounds = nvs.loadServoLimits(i);
        
        // Przekazujemy załadowane limity do menedżera ruchu
        servo.setSoftwareLimits(i, bounds);
    }

    // 4. Sprzętowe zabezpieczenie struktury (Haptic Boot)
    // Zlecamy płynny powrót wszystkich serwomechanizmów do fizycznego pionu (HOME).
    // Zapobiega to uszkodzeniu obudowy lub zębatek.
    servo.moveHomeAll();

    // 5. Uruchomienie zakończone sukcesem – maszyna przechodzi w tryb nasłuchu
    transitionTo(RobotState::IDLE);
    Serial.println("[SYSTEM] Robot gotowy do pracy, maszyna stanów aktywna.");
}

// ==============================================================================
// ASYNCHRONICZNA PĘTLA GŁÓWNA (ZERO-BLOCKING ROUTINE)
// ==============================================================================
void loop() {
    
    // BEZWARUNKOWE TAKTOWANIE RUCHU (SERVO SWEEPING)
    // Ta funkcja musi kręcić się bez przerwy. Jeśli cel ruchu (Target) jest inny
    // niż obecna pozycja, funkcja w tle wykona mikro-krok co 15ms.
    servo.updateInterpolation();

    // ROUTING ZDARZEŃ W ZALEŻNOŚCI OD OBECNEGO STANU
    switch (currentState) {
        case RobotState::IDLE:
            // W przyszłości tutaj dodamy np. asynchroniczne odpytywanie czujnika dotyku
            // lub sonaru odległości.
            break;
            
        default:
            break;
    }
}