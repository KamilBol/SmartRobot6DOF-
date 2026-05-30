#include <Arduino.h>
#include <Wire.h>
#include <SPI.h> 
#include <Adafruit_PWMServoDriver.h>

#define I2C_SDA 21
#define I2C_SCL 47

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define SERVOMIN  150
#define SERVOMAX  600
#define SERVO_FREQ 50

const int BUTTON_PIN = 0; 
const int LED_PIN = 2; 

int currentSequence = 0;

// Obsługa przycisku
bool buttonPressed = false;
bool longPressTriggered = false;
unsigned long buttonDownTime = 0;
const unsigned long LONG_PRESS_TIME = 5000; // 5 sekund do odpalenia finału

// Funkcja uniwersalna: dla serw 180 = kąt, dla serw 360 = prędkość
void setServoAngle(uint8_t n, uint8_t angle) {
  angle = constrain(angle, 0, 180); 
  uint16_t pulse = map(angle, 0, 180, SERVOMIN, SERVOMAX);
  pwm.setPWM(n, 0, pulse);
}

void blinkLED(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(150);
    digitalWrite(LED_PIN, LOW);
    delay(150);
  }
}

// ==========================================
// 80-SEKUNDOWY GRAND SHOW (8 Serw 180° + 2 Serwa 360°)
// Piny 0-7: Ramiona / Elementy ruchome 180°
// Piny 8-9: Rotory / Radary / Koła 360°
// ==========================================
void seqFestival() {
  Serial.println("--- START CHOREOGRAFII: GRAND FESTIVAL (~80s) ---");
  
  // Stroboskop startowy
  for(int i=0; i<15; i++) { digitalWrite(LED_PIN, HIGH); delay(40); digitalWrite(LED_PIN, LOW); delay(40); }
  delay(500);

  // === FAZA 1: Budzenie Maszyny (~10s) ===
  // Rotory powoli ruszają, reszta serw majestatycznie rozkłada się na boki
  setServoAngle(8, 80); // Rotor 1 wolno w lewo
  setServoAngle(9, 100); // Rotor 2 wolno w prawo
  for(int angle=90; angle<=180; angle+=1) {
    for(int i=0; i<8; i++) setServoAngle(i, angle);
    delay(40); 
  }
  delay(1000);
  for(int angle=180; angle>=0; angle-=2) {
    for(int i=0; i<8; i++) setServoAngle(i, angle);
    delay(30); 
  }
  delay(1000);

  // === FAZA 2: Radar i Szperacze (~10s) ===
  // Rotory na maxa w jednym kierunku, serwa 180 "skanują" teren
  setServoAngle(8, 0); // Max speed
  setServoAngle(9, 0); // Max speed
  for(int iter=0; iter<3; iter++) {
    for(int i=0; i<8; i++) { setServoAngle(i, 45); delay(100); }
    for(int i=7; i>=0; i--) { setServoAngle(i, 135); delay(100); }
  }
  setServoAngle(8, 180); // Nagła zmiana kierunku rotacji
  setServoAngle(9, 180);
  for(int i=0; i<8; i++) setServoAngle(i, 90);
  delay(2000);

  // === FAZA 3: Synchronizacja Falowa (~10s) ===
  // Pływający wąż ramion, rotory pulsują kierunkami
  for(int iter = 0; iter < 4; iter++) {
    setServoAngle(8, (iter%2==0)? 0 : 180); // Zmiana rotacji z każdą falą
    setServoAngle(9, (iter%2==0)? 180 : 0);
    
    for(int i=0; i<8; i++) { setServoAngle(i, 180); delay(60); }
    for(int i=0; i<8; i++) { setServoAngle(i, 0); delay(60); }
  }

  // === FAZA 4: Pełny Asynchroniczny Chaos (~12s) ===
  // Każdy silnik, łącznie z rotorami, wariuje całkowicie niezależnie
  for(int step=0; step<50; step++) {
    for(int i=0; i<8; i++) {
      setServoAngle(i, random(10, 170)); 
    }
    // Rotory losują prędkości i kierunki (od 0 do 180)
    setServoAngle(8, random(0, 181)); 
    setServoAngle(9, random(0, 181));
    delay(200); 
  }

  // === FAZA 5: Marsz Mechaniczny / Tłoki (~10s) ===
  // Agresywne uderzenia, rotory robią szybkie zrywy stop/start
  for(int step=0; step<8; step++) {
    setServoAngle(8, 0); setServoAngle(9, 180); // Ogień
    for(int i=0; i<8; i+=2) setServoAngle(i, 180);
    for(int i=1; i<8; i+=2) setServoAngle(i, 0);
    delay(500);

    setServoAngle(8, 90); setServoAngle(9, 90); // Hamulec (stop)
    for(int i=0; i<8; i+=2) setServoAngle(i, 0);
    for(int i=1; i<8; i+=2) setServoAngle(i, 180);
    delay(500);
  }

  // === FAZA 6: Zapadanie i Wibracje (~8s) ===
  // Bardzo drobne drgania udające przeciążenie układu
  setServoAngle(8, 85); // Bardzo powolne, dławiące się obroty
  setServoAngle(9, 95);
  for(int step=0; step<40; step++) {
    for(int i=0; i<8; i++) setServoAngle(i, 85 + random(-10, 10)); delay(50);
    for(int i=0; i<8; i++) setServoAngle(i, 95 + random(-10, 10)); delay(50);
  }

  // === FAZA 7: Powolny Rozjazd Lustrzany (~10s) ===
  // Od środka na zewnątrz
  setServoAngle(8, 180); setServoAngle(9, 0); 
  for(int i=0; i<4; i++) {
    setServoAngle(3-i, 0);   // Lewa strona się kładzie
    setServoAngle(4+i, 180); // Prawa strona się kładzie
    delay(800);
  }
  delay(1500); 

  // === FAZA 8: Odrzut Finałowy i Zatrzymanie (~10s) ===
  // Błyskawiczne zbiegnięcie do środka, zatrzymanie rotorów
  for(int i=0; i<8; i++) setServoAngle(i, 90);
  setServoAngle(8, 90); // Zatrzymanie obrotu 360
  setServoAngle(9, 90); // Zatrzymanie obrotu 360
  
  // Dioda gaśnie, udajemy odcięcie prądu na 3 sekundy
  digitalWrite(LED_PIN, LOW);
  delay(3000);

  // Szybki, końcowy "tik" powrotny 
  for(int i=0; i<8; i++) { setServoAngle(i, 100); delay(20); setServoAngle(i, 90); }
  
  Serial.println("--- KONIEC CHOREOGRAFII ---");
}

