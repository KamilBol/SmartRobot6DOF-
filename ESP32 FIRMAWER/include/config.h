#ifndef CONFIG_H
#define CONFIG_H

// Mapowanie portów I2C dla ESP32-S3
#define I2C_SDA 8
#define I2C_SCL 9

// Indeksy kanałów PCA9685 dla poszczególnych stawów
#define SERVO_L_FOOT 0
#define SERVO_R_FOOT 1
#define SERVO_L_LEG  2
#define SERVO_R_LEG  3
#define SERVO_L_ARM  4
#define SERVO_R_ARM  5

// Twarde ograniczenia sprzętowe (nieprzekraczalne)
#define HARD_LIMIT_MIN 150
#define HARD_LIMIT_MAX 450

// Pamięć Awaryjna (Magistrala SPI): Czytnik MicroSD [cite: 19-25]
#define SD_CS 10
#define SD_MOSI 11
#define SD_SCK 12
#define SD_MISO 13

// Usta (Magistrala I2S1): Wzmacniacz Audio MAX98357A [cite: 33-38]
#define SPK_BCLK 17
#define SPK_LRC 18
#define SPK_DIN 21

#endif