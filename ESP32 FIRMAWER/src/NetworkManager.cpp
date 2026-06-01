#include "NetworkManager.h"

// ==============================================================================
// KONSTRUKTOR
// ==============================================================================
// Inicjacja asynchronicznego serwera WWW na porcie 80
NetworkManager::NetworkManager() : server(80), isAPMode(false) {}

// ==============================================================================
// GŁÓWNA FUNKCJA STARTOWA SIECI
// ==============================================================================
void NetworkManager::startSystem(NvsManager* nvs) {
    nvsRef = nvs; // Przypisanie wskaźnika pamięci Flash
    
    Serial.println("\n[NET] Inicjalizacja podsystemu sieciowego...");

    // UWAGA: Tutaj w przyszłości dodamy logikę odczytu zapisanego hasła z NVS.
    // Na razie ZAWSZE wymuszamy tryb Access Point, byś mógł przetestować Captive Portal!
    setupAP();
}

// ==============================================================================
// URUCHOMIENIE TRYBU ACCESS POINT I CAPTIVE PORTAL
// ==============================================================================
void NetworkManager::setupAP() {
    isAPMode = true;
    Serial.println("[NET_AP] Przechodzę w tryb Access Point (Punkt Dostępowy).");

    // Tworzenie otwartej sieci Wi-Fi bez hasła
    WiFi.mode(WIFI_AP);
    WiFi.softAP("🤖 _SmartRobot_Setup");

    Serial.print("[NET_AP] Adres IP robota: ");
    Serial.println(WiFi.softAPIP());

    // Uruchomienie serwera DNS (nasłuch na porcie 53)[cite: 186].
    // Gwiazdka "*" oznacza, że KAŻDE zapytanie z Twojego telefonu o dowolną 
    // stronę (np. google.com) zostanie przekierowane na IP robota (Captive Portal)[cite: 186, 187].
    dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

    // Konfiguracja tras URL
    setupEndpoints();

    // Start serwera asynchronicznego
    server.begin();
    Serial.println("[NET_AP] Captive Portal aktywny. Połącz się z siecią robota!");
}

// ==============================================================================
// DEFINICJA ENDPOINTÓW HTTP (TRASY WWW)
// ==============================================================================
void NetworkManager::setupEndpoints() {
    
    // Główny Handler dla Captive Portal (ON_NOT_FOUND) [cite: 188]
    // Jeśli telefon szuka czegokolwiek w sieci, ładujemy mu naszą stronę z PROGMEM[cite: 189].
    server.onNotFound([](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", SETUP_HTML);
    });

    // Endpoint odbierający formularz z nowymi danymi Wi-Fi (żądanie POST) [cite: 190]
    server.on("/connect", HTTP_POST, [](AsyncWebServerRequest *request){
        String ssid = "";
        String pass = "";
        
        // Wyciąganie parametrów z zapytania
        if(request->hasParam("ssid", true)) { ssid = request->getParam("ssid", true)->value(); }
        if(request->hasParam("pass", true)) { pass = request->getParam("pass", true)->value(); }

        Serial.println("\n[NET_AP] --- OTRZYMANO DANE KONFIGURACYJNE ---");
        Serial.printf("SSID: %s\n", ssid.c_str());
        Serial.printf("PASS: %s\n", pass.c_str());
        Serial.println("----------------------------------------------\n");

        request->send(200, "text/html", "<h2 style='color:green; text-align:center;'>Dane odebrane! Sprawdz monitor portu szeregowego.</h2>");
        
        // W przyszłości tutaj przekażemy te dane do NvsManager i zrobimy ESP.restart() [cite: 191]
    });
}

// ==============================================================================
// TAKTOWANIE SIECI (W PĘTLI MAIN)
// ==============================================================================
void NetworkManager::process() {
    // Serwer DNS (odpowiedzialny za Captive Portal) wymaga ciągłego odświeżania w trybie AP [cite: 217]
    if (isAPMode) {
        dnsServer.processNextRequest();
    }
}