// ==========================================
// 10 STANDARDOWYCH SEKWENCJI (Krótkie kliknięcia)
// (Zabezpieczone - działają tylko na serwach 0-7, rotory 8-9 odpoczywają)
// ==========================================

void seq1() {
  for(int angle = 0; angle <= 180; angle += 5) {
    for(int i = 0; i < 8; i++) setServoAngle(i, angle);
    delay(20);
  }
  delay(300);
  for(int angle = 180; angle >= 0; angle -= 5) {
    for(int i = 0; i < 8; i++) setServoAngle(i, angle);
    delay(20);
  }
}

void seq2() {
  for(int i = 0; i < 8; i++) {
    setServoAngle(i, 180); delay(200); setServoAngle(i, 0);
  }
}

void seq3() {
  for (int step = 0; step < 4; step++) {
    for(int i = 0; i < 8; i++) setServoAngle(i, (i % 2 == 0) ? 180 : 0); delay(400);
    for(int i = 0; i < 8; i++) setServoAngle(i, (i % 2 == 0) ? 0 : 180); delay(400);
  }
}

void seq4() {
  for(int i=0; i<8; i++) setServoAngle(i, 90); delay(300);
  for(int step=0; step<4; step++) {
    setServoAngle(3-step, 0); setServoAngle(4+step, 180); delay(400);
  }
}

void seq5() {
  for(int step = 0; step < 10; step++) {
    for(int i = 0; i < 8; i++) setServoAngle(i, random(0, 181)); delay(250);
  }
}

void seq6() {
  for(int step = 0; step < 3; step++) {
    for(int angle = 60; angle <= 120; angle++) { for(int i = 0; i < 8; i++) setServoAngle(i, angle); delay(15); }
    for(int angle = 120; angle >= 60; angle--) { for(int i = 0; i < 8; i++) setServoAngle(i, angle); delay(15); }
  }
}

void seq7() {
  for(int step = 0; step < 15; step++) {
    for(int i = 0; i < 8; i++) setServoAngle(i, 80); delay(60);
    for(int i = 0; i < 8; i++) setServoAngle(i, 100); delay(60);
  }
}

void seq8() {
  for(int i = 0; i < 8; i++) { setServoAngle(i, 180); delay(150); }
  for(int i = 7; i >= 0; i--) { setServoAngle(i, 0); delay(150); }
}

void seq9() {
  for(int i = 0; i < 8; i++) setServoAngle(i, 180);
  for(int step = 0; step < 2; step++) {
    for(int i = 0; i < 8; i++) { setServoAngle(i, 0); delay(200); setServoAngle(i, 180); }
  }
}

void seq10() {
  for(int i = 0; i < 8; i++) setServoAngle(i, 90); delay(1500);
}

// ==========================================
// SETUP
// ==========================================

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  Wire.begin(I2C_SDA, I2C_SCL); 
  pwm.begin();
  pwm.setOscillatorFrequency(27000000); 
  pwm.setPWMFreq(SERVO_FREQ);

  // Ustawiamy WSZYSTKIE 10 serw na pozycję neutralną (90 stopni dla 180°, STOP dla 360°)
  for(uint8_t i = 0; i < 10; i++) {
    setServoAngle(i, 90);
  }
  delay(500);

  Serial.println("System gotowy!");
  blinkLED(5); 
}

// ==========================================
// LOOP
// ==========================================

void loop() {
  int buttonState = digitalRead(BUTTON_PIN);

  if (buttonState == LOW) {
    if (!buttonPressed) {
      buttonPressed = true;
      buttonDownTime = millis();
      longPressTriggered = false;
    } 
    // Sprawdzanie czy przycisk jest wciśnięty >= 5 sekund
    else if (!longPressTriggered && (millis() - buttonDownTime >= LONG_PRESS_TIME)) {
      longPressTriggered = true; 
      seqFestival(); 
    }
  } 
  else {
    if (buttonPressed) {
      unsigned long pressDuration = millis() - buttonDownTime;
      buttonPressed = false;
      
      // Krótkie kliknięcie obsługuje standardowe sekwencje
      if (!longPressTriggered && pressDuration > 50) { 
        currentSequence++;
        if (currentSequence > 10) {
          currentSequence = 1; 
        }

        Serial.print("Wybrano sekwencje: ");
        Serial.println(currentSequence);

        blinkLED(currentSequence);

        switch (currentSequence) {
          case 1: seq1(); break;
          case 2: seq2(); break;
          case 3: seq3(); break;
          case 4: seq4(); break;
          case 5: seq5(); break;
          case 6: seq6(); break;
          case 7: seq7(); break;
          case 8: seq8(); break;
          case 9: seq9(); break;
          case 10: seq10(); break;
        }
      }
    }
  }
}