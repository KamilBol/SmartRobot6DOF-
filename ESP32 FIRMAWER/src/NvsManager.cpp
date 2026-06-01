#include "NvsManager.h"

// =================================================================================================
// 🎛️  PANEL KONTROLNY: BEZPIECZNE ZAKRESY RUCHU SERWOMECHANIZMÓW (FAIL-SAFE)  🎛️
// =================================================================================================
// >>> HALO! TUTAJ USTAWIAMY ZAKRESY MAKSYMALNE POSZCZEGÓLNYCH SERW <<<
//
// Serwa (180 stopni) zamontowane są na środku (Kąt 90 st. = Ticks 307).
// Odcięto skrajne wychylenia o 10%, aby zablokować możliwość uderzania o obudowę.
//
// ZASADA DZIAŁANIA:
// [LIMIT MIN]  <---------- ŚRODEK (HOME: 307) ---------->  [LIMIT MAX]
// (Blokuje ruch w lewo)                                    (Blokuje ruch w prawo)
//
// Standardowy zakres Ticks dla 180 st. to 150 - 450. 
// Bezpieczny margines z odciętym 10% to: 190 - 410.
// =================================================================================================
const ServoBounds SAFE_DEFAULTS[6] = {
    // ID |  MIN   |  MAX   | HOME (Środek) | OPIS (Zmieniaj te wartości dla każdego serwa z osobna)
    /* 0 */ { 190,     410,     307 },     // ⬅️ Lewa Stopa
    /* 1 */ { 190,     410,     307 },     // ➡️ Prawa Stopa
    /* 2 */ { 190,     410,     307 },     // ⬅️ Lewa Noga
    /* 3 */ { 190,     410,     307 },     // ➡️ Prawa Noga
    /* 4 */ { 190,     410,     307 },     // ⬅️ Lewa Ręka
    /* 5 */ { 190,     410,     307 }      // ➡️ Prawa Ręka
};


// ==============================================================================
// METODA INICJUJĄCA PARTYCJĘ
// ==============================================================================
bool NvsManager::init() {
    Serial.println("[NVS] Rozpoczynam montowanie partycji bot_calib...");
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
    // Twarde ograniczenia chroniące serwa przed Stall Current. 
    // Odcięte dodatkowe 10% zakresu (Granice bezwzględne: 190 - 410)
    if (bounds.minTicks < 190 || bounds.maxTicks > 410) {
        return false;
    }
    
    // Punkt środkowy musi leżeć pomiędzy limitami
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
    
    char keyMin[15], keyMax[15], keyHome[15];
    snprintf(keyMin, sizeof(keyMin), "s%d_min", servoId);
    snprintf(keyMax, sizeof(keyMax), "s%d_max", servoId);
    snprintf(keyHome, sizeof(keyHome), "s%d_home", servoId);

    // Próba odczytu z NVS (z domyślnymi wartościami skrajnymi jako bufor)
    bounds.minTicks = preferences.getShort(keyMin, 0);
    bounds.maxTicks = preferences.getShort(keyMax, 0);
    bounds.homeTicks = preferences.getShort(keyHome, 0);

    // Weryfikacja logiczna załadowanych danych z pamięci.
    // Jeśli NVS miał stare dane (np. 180 i 430), zostaną tu odrzucone!
    if (!validateLimits(bounds)) {
        Serial.printf("[NVS_WARN] Odrzucono stare/błędne dane z NVS dla serwa ID:%d! Ładuję limity z Panelu Kontrolnego.\n", servoId);
        
        // Wstrzyknięcie zaktualizowanych limitów z naszego GUI na górze pliku
        bounds.minTicks = SAFE_DEFAULTS[servoId].minTicks;
        bounds.maxTicks = SAFE_DEFAULTS[servoId].maxTicks;
        bounds.homeTicks = SAFE_DEFAULTS[servoId].homeTicks;
        
        // Nadpisanie pamięci NVS prawidłowymi wartościami
        saveServoLimits(servoId, bounds);
    } else {
        Serial.printf("[NVS] Załadowano poprawne limity z pamięci Flash dla serwa ID:%d\n", servoId);
    }
    
    return bounds;
}

// ==============================================================================
// ZAPIS NOWYCH LIMITÓW DO PAMIĘCI FLASH
// ==============================================================================
bool NvsManager::saveServoLimits(uint8_t servoId, ServoBounds bounds) {
    if (!validateLimits(bounds)) {
        Serial.printf("[NVS_ERR] Zablokowano zapis do NVS! Parametry przekraczają bezpieczne granice robota.\n");
        return false;
    }

    char keyMin[15], keyMax[15], keyHome[15];
    snprintf(keyMin, sizeof(keyMin), "s%d_min", servoId);
    snprintf(keyMax, sizeof(keyMax), "s%d_max", servoId);
    snprintf(keyHome, sizeof(keyHome), "s%d_home", servoId);

    size_t w1 = preferences.putShort(keyMin, bounds.minTicks);
    size_t w2 = preferences.putShort(keyMax, bounds.maxTicks);
    size_t w3 = preferences.putShort(keyHome, bounds.homeTicks);

    if (w1 == 0 || w2 == 0 || w3 == 0) {
        Serial.println("[NVS_ERR] Awaria zapisu danych do układu Flash!");
        return false;
    }

    return true;

}

// ==============================================================================
// OBSŁUGA POŚWIADCZEŃ SIECI WI-FI (NVS)
// ==============================================================================
void NvsManager::saveWiFi(String ssid, String pass) {
    preferences.putString("wifi_ssid", ssid);
    preferences.putString("wifi_pass", pass);
    Serial.println("[NVS] Zapisano nowe poświadczenia Wi-Fi do pamięci Flash.");
}

String NvsManager::getWiFiSSID() {
    return preferences.getString("wifi_ssid", ""); 
}

String NvsManager::getWiFiPass() {
    return preferences.getString("wifi_pass", "");
}

void NvsManager::clearWiFi() {
    preferences.remove("wifi_ssid");
    preferences.remove("wifi_pass");
    Serial.println("[NVS] Trwale usunięto poświadczenia Wi-Fi z pamięci.");
}