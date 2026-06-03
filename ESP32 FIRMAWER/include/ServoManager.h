#ifndef SERVO_MANAGER_H
#define SERVO_MANAGER_H

#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h> 
#include "config.h"
#include "NvsManager.h"

// ==============================================================================
// KLASA ZARZĄDZAJĄCA RUCHEM SERWOMECHANIZMÓW (PCA9685)
// ==============================================================================
class ServoManager {
private:
    Adafruit_PWMServoDriver pwm;
    ServoBounds activeLimits[6];
    
    int16_t currentPositions[6];
    int16_t targetPositions[6];
    unsigned long lastMoveTime[6];
    bool isRelaxed[6];
    unsigned long lastUpdateTime;

    // --- ULEPSZENIE: Dynamiczna prędkość (odblokowana z const) ---
    uint8_t sweepDelay = 15;

    void waitAndInterpolate(unsigned long ms);

public:
    ServoManager();
    bool init();
    void setSoftwareLimits(uint8_t servoId, ServoBounds bounds);
    void setServoAngle(uint8_t servoId, int16_t angle);
    void setServoTicksDirect(uint8_t servoId, int16_t ticks);
    void updateInterpolation();
    void executeCalibrationDance();

    // --- ULEPSZENIE: Funkcja odczytująca suwak ze strony WWW ---
    void setSpeed(uint8_t ms) {
        if (ms >= 2 && ms <= 50) sweepDelay = ms;
    }

    // --- ULEPSZENIE: Ochrona przeciwprzeciążeniowa przekładni ---
    void emergencyStop();
    void moveHomeAll();
};

#endif