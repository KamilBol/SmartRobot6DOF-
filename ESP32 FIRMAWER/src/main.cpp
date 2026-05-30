#include <Arduino.h>
#include <Wire.h>
#include <SPI.h> 
#include <Adafruit_PWMServoDriver.h>

// Piny magistrali I2C dla Twojego ESP32-S3
#define I2C_SDA 21
#define I2C_SCL 47

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

// Parametry impulsów PWM dla serw
#define SERVOMIN  150
#define SERVOMAX  600
#define SERVO_FREQ 50

const int LED_PIN = 2; 

// Funkcja ustawiająca kąt serwa
void setServoAngle(uint8_t n, uint8_t angle) {
  angle = constrain(angle, 0, 180); 
  uint16_t pulse = map(angle, 0, 180, SERVOMIN, SERVOMAX);
  pwm.setPWM(n, 0, pulse);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  // Uruchomienie I2C i modułu PCA9685
  Wire.begin(I2C_SDA, I2C_SCL); 
  pwm.begin();
  pwm.setOscillatorFrequency(27000000); 
  pwm.setPWMFreq(SERVO_FREQ);

  Serial.println("Rozpoczynam centrowanie serw...");
  digitalWrite(LED_PIN, HIGH);

  // Ustawienie 10 pierwszych pinów (0-9) na idealny środek (90 stopni)
  for(uint8_t i = 0; i < 10; i++) {
    setServoAngle(i, 90);
    delay(50); // Krótkie opóźnienie, żeby nie szarpnąć zasilaczem przy starcie wszystkich naraz
  }

  Serial.println("Serwa zablokowane na 90 stopniach. Mozesz zakladac orczyki.");
}

void loop() {
  // Pusta pętla - system po prostu trzyma pozycję 90 stopni na wszystkich kanałach
  delay(1000);
}