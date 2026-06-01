#ifndef NVS_MANAGER_H
#define NVS_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>
#include "config.h"

struct ServoBounds {
    int16_t minTicks;
    int16_t maxTicks;
    int16_t homeTicks;
};

class NvsManager {
private:
    Preferences preferences;
    bool validateLimits(ServoBounds bounds);

public:
    bool init();
    ServoBounds loadServoLimits(uint8_t servoId);
    bool saveServoLimits(uint8_t servoId, ServoBounds bounds);
};

#endif