#include "NvsManager.h"

// ==============================================================================
// METODA INICJUJĄCA PARTYCJĘ
// ==============================================================================
bool NvsManager::init() {
    Serial.println("[NVS] Rozpoczynam montowanie partycji bot_calib...");
    
    // Otwarcie izolowanej przestrzeni nazw "bot_calib" w trybie odczytu/zapisu (false).
    // Używamy natywnego wear-levelingu frameworku ESP-IDF.
    bool success = preferences.begin("bot_calib", false); 
    
    if (!success) {
        Serial.println("[NVS_ERR] Krytyczny błąd! Nie można zamontować partycji pamięci Flash.");
        return false;
    }
    
    Serial.println("[NVS] Partycja kalibracyjna zamontowana poprawnie.");
    return true;
}

// ==============================================================================
// WEWNĘTRZNA FUNKCJA WALIDUJĄCA (SANITY CHECK)
// ==============================================================================
bool NvsManager::validateLimits(ServoBounds bounds) {
    // Twarde, fabryczne ograniczenia przed zablokowaniem przekładni (Stall Current).
    if (bounds.minTicks < 150 || bounds.maxTicks > 450) {
        return false;
    }
    
    // Punkt środkowy (HOME) musi fizycznie znajdować się pomiędzy punktami skrajnymi.
    if (bounds.homeTicks <= bounds.minTicks || bounds.homeTicks >= bounds.maxTicks) {
        return false;
    }
    
    return true;
}

// ==============================================================================
// ODCZYT LIMITÓW DLA KONKRETNEGO SERWA
// ==============================================================================
ServoBounds NvsManager::loadServoLimits(uint8_t servoId) {
    ServoBounds bounds;
    
    // Generowanie kluczy 15-znakowych (wymóg biblioteki Preferences.h) dla danego portu.
    char keyMin[15], keyMax[15], keyHome[15];
    snprintf(keyMin, sizeof(keyMin), "s%d_min", servoId);
    snprintf(keyMax, sizeof(keyMax), "s%d_max", servoId);
    snprintf(keyHome, sizeof(keyHome), "s%d_home", servoId);

    // Pobranie danych. Jeśli wpis nie istnieje w pamięci, ładujemy wartości Fail-Safe.
    bounds.minTicks = preferences.getShort(keyMin, 180);
    bounds.maxTicks = preferences.getShort(keyMax, 430);
    bounds.homeTicks = preferences.getShort(keyHome, 307);

    // Weryfikacja logiczna załadowanych danych z pamięci NVS.
    if (!validateLimits(bounds)) {
        Serial.printf("[NVS_WARN] Złe dane kalibracyjne dla serwa ID:%d! Wymuszam tryb Fail-Safe.\n", servoId);
        bounds.minTicks = 180;
        bounds.maxTicks = 430;
        bounds.homeTicks = 307;
    } else {
        Serial.printf("[NVS] Załadowano poprawnie serwo ID:%d [Min:%d, Max:%d, Home:%d]\n", 
                      servoId, bounds.minTicks, bounds.maxTicks, bounds.homeTicks);
    }
    
    return bounds;
}

// ==============================================================================
// ZAPIS NOWYCH LIMITÓW DO PAMIĘCI FLASH
// ==============================================================================
bool NvsManager::saveServoLimits(uint8_t servoId, ServoBounds bounds) {
    // Ostatnia blokada bezpieczeństwa przed nadpisaniem komórek krzemu.
    if (!validateLimits(bounds)) {
        Serial.printf("[NVS_ERR] Zablokowano próbę zapisu niszczących wartości dla serwa ID:%d!\n", servoId);
        return false;
    }

    char keyMin[15], keyMax[15], keyHome[15];
    snprintf(keyMin, sizeof(keyMin), "s%d_min", servoId);
    snprintf(keyMax, sizeof(keyMax), "s%d_max", servoId);
    snprintf(keyHome, sizeof(keyHome), "s%d_home", servoId);

    // Właściwy zapis parametrów.
    size_t w1 = preferences.putShort(keyMin, bounds.minTicks);
    size_t w2 = preferences.putShort(keyMax, bounds.maxTicks);
    size_t w3 = preferences.putShort(keyHome, bounds.homeTicks);

    // Sprawdzenie, czy zapis powiódł się na poziomie sprzętowym pamięci Flash.
    if (w1 == 0 || w2 == 0 || w3 == 0) {
        Serial.println("[NVS_ERR] Awaria zapisu danych do fizycznego układu Flash!");
        return false;
    }

    Serial.printf("[NVS] Sukces. Nowa kalibracja zapisana dla serwa ID:%d [Min:%d, Max:%d, Home:%d]\n", 
                  servoId, bounds.minTicks, bounds.maxTicks, bounds.homeTicks);
    return true;
}