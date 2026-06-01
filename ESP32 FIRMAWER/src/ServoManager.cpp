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

    // TWARDY ZAMEK (SafeClamp): Przed nadpisaniem celu wykonujemy ostateczne cięcie wartości do limitów z NVS.
    // Zapewnia to, że cokolwiek (frontend WWW, AI czy zły kod) nie zażąda zniszczenia mechaniki.
    ticks = constrain(ticks, activeLimits[servoId].minTicks, activeLimits[servoId].maxTicks);

    // Zapisujemy nowy, bezpieczny cel w tablicy. Faktyczny ruch wykona asynchroniczna funkcja updateInterpolation()
    targetPositions[servoId] = ticks;
}

// ==============================================================================
// ASYNCHRONICZNY INTERPOLATOR RUCHU (SERCE PŁYNNEJ KINEMATYKI)
// ==============================================================================
void ServoManager::updateInterpolation() {
    // Sprawdzamy czy minął określony czas (15 milisekund). Używamy funkcji millis(),
    // która NIE BLOKUJE procesora w przeciwieństwie do złej funkcji delay().
    if (millis() - lastUpdateTime >= SWEEP_INTERVAL_MS) {
        
        // Aktualizacja stopera dla kolejnej iteracji
        lastUpdateTime = millis();

        // Przeszukujemy (iterujemy) po wszystkich 6 obsługiwanych serwach
        for (uint8_t i = 0; i < 6; i++) {
            
            // Jeśli aktualna pozycja ramienia nie zgadza się z pozycją docelową...
            if (currentPositions[i] != targetPositions[i]) {
                
                // ...wykonaj JEDEN mikrokrok w kierunku celu.
                // Jeśli cel jest większy niż obecna pozycja -> dodaj 1 Ticks.
                // Jeśli cel jest mniejszy -> odejmij 1 Ticks.
                if (currentPositions[i] < targetPositions[i]) {
                    currentPositions[i]++; 
                } else {
                    currentPositions[i]--;
                }

                // Bezpośrednia, sprzętowa komenda do kontrolera PCA9685 nakazująca ustawienie ramienia
                pwm.setPWM(i, 0, currentPositions[i]);
            }
        }
    }
}

void ServoManager::moveHomeAll() {
    Serial.println("[SERVO] Wykonuję rozkaz: Sprowadzenie całej konstrukcji do bezpiecznej pozycji HOME...");
    
    for (uint8_t i = 0; i < 6; i++) {
        targetPositions[i] = activeLimits[i].homeTicks;
        currentPositions[i] = activeLimits[i].homeTicks;
        
        // Zabezpieczenia z NVS gwarantują teraz bezpieczny, twardy start dla każdego stawu
        pwm.setPWM(i, 0, targetPositions[i]);
    }
}