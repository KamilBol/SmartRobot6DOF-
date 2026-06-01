#include "NetworkManager.h"

// ==============================================================================
// KONSTRUKTOR
// ==============================================================================
// Inicjacja asynchronicznego serwera WWW na porcie 80
NetworkManager::NetworkManager() : server(80), isAPMode(false) {}

// ==============================================================================
// GŁÓWNA FUNKCJA STARTOWA SIECI (AUTODECYZJA)
// ==============================================================================
void NetworkManager::startSystem(NvsManager* nvs) {
    nvsRef = nvs; 
    Serial.println("\n[NET] Inicjalizacja podsystemu sieciowego...");

    String savedSSID = nvsRef->getWiFiSSID();
    String savedPass = nvsRef->getWiFiPass();

    // Sprawdzamy czy robot zna sieć domową
    if (savedSSID.length() > 0) {
        Serial.println("[NET] Znaleziono zapisane poświadczenia Wi-Fi.");
        setupSTA(savedSSID, savedPass);
    } else {
        Serial.println("[NET] Brak zapisanego Wi-Fi. Uruchamiam konfigurator.");
        setupAP();
    }
}

// ==============================================================================
// TRYB STATION (ŁĄCZENIE Z DOMOWYM ROUTEREM)
// ==============================================================================
void NetworkManager::setupSTA(String ssid, String pass) {
    isAPMode = false;
    Serial.printf("[NET_STA] Próba połączenia z domową siecią: %s\n", ssid.c_str());

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());

    // Czekamy na połączenie
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("[NET_STA] SUKCES! Robot podłączony do sieci domowej.");
        Serial.print("[NET_STA] Adres IP w Twojej sieci: ");
        Serial.println(WiFi.localIP());

        // Usługa mDNS (żeby nie wpisywać IP)
        if (MDNS.begin("robot")) {
            Serial.println("[NET_mDNS] Usługa mDNS aktywna! Panel dostępny pod adresem: http://robot.local");
            MDNS.addService("http", "tcp", 80);
        }

        setupEndpoints();
        server.begin();
    } else {
        Serial.println("[NET_ERR] Nie udało się połączyć z domowym Wi-Fi! Złe hasło lub brak zasięgu.");
        Serial.println("[NET_ERR] Uruchamiam Tryb Ratunkowy (Access Point)...");
        setupAP();
    }
}

// ==============================================================================
// URUCHOMIENIE TRYBU ACCESS POINT I CAPTIVE PORTAL
// ==============================================================================
void NetworkManager::setupAP() {
    isAPMode = true;
    Serial.println("[NET_AP] Stawiam własny Punkt Dostępowy: 🤖 _SmartRobot_Setup");

    WiFi.mode(WIFI_AP);
    WiFi.softAP("🤖 _SmartRobot_Setup");

    dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
    setupEndpoints();
    server.begin();
    Serial.println("[NET_AP] Captive Portal aktywny. Czekam na dane logowania.");
}

// ==============================================================================
// DEFINICJA ENDPOINTÓW HTTP (TRASY WWW)
// ==============================================================================
void NetworkManager::setupEndpoints() {
    
    server.onNotFound([](AsyncWebServerRequest *request){
        request->send(200, "text/html", SETUP_HTML);
    });

    server.on("/connect", HTTP_POST, [this](AsyncWebServerRequest *request){
        String ssid = "";
        String pass = "";
        
        if(request->hasParam("ssid", true)) { ssid = request->getParam("ssid", true)->value(); }
        if(request->hasParam("pass", true)) { pass = request->getParam("pass", true)->value(); }

        if (ssid.length() > 0) {
            Serial.println("\n[NET_AP] --- OTRZYMANO DANE KONFIGURACYJNE ---");
            Serial.printf("SSID: %s\n", ssid.c_str());
            
            request->send(200, "text/html", "<h2 style='color:green; text-align:center;'>Dane zapisane! Robot laczy sie z domowa siecia. Odswiez przegladarke za 10 sekund (http://robot.local).</h2>");
            
            // ZAPIS I TWARDY RESTART PROCESORA
            nvsRef->saveWiFi(ssid, pass);
            delay(1000);
            ESP.restart(); 
        } else {
            request->send(400, "text/html", "<h2 style='color:red; text-align:center;'>Blad: Brak nazwy sieci! Wroc i sprobuj ponownie.</h2>");
        }
    });
}

// ==============================================================================
// TAKTOWANIE SIECI (W PĘTLI MAIN)
// ==============================================================================
void NetworkManager::process() {
    if (isAPMode) {
        dnsServer.processNextRequest();
    }
}