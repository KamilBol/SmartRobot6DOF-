#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_PWMServoDriver.h>
#include <driver/i2s.h>
#include "AudioFileSourceSD.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2S.h"

// --- PINOUT MAP ---
// Sensory
#define TOUCH_PIN 4
#define TRIG_PIN 5
#define ECHO_PIN 6

// I2C (Serwa)
#define I2C_SDA 8
#define I2C_SCL 9

// SPI (Karta SD)
#define SD_CS 10
#define SD_MOSI 11
#define SD_SCK 12
#define SD_MISO 13

// I2S0 (Mikrofon)
#define MIC_SCK 14
#define MIC_WS 15
#define MIC_SD 16

// I2S1 (Głośnik)
#define SPK_BCLK 17
#define SPK_LRC 18
#define SPK_DIN 21

// --- OBIEKTY ---
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40, Wire);
AudioFileSourceSD *audioFile = nullptr;
AudioGeneratorWAV *wav = nullptr;
AudioOutputI2S *out = nullptr;

// --- ZWĘŻONE, BEZPIECZNE ZAKRESY SERW (Eliminacja zacięć) ---
#define SERVO_MID 307  // Środek - dokładnie 90 stopni (ok. 1500us)
#define SAFE_MIN  180  // Bezpieczne ok. 30 stopni (ok. 880us)
#define SAFE_MAX  430  // Bezpieczne ok. 150 stopni (ok. 2100us)

// Aktualne pozycje serw (początkowo wszystkie na środku)
int currentPositions[6] = {SERVO_MID, SERVO_MID, SERVO_MID, SERVO_MID, SERVO_MID, SERVO_MID};

void initMicrophone() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = 16000,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = 256,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };
    i2s_pin_config_t pin_config = {
        .bck_io_num = MIC_SCK,
        .ws_io_num = MIC_WS,
        .data_out_num = I2S_PIN_NO_CHANGE, // <-- POPRAWIONA LITERÓWKA
        .data_in_num = MIC_SD
    };
    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
}

void playWAV(const char* filename) {
    if (wav->isRunning()) {
        wav->stop();
    }
    if (audioFile) delete audioFile;

    Serial.printf("[AUDIO] Odtwarzam: %s\n", filename);
    audioFile = new AudioFileSourceSD(filename);
    wav->begin(audioFile, out);
    
    // Pętla odtwarzania (blokująca na czas pliku dla czystości testu)
    while (wav->isRunning()) {
        if (!wav->loop()) wav->stop();
    }
    Serial.println("[AUDIO] Koniec odtwarzania.");
}

// Funkcja realizująca płynny, kontrolowany ruch wielu serw naraz
void moveServosSmooth(int targetPositions[6], int stepDelayMs) {
    Serial.println("[SERWA] Płynny ruch ramion i nóg...");
    bool allReached = false;
    
    while (!allReached) {
        allReached = true;
        for (int i = 0; i < 6; i++) {
            if (currentPositions[i] < targetPositions[i]) {
                currentPositions[i] += 2; // Krok o 2 jednostki PWM
                if (currentPositions[i] > targetPositions[i]) currentPositions[i] = targetPositions[i];
                pwm.setPWM(i, 0, currentPositions[i]);
                allReached = false;
            } 
            else if (currentPositions[i] > targetPositions[i]) {
                currentPositions[i] -= 2;
                if (currentPositions[i] < targetPositions[i]) currentPositions[i] = targetPositions[i];
                pwm.setPWM(i, 0, currentPositions[i]);
                allReached = false;
            }
        }
        delay(stepDelayMs); // Kontrola prędkości ruchu
    }
    Serial.println("[SERWA] Pozycja osiągnięta.");
}

