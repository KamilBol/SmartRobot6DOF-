#include "NetworkManager.h"
#include <SD.h> // Wymagane do wgrywania plików audio

NetworkManager::NetworkManager() : server(80), ws("/ws"), isAPMode(false) {}

void NetworkManager::startSystem(NvsManager* nvs, ServoManager* servo) {
    nvsRef = nvs; 
    servoRef = servo;
    Serial.println("\n[NET] Inicjalizacja podsystemu sieciowego...");

    String savedSSID = nvsRef->getWiFiSSID();
    String savedPass = nvsRef->getWiFiPass();

    if (savedSSID.length() > 0) {
        Serial.println("[NET] Znaleziono zapisane poświadczenia Wi-Fi.");
        setupSTA(savedSSID, savedPass);
    } else {
        Serial.println("[NET] Brak zapisanego Wi-Fi. Uruchamiam konfigurator.");
        setupAP();
    }
}

void NetworkManager::setupSTA(String ssid, String pass) {
    isAPMode = false;
    Serial.printf("[NET_STA] Próba połączenia z: %s\n", ssid.c_str());

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("[NET_STA] SUKCES! Podłączono do sieci domowej.");
        Serial.print("[NET_STA] Adres IP: ");
        Serial.println(WiFi.localIP());

        if (MDNS.begin("robot")) {
            Serial.println("[NET_mDNS] Panel gotowy pod adresem: http://robot.local");
            MDNS.addService("http", "tcp", 80);
            MDNS.addService("ws", "tcp", 80); // Rejestracja WebSockets
        }

        setupEndpoints();
        server.begin();
    } else {
        Serial.println("[NET_ERR] Nie udało się połączyć z Wi-Fi!");
        setupAP();
    }
}

void NetworkManager::setupAP() {
    isAPMode = true;
    Serial.println("[NET_AP] Stawiam Punkt Dostępowy: 🤖 _SmartRobot_Setup");
    WiFi.mode(WIFI_AP);
    WiFi.softAP("🤖 _SmartRobot_Setup");
    dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
    setupEndpoints();
    server.begin();
}

