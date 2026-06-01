#ifndef SERVO_MANAGER_H
#define SERVO_MANAGER_H

#include <Arduino.h>
// Biblioteka obsługująca sprzętowy generator PWM (PCA9685) po magistrali I2C
#include <Adafruit_PWMServoDriver.h> 
#include "config.h"
#include "NvsManager.h"

// ==============================================================================
// KLASA ZARZĄDZAJĄCA RUCHEM SERWOMECHANIZMÓW (PCA9685)
// ==============================================================================
class ServoManager {
private:
    // Prywatna instancja sterownika I2C. Komunikuje się z PCA9685 pod domyślnym adresem 0x40.
    Adafruit_PWMServoDriver pwm;

    // Tablica przechowująca załadowane z pamięci Flash (NVS) twarde limity dla 6 serw.
    // Indeksy [0-5] odpowiadają fizycznym portom na sterowniku PCA9685.
    ServoBounds activeLimits[6];

    // Tablice asynchronicznego interpolatora ruchu (Servo Sweeping):
    // currentPositions - przechowuje aktualną fizyczną pozycję ramienia robota (w jednostkach Ticks)
    int16_t currentPositions[6];
    
    // targetPositions - przechowuje pozycję, do której robot ma dążyć w tle
    int16_t targetPositions[6];

    // Zmienne do usypiania serw (Auto-Sleep / Anti-Buzz) po osiągnięciu celu
    unsigned long lastMoveTime[6];
    bool isRelaxed[6];

    // Stoper (timer) sprzętowy oparty na funkcji millis() do regulacji prędkości ruchu
    unsigned long lastUpdateTime;
    
    // Stała określająca odstęp czasu (w milisekundach) pomiędzy kolejnymi mikrokrokami serw.
    // Wartość 15ms zapewnia płynny ruch przy częstotliwości pracy serw 50Hz (20ms okres).
    const uint8_t SWEEP_INTERVAL_MS = 15;

public:
    // Konstruktor klasy
    ServoManager();

    // Funkcja inicjująca magistralę I2C i sam sterownik PCA9685. Zwraca true jeśli wykryje układ.
    bool init();

    // Funkcja ładująca z NVS indywidualne limity bezpieczeństwa dla konkretnego portu/stawu.
    void setSoftwareLimits(uint8_t servoId, ServoBounds bounds);

    // Zleca asynchroniczny ruch serwa podając przyjazne kąty (0-180 stopni).
    // Funkcja NIE BLOKUJE procesora!
    void setServoAngle(uint8_t servoId, int16_t angle);

    // Zleca asynchroniczny ruch serwa podając surowe wartości PWM (Ticks: 150-450).
    void setServoTicksDirect(uint8_t servoId, int16_t ticks);

    // Serce modułu: Funkcja ta MUSI BYĆ WYWOŁYWANA CYKLICZNIE w pętli loop().
    // Wykonuje mikro-kroki do przodu, jeśli cel (target) różni się od obecnej pozycji (current).
    void updateInterpolation();

    // Rozkaz ratunkowy/resetujący: Natychmiast ustawia wszystkie serwa w bezpiecznych pozycjach HOME.
    void moveHomeAll();
};

#endif