#include <Arduino.h>
#include <driver/i2s.h>
#include "config.h"
#include "NvsManager.h"
#include "ServoManager.h"
#include "NetworkManager.h"
#include "AudioManager.h"

// ==============================================================================
// DEFINICJA MASZYNY STANÓW
// ==============================================================================
enum class RobotState : uint8_t {
    BOOT,
    IDLE,
    AUTONOMOUS_REACTION
};

RobotState currentState = RobotState::BOOT;
NvsManager nvs;
ServoManager servo;
NetworkManager net;
AudioManager audio;

void transitionTo(RobotState newState) {
    if (currentState == newState) return;
    Serial.printf("[STATE] Zmiana stanu systemu: %d -> %d\n",
                  static_cast<uint8_t>(currentState), static_cast<uint8_t>(newState));
    currentState = newState;
}

// ==============================================================================
// INICJALIZACJA ZMYSŁU SŁUCHU (MIKROFON I2S0)
// ==============================================================================
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
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = MIC_SD
    };
    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
    Serial.println("[MIC] Zmysł słuchu (INMP441) zainicjowany na szynie I2S0.");
}

// ==============================================================================
// BOOT SEQUENCE
// ==============================================================================
void setup() {
    Serial.begin(115200);
    delay(1000);

    // Konfiguracja pinów wejścia/wyjścia
    pinMode(TOUCH_PIN, INPUT_PULLUP);
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    if (!nvs.init()) Serial.println("[ERR] Błąd pamięci NVS.");
    if (!servo.init()) Serial.println("[ERR] Błąd kontrolera PCA9685.");

    // Pobranie i wstrzyknięcie zaktualizowanych limitów do serw
    for (uint8_t i = 0; i < 6; i++) {
        ServoBounds bounds = nvs.loadServoLimits(i);
        servo.setSoftwareLimits(i, bounds);
    }
    servo.moveHomeAll();

    if (!audio.init()) Serial.println("[ERR] Błąd modułu Audio/SD.");
    initMicrophone();
    net.startSystem(&nvs, &servo); // Przekazujemy wskaźnik do serw, aby WebApp mógł nimi ruszać

    transitionTo(RobotState::IDLE);
    Serial.println("[SYSTEM] Operacja zasilania ukończona. Robot w trybie nasłuchu (IDLE).");
}

// ==============================================================================
// ASYNCHRONICZNA PĘTLA GŁÓWNA (ZERO-BLOCKING ROUTINE)
// ==============================================================================
void loop() {
    // 1. Priorytetowe, sprzętowe odświeżanie dźwięku i ruchu
    audio.process();
    servo.updateInterpolation();
    net.process();

    // 2. Router Reakcji Sensorycznych
    switch (currentState) {
        case RobotState::IDLE: {
            // REAKCJA A: Dotyk głowy (Czujnik Pojemnościowy)
            if (digitalRead(TOUCH_PIN) == LOW) {
                Serial.println("[ZMYSL] Wykryto dotyk! Odtwarzam 'dotyk.wav'");
                audio.playFile("/dotyk.wav");
                servo.setServoTicksDirect(SERVO_L_ARM, 200);
                servo.setServoTicksDirect(SERVO_R_ARM, 410);
                transitionTo(RobotState::AUTONOMOUS_REACTION);
                break;
            }

            // REAKCJA B: Odległość (Radar Ultradźwiękowy)
            // Stoper odpytuje radar co 1 sekundę, chroniąc przepustowość danych audio
            static unsigned long lastSonarTime = 0;
            if (millis() - lastSonarTime > 1000) {
                lastSonarTime = millis();
                digitalWrite(TRIG_PIN, LOW);
                delayMicroseconds(2);
                digitalWrite(TRIG_PIN, HIGH);
                delayMicroseconds(10);
                digitalWrite(TRIG_PIN, LOW);
                
                // Odczyt pulsu, timeout ustawiony na bezpieczne 20 milisekund (około 3 metry)
                long duration = pulseIn(ECHO_PIN, HIGH, 20000);
                if (duration > 0) {
                    float dist = duration * 0.034 / 2;
                    if (dist > 0 && dist < 20.0) { // Reaguj poniżej 20 centymetrów
                        Serial.printf("[ZMYSL] Zbliżenie obiektu: %.1f cm\n", dist);
                        audio.playFile("/graj.wav");
                        servo.setServoTicksDirect(SERVO_L_ARM, 410);
                        servo.setServoTicksDirect(SERVO_R_ARM, 200);
                        transitionTo(RobotState::AUTONOMOUS_REACTION);
                        break;
                    }
                }
            }

            // REAKCJA C: Hałas akustyczny (Mikrofon I2S)
            int32_t sample = 0;
            size_t bytesRead = 0;
            i2s_read(I2S_NUM_0, &sample, sizeof(sample), &bytesRead, 0);
            if (bytesRead > 0) {
                sample = sample >> 14; 
                if (abs(sample) > 6500) { // Skok amplitudy sygnału (np. klaśnięcie)
                    Serial.println("[ZMYSL] Klaśnięcie wykryte przez mikrofon!");
                    audio.playFile("/graj.wav");
                    servo.setServoTicksDirect(SERVO_L_ARM, 180);
                    servo.setServoTicksDirect(SERVO_R_ARM, 430);
                    transitionTo(RobotState::AUTONOMOUS_REACTION);
                    break;
                }
            }
            break;
        }

        case RobotState::AUTONOMOUS_REACTION: {
            // Asynchroniczny stoper podtrzymujący trwanie gestu
            static unsigned long reactionStart = 0;
            if (reactionStart == 0) reactionStart = millis();

            // Po upływie 2.5 sekundy, stawy powracają do spoczynku a system wznawia nasłuch
            if (millis() - reactionStart > 2500) {
                reactionStart = 0;
                servo.moveHomeAll();
                transitionTo(RobotState::IDLE);
            }
            break;
        }

        default:
            break;
    }
}