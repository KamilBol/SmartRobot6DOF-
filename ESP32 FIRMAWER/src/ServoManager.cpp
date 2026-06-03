#include "ServoManager.h"
#include <Wire.h>

// ==============================================================================
// KONSTRUKTOR KLASY
// ==============================================================================
ServoManager::ServoManager() : pwm(Adafruit_PWMServoDriver()) {
    lastUpdateTime = 0;
    for (int i = 0; i < 6; i++) {
        currentPositions[i] = 307; // Bezpieczny środek (HOME) w Ticks dla SG90
        targetPositions[i] = 307;
        lastMoveTime[i] = 0;
        isRelaxed[i] = false;
    }
}

// ==============================================================================
// INICJALIZACJA MAGISTRALI I SPRZĘTU (PCA9685)
// ==============================================================================
bool ServoManager::init() {
    Serial.println("[SERVO] Inicjalizacja magistrali I2C (SDA:8, SCL:9)...");
    
    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.setClock(400000); // 400 kHz - Fast I2C Mode minimalizuje opóźnienia

    if (!pwm.begin()) {
        Serial.println("[SERVO_ERR] Nie wykryto układu PCA9685!");
        return false;
    }
    
    pwm.setOscillatorFrequency(27000000);
    pwm.setPWMFreq(50);
    
    // Zapobiega szarpnięciu prądowemu (udarowemu) przy rozruchu płyty
    emergencyStop(); 
    
    return true;
}

// ==============================================================================
// OCHRONA I TWARDE LIMITY
// ==============================================================================
void ServoManager::setSoftwareLimits(uint8_t servoId, ServoBounds bounds) {
    if (servoId > 5) return;
    activeLimits[servoId] = bounds;
}

void ServoManager::setServoTicksDirect(uint8_t servoId, int16_t ticks) {
    if (servoId > 5) return;
    
    // Twarda obrona przed uderzeniem o obudowę
    if (ticks < activeLimits[servoId].minTicks) ticks = activeLimits[servoId].minTicks;
    if (ticks > activeLimits[servoId].maxTicks) ticks = activeLimits[servoId].maxTicks;
    
    targetPositions[servoId] = ticks;
    isRelaxed[servoId] = false;
}

void ServoManager::setServoAngle(uint8_t servoId, int16_t angle) {
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;
    int16_t ticks = map(angle, 0, 180, activeLimits[servoId].minTicks, activeLimits[servoId].maxTicks);
    setServoTicksDirect(servoId, ticks);
}

// ==============================================================================
// ASYNCHRONICZNY SILNIK INTERPOLACJI (SERCE UKŁADU KINEMATYCZNEGO)
// ==============================================================================
void ServoManager::updateInterpolation() {
    // Odczyt z dynamicznej prędkości narzuconej ze strony WWW
    if (millis() - lastUpdateTime >= sweepDelay) {
        lastUpdateTime = millis();
        for (int i = 0; i < 6; i++) {
            if (currentPositions[i] != targetPositions[i]) {
                if (currentPositions[i] < targetPositions[i]) currentPositions[i]++;
                else currentPositions[i]--;
                
                pwm.setPWM(i, 0, currentPositions[i]);
                lastMoveTime[i] = millis();
                isRelaxed[i] = false;
            } 
            // Odcięcie zasilania PWM (Auto-Sleep), żeby piny nie buczały, a płyta nie ciągnęła prądu
            else if (!isRelaxed[i] && (millis() - lastMoveTime[i] > 1000)) {
                pwm.setPWM(i, 0, 4096); 
                isRelaxed[i] = true;
            }
        }
    }
}

// ==============================================================================
// NARZĘDZIA POMOCNICZE
// ==============================================================================
void ServoManager::waitAndInterpolate(unsigned long ms) {
    unsigned long start = millis();
    while (millis() - start < ms) {
        updateInterpolation();
        delay(1);
    }
}

// ==============================================================================
// ULEPSZENIE: EMERGENCY STOP (Zabezpieczenie zębatek)
// ==============================================================================
void ServoManager::emergencyStop() {
    Serial.println("[SERVO] E-STOP! Natychmiastowe zrzucenie napiecia z silnikow.");
    for (int i = 0; i < 6; i++) {
        pwm.setPWM(i, 0, 4096); 
        targetPositions[i] = currentPositions[i]; // Skasowanie celu
        isRelaxed[i] = true;
    }
}

// ==============================================================================
// TANIEC DIAGNOSTYCZNY
// ==============================================================================
void ServoManager::executeCalibrationDance() {
    Serial.println("[SERVO] Rozpoczynam sekwencję tańca kalibracyjnego...");

    // Stopy
    setServoTicksDirect(SERVO_L_FOOT, 240);
    setServoTicksDirect(SERVO_R_FOOT, 370);
    waitAndInterpolate(1000);
    
    setServoTicksDirect(SERVO_L_FOOT, 370);
    setServoTicksDirect(SERVO_R_FOOT, 240);
    waitAndInterpolate(1000);
    
    setServoTicksDirect(SERVO_L_FOOT, 307);
    setServoTicksDirect(SERVO_R_FOOT, 307);
    waitAndInterpolate(1000);

    // Nogi
    setServoTicksDirect(SERVO_L_LEG, 240);
    setServoTicksDirect(SERVO_R_LEG, 370);
    waitAndInterpolate(1000);
    
    setServoTicksDirect(SERVO_L_LEG, 370);
    setServoTicksDirect(SERVO_R_LEG, 240);
    waitAndInterpolate(1000);
    
    setServoTicksDirect(SERVO_L_LEG, 307);
    setServoTicksDirect(SERVO_R_LEG, 307);
    waitAndInterpolate(1000);
}
void ServoManager::moveHomeAll() {
    Serial.println("[SERVO] Awaryjny powrót wszystkich serw do pozycji HOME (307).");
    for (int i = 0; i < 6; i++) {
        setServoTicksDirect(i, 307);
    }
}