// Sekwencja ruchów testowych: płynny taniec
void executeRobotDance() {
    // 0:L_Stopa, 1:P_Stopa, 2:L_Noga, 3:P_Noga, 4:L_Ręka, 5:P_Ręka
    int step1[6] = {SAFE_MIN, SAFE_MAX, SERVO_MID, SERVO_MID, SAFE_MAX, SAFE_MIN};
    int step2[6] = {SAFE_MAX, SAFE_MIN, SERVO_MID, SERVO_MID, SAFE_MIN, SAFE_MAX};
    int home[6]  = {SERVO_MID, SERVO_MID, SERVO_MID, SERVO_MID, SERVO_MID, SERVO_MID};

    moveServosSmooth(step1, 15);
    delay(200);
    moveServosSmooth(step2, 15);
    delay(200);
    moveServosSmooth(home, 10);
}

float measureDistance() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    long duration = pulseIn(ECHO_PIN, HIGH, 20000);
    if (duration == 0) return -1;
    return duration * 0.034 / 2;
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=== ROBOT 6DOF - DIAGNOSTYKA V2.0 ===");

    // 1. Sensory GPIO
    pinMode(TOUCH_PIN, INPUT_PULLUP);
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    // 2. Magistrala I2C & PCA9685
    Wire.begin(I2C_SDA, I2C_SCL);
    pwm.begin();
    pwm.setOscillatorFrequency(27000000);
    pwm.setPWMFreq(50);
    
    // Wymuszenie pozycji startowej (90 stopni)
    for(int i=0; i<6; i++) {
        pwm.setPWM(i, 0, SERVO_MID);
        currentPositions[i] = SERVO_MID;
    }
    Serial.println("[OK] Sterownik serw ustawiony w pozycji środkowej.");

    // 3. Magistrala SPI & Karta SD
    SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
    if (!SD.begin(SD_CS, SPI)) {
        Serial.println("[ERR] Błąd karty SD!");
    } else {
        Serial.println("[OK] Karta SD gotowa.");
    }

    // 4. Wyjście Audio I2S1 (Głośnik)
    wav = new AudioGeneratorWAV();
    out = new AudioOutputI2S(1, AudioOutputI2S::EXTERNAL_I2S);
    out->SetPinout(SPK_BCLK, SPK_LRC, SPK_DIN);
    out->SetGain(2.0); // Dajemy 100% z cyfrowego źródła, głośność sprzętową dają rezystory 100k
    Serial.println("[OK] System audio wzmocniony na max.");

    // 5. Wejście Audio I2S0 (Mikrofon)
    initMicrophone();
    Serial.println("[OK] Mikrofon nasłuchuje.");
}

void loop() {
    // REAKCJA A: Dotyk (XKC-Y25)
    if (digitalRead(TOUCH_PIN) == LOW) {
        Serial.println("[EVENT] Dotyk wykryty!");
        playWAV("/dotyk.wav");
        executeRobotDance();
        delay(500); 
    }

    // REAKCJA B: Odległość (HC-SR04)
    static unsigned long lastSonarScan = 0;
    if (millis() - lastSonarScan > 1500) {
        float distance = measureDistance();
        if (distance > 0 && distance < 20.0) {
            Serial.printf("[SONAR] Obiekt wykryty: %.1f cm\n", distance);
            executeRobotDance();
        }
        lastSonarScan = millis();
    }

    // REAKCJA C: Nasłuch (Mikrofon)
    int32_t sample = 0;
    size_t bytesRead = 0;
    i2s_read(I2S_NUM_0, &sample, sizeof(sample), &bytesRead, 0);
    if (bytesRead > 0) {
        sample = sample >> 14; 
        if (abs(sample) > 6500) { // Próg hałasu
            Serial.printf("[MIKROFON] Głośny dźwięk: %d\n", abs(sample));
            playWAV("/graj.wav");
            executeRobotDance();
            delay(500);
        }
    }

    // REAKCJA D: Konsola PC
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();
        if (cmd.equalsIgnoreCase("graj")) {
            playWAV("/graj.wav");
            executeRobotDance();
        }
    }
}