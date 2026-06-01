#include "ServoManager.h"
#include <Wire.h>

// ==============================================================================
// KONSTRUKTOR KLASY
// ==============================================================================
ServoManager::ServoManager() : pwm(Adafruit_PWMServoDriver()) {
    // Inicjalizacja tablic zmiennych zerami
    lastUpdateTime = 0;
    for (int i = 0; i < 6; i++) {
        currentPositions[i] = 307; // 307 to bezpieczny środek (HOME) w Ticks dla SG90
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
    
    // Inicjalizacja I2C na pinach zdefiniowanych w config.h
    Wire.begin(I2C_SDA, I2C_SCL);
    
    // PCA9685 obsługuje Fast I2C Mode (400 kHz), co drastycznie skraca czas komunikacji i opóźnienia
    Wire.setClock(400000); 

    // Uruchomienie układu PCA9685
    pwm.begin();
    
    // Standardowa częstotliwość dla serwomechanizmów modelarskich to 50 Hz (1 cykl co 20ms)
    pwm.setPWMFreq(50);
    
    Serial.println("[SERVO] Sterownik PCA9685 zainicjowany poprawnie z częstotliwością 50Hz.");
    return true;
}

// ==============================================================================
// WSTRZYKNIĘCIE LIMITÓW ODCZYTANYCH Z PAMIĘCI FLASH
// ==============================================================================
void ServoManager::setSoftwareLimits(uint8_t servoId, ServoBounds bounds) {
    if (servoId > 5) return; // Zabezpieczenie na wypadek wywołania nieistniejącego portu PCA (mamy porty 0-5)
    
    // Kopiujemy załadowane z pamięci NVS limity do pamięci podręcznej RAM (tablicy activeLimits)
    activeLimits[servoId] = bounds;
    
    Serial.printf("[SERVO_CFG] Aktywowano programowe limity dla serwa ID:%d [Min:%d, Max:%d, Home:%d]\n", 
                  servoId, bounds.minTicks, bounds.maxTicks, bounds.homeTicks);
}

// ==============================================================================
// USTAWIENIE POZYCJI W STOPNIACH KĄTOWYCH (0-180) Z WALIDACJĄ
// ==============================================================================
void ServoManager::setServoAngle(uint8_t servoId, int16_t angle) {
    if (servoId > 5) return;

    // Bezpieczne przycięcie wartości kąta, aby na pewno nie wykroczył poza 0-180 stopni
    angle = constrain(angle, 0, 180);

    // Rzutowanie logicznych stopni (0-180) na fizyczne, sprzętowe jednostki sygnału PWM układu PCA9685 (Ticks).
    // Wykorzystujemy indywidualne, kalibrowane limity wczytane z pamięci NVS dla tego konkretnego stawu.
    int16_t targetTicks = map(angle, 0, 180, activeLimits[servoId].minTicks, activeLimits[servoId].maxTicks);

    // Wywołujemy funkcję asynchroniczną przypisującą cel (target).
    setServoTicksDirect(servoId, targetTicks);
}

// ==============================================================================
// BEZPOŚREDNIE USTAWIENIE POZYCJI DOCELOWEJ (TICKS)
// ==============================================================================
void ServoManager::setServoTicksDirect(uint8_t servoId, int16_t ticks) {
    if (servoId > 5) return;

    ticks = constrain(ticks, activeLimits[servoId].minTicks, activeLimits[servoId].maxTicks);
    
    // Jeśli zlecamy serwu nowy cel, wybudzamy je z uśpienia (wyłączamy relax)
    if (targetPositions[servoId] != ticks || currentPositions[servoId] != ticks) {
        isRelaxed[servoId] = false;
    }

    targetPositions[servoId] = ticks;
}

// ==============================================================================
// ASYNCHRONICZNY INTERPOLATOR RUCHU (SERCE PŁYNNEJ KINEMATYKI)
// ==============================================================================
void ServoManager::updateInterpolation() {
    if (millis() - lastUpdateTime >= SWEEP_INTERVAL_MS) {
        lastUpdateTime = millis();

        for (uint8_t i = 0; i < 6; i++) {
            // Jeśli serwo NIE JEST w celu, wykonujemy mikrokrok (ruch)
            if (currentPositions[i] != targetPositions[i]) {
                isRelaxed[i] = false; // Wybudzenie podczas ruchu
                
                if (currentPositions[i] < targetPositions[i]) {
                    currentPositions[i]++; 
                } else {
                    currentPositions[i]--;
                }
                pwm.setPWM(i, 0, currentPositions[i]);
                lastMoveTime[i] = millis(); // Resetujemy stoper uśpienia
            } 
            // Jeśli serwo OSIĄGNĘŁO CEL, czeka 1 sekundę i się usypia (ODCIĘCIE BZYCZENIA)
            else {
                if (!isRelaxed[i] && (millis() - lastMoveTime[i] > 1000)) {
                    // Komenda 4096 dla PCA9685 to sygnał sprzętowy FULL-OFF (Całkowite odcięcie sygnału PWM)
                    pwm.setPWM(i, 0, 4096); 
                    isRelaxed[i] = true;
                    Serial.printf("[SERVO] Serwo ID:%d w pozycji celu. Odcięto prąd (Auto-Sleep).\n", i);
                }
            }
        }
    }
}

// ==============================================================================
// WYMUSZENIE BEZPIECZNEJ POZYCJI DOMOWEJ (HOME) DLA CAŁEGO KORPUSU
// ==============================================================================
void ServoManager::moveHomeAll() {
    Serial.println("[SERVO] Wykonuję rozkaz: Sprowadzenie całej konstrukcji do bezpiecznej pozycji HOME...");
    
    for (uint8_t i = 0; i < 6; i++) {
        targetPositions[i] = activeLimits[i].homeTicks;
        currentPositions[i] = activeLimits[i].homeTicks;
        
        // Twardy start i reset blokady czasowej
        isRelaxed[i] = false;
        lastMoveTime[i] = millis();
        pwm.setPWM(i, 0, targetPositions[i]);
    }
}// Pomocnicza funkcja blokująca, która pozwala na ruch asynchroniczny
void ServoManager::waitAndInterpolate(unsigned long ms) {
    unsigned long start = millis();
    while (millis() - start < ms) {
        updateInterpolation();
        delay(1); // Mała pauza, żeby watchdog nie zwariował
    }
}

// ==============================================================================
// SEKWENCJA TESTOWA (TANIEC DIAGNOSTYCZNY)
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

    // Ręce (Większy ruch)
    setServoTicksDirect(SERVO_L_ARM, 220);
    setServoTicksDirect(SERVO_R_ARM, 390);
    waitAndInterpolate(1000);
    
    setServoTicksDirect(SERVO_L_ARM, 390);
    setServoTicksDirect(SERVO_R_ARM, 220);
    waitAndInterpolate(1000);
    
    setServoTicksDirect(SERVO_L_ARM, 307);
    setServoTicksDirect(SERVO_R_ARM, 307);
    waitAndInterpolate(1500); // Czas na powrót i uspokojenie
    
    Serial.println("[SERVO] Taniec zakończony.");
}