void NetworkManager::setupEndpoints() {
    // 1. Zabezpieczenie przed atakami (CORS)
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");

    // 2. PARSER WEBSOCKETS (STEROWANIE SERWAMI LIVE)
    ws.onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len){
        if(type == WS_EVT_DATA){
            AwsFrameInfo *info = (AwsFrameInfo*)arg;
            if(info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT){
                data[len] = 0;
                String msg = (char*)data;
                // Jeśli wiadomość zaczyna się od 'S' i ma ':', to jest to komenda np. "S4:350" (Serwo 4, Pozycja 350)
                if(msg.startsWith("S") && msg.indexOf(":") > 0){
                    uint8_t id = msg.substring(1, msg.indexOf(":")).toInt();
                    int16_t val = msg.substring(msg.indexOf(":")+1).toInt();
                    if(servoRef) servoRef->setServoTicksDirect(id, val); // Natychmiastowy ruch serwa!
                }
                if(servoRef) servoRef->setServoTicksDirect(id, val); // Natychmiastowy ruch serwa!
                }
                // === WKLEJASZ TEN NOWY FRAGMENT PONIŻEJ ===
                else if (msg.startsWith("SPD:")) {
                    uint8_t spd = msg.substring(4).toInt();
                    if(servoRef) servoRef->setSpeed(spd);
                    Serial.printf("[SERVO] Zmiana predkosci globalnej na: %d ms\n", spd);
                }
                // === KONIEC NOWEGO FRAGMENTU ===
                // Obsługa zapisu limitów do pamięci NVS
                else if (msg.startsWith("CFG:")) {
                    int p1 = msg.indexOf(':');
                    int p2 = msg.indexOf(':', p1 + 1);
                    int p3 = msg.indexOf(':', p2 + 1);
                    int p4 = msg.indexOf(':', p3 + 1);
                    
                    uint8_t id = msg.substring(p1 + 1, p2).toInt();
                    ServoBounds bounds;
                    bounds.minTicks = msg.substring(p2 + 1, p3).toInt();
                    bounds.maxTicks = msg.substring(p3 + 1, p4).toInt();
                    bounds.homeTicks = msg.substring(p4 + 1).toInt();
                    
                    if(nvsRef) nvsRef->saveServoLimits(id, bounds);
                    if(servoRef) servoRef->setSoftwareLimits(id, bounds);
                }
            }
        }
    });
    server.addHandler(&ws); // Podpięcie serwera czasu rzeczywistego

    // 3. Główna strona zależna od trybu (AP -> Setup, STA -> WebApp)
    server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request){
        if (isAPMode) request->send(200, "text/html", SETUP_HTML);
        else request->send(200, "text/html", INDEX_HTML);
    });

    server.onNotFound([this](AsyncWebServerRequest *request){
        if (isAPMode) request->send(200, "text/html", SETUP_HTML);
        else request->send(404, "text/plain", "Nie znaleziono pliku");
    });

    // 4. API DO ODBIERANIA DANYCH LOGOWANIA
    server.on("/connect", HTTP_POST, [this](AsyncWebServerRequest *request){
        String ssid = ""; String pass = "";
        if(request->hasParam("ssid", true)) ssid = request->getParam("ssid", true)->value();
        if(request->hasParam("pass", true)) pass = request->getParam("pass", true)->value();

        if (ssid.length() > 0) {
            request->send(200, "text/html", "<h2 style='color:green; text-align:center;'>Restartowanie systemu... Odswiez przegladarke za chwile.</h2>");
            nvsRef->saveWiFi(ssid, pass);
            delay(1000);
            ESP.restart(); 
        } else {
            request->send(400, "text/plain", "Brak nazwy sieci.");
        }
    });

    // 5. ASYNCHRONICZNY SERWER PLIKÓW - WGRYWANIE MP3/WAV NA KARTĘ SD (Etap 3)
    server.on("/api/upload", HTTP_POST, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "Upload zakonczony sukcesem");
    }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
        // Zabezpieczenie ścieżki pliku
        if (!filename.startsWith("/")) filename = "/" + filename;
        
        // Utworzenie uchwytu systemowego jeśli to pierwsza paczka bajtów
        if(!index){
            Serial.printf("[FILE_SERVER] Odbieram plik: %s\n", filename.c_str());
            request->_tempFile = SD.open(filename, FILE_WRITE);
        }
        // Wgrywanie w paczkach bez blokowania procesora
        if(request->_tempFile){
            if(len) request->_tempFile.write(data, len);
            if(final){
                request->_tempFile.close();
                Serial.printf("[FILE_SERVER] Zapisano plik na SD: %s (Bajtów: %u)\n", filename.c_str(), index+len);
            }
        }
    });
    // Zwraca listę plików z karty SD w formacie JSON
    server.on("/api/files", HTTP_GET, [](AsyncWebServerRequest *request){
        String json = "[";
        File root = SD.open("/");
        File file = root.openNextFile();
        bool first = true;
        while(file){
            if(!file.isDirectory()){
                if(!first) json += ",";
                json += "{\"name\":\"" + String(file.name()) + "\",\"size\":" + String(file.size()) + "}";
                first = false;
            }
            file = root.openNextFile();
        }
        json += "]";
        request->send(200, "application/json", json);
    });

    // Endpoint do usuwania plików z karty SD z przeglądarki
    server.on("/api/delete", HTTP_POST, [](AsyncWebServerRequest *request){
        if(request->hasParam("path", true)){
            String p = request->getParam("path", true)->value();
            if(!p.startsWith("/")) p = "/" + p;
            if(SD.remove(p)) request->send(200, "text/plain", "OK");
            else request->send(500, "text/plain", "Blad usuwania");
        } else {
            request->send(400);
        }
    });

    // 6. Odczyt statycznych plików (np. muzyki) prosto z karty SD
    server.serveStatic("/fs/", SD, "/");
}

void NetworkManager::process() {
    if (isAPMode) dnsServer.processNextRequest();
    ws.cleanupClients(); // Czyszczenie odłączonych klientów (zapobiega wyciekom pamięci)
}

void NetworkManager::broadcastText(String msg) {
    ws.textAll(msg);
}