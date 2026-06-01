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

#endif