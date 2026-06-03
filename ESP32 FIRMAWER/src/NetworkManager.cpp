#include "NetworkManager.h"
#include <SD.h>

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

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n[NET_STA] Polaczono! IP: " + WiFi.localIP().toString());
        if (MDNS.begin("wally")) Serial.println("[NET_STA] mDNS dziala: http://wally.local");
        setupEndpoints();
        server.begin();
    } else {
        Serial.println("\n[NET_STA] Blad polaczenia. Startuje AP...");
        setupAP();
    }
}

void NetworkManager::setupAP() {
    isAPMode = true;
    WiFi.mode(WIFI_AP);
    WiFi.softAP("Wally-Setup", "12345678");
    Serial.println("[NET_AP] Tryb AP aktywny: Wally-Setup / 12345678");
    Serial.println("[NET_AP] IP: " + WiFi.softAPIP().toString());
    dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
    setupEndpoints();
    server.begin();
}

void NetworkManager::setupEndpoints() {
    // --- WEBSOCKETS (RUCH, KALIBRACJA I PRĘDKOŚĆ) ---
    ws.onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len){
        if(type == WS_EVT_DATA){
            AwsFrameInfo *info = (AwsFrameInfo*)arg;
            if(info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT){
                data[len] = 0;
                String msg = (char*)data;
                
                // 1. Odbiór pojedynczego ruchu np. "S4:350"
                if(msg.startsWith("S") && msg.indexOf(":") > 0){
                    uint8_t id = msg.substring(1, msg.indexOf(":")).toInt();
                    int16_t val = msg.substring(msg.indexOf(":")+1).toInt();
                    if(servoRef) servoRef->setServoTicksDirect(id, val);
                }
                // 2. Odbiór zmiany prędkości np. "SPD:15"
                else if (msg.startsWith("SPD:")) {
                    uint8_t spd = msg.substring(4).toInt();
                    if(servoRef) servoRef->setSpeed(spd);
                    Serial.printf("[SERVO] Zmiana predkosci na: %d ms\n", spd);
                }
                // 3. Zapis limitów z GUI np. "CFG:4:150:450:307"
                else if (msg.startsWith("CFG:")) {
                    int p1 = msg.indexOf(':');
                    int p2 = msg.indexOf(':', p1 + 1);
                    int p3 = msg.indexOf(':', p2 + 1);
                    int p4 = msg.indexOf(':', p3 + 1);
                    
                    if(p1>0 && p2>0 && p3>0 && p4>0) {
                        uint8_t id = msg.substring(p1 + 1, p2).toInt();
                        ServoBounds bounds;
                        bounds.minTicks = msg.substring(p2 + 1, p3).toInt();
                        bounds.maxTicks = msg.substring(p3 + 1, p4).toInt();
                        bounds.homeTicks = msg.substring(p4 + 1).toInt();
                        
                        if(nvsRef) nvsRef->saveServoLimits(id, bounds);
                        if(servoRef) servoRef->setSoftwareLimits(id, bounds);
                        Serial.printf("[NVS] Zapisano kalibracje dla serwa %d\n", id);
                    }
                }
            }
        }
    });
    server.addHandler(&ws);

    // --- HTTP ENDPOINTS ---
    server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request){
        if(isAPMode) request->send_P(200, "text/html", SETUP_HTML);
        else request->send_P(200, "text/html", INDEX_HTML);
    });

    server.onNotFound([](AsyncWebServerRequest *request){
        request->send(404, "text/plain", "Nie znaleziono");
    });

    server.on("/connect", HTTP_POST, [this](AsyncWebServerRequest *request){
        if(request->hasParam("ssid", true) && request->hasParam("pass", true)){
            String ssid = request->getParam("ssid", true)->value();
            String pass = request->getParam("pass", true)->value();
            if(nvsRef) nvsRef->saveWiFi(ssid, pass);
            request->send(200, "text/html", "Zapisano Wi-Fi! Robot sie zrestartuje...");
            delay(1000);
            ESP.restart();
        } else {
            request->send(400, "text/html", "Brak danych");
        }
    });

    server.on("/api/upload", HTTP_POST, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "OK");
    }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
        if(!filename.startsWith("/")) filename = "/" + filename;
        if(!index){
            request->_tempFile = SD.open(filename, FILE_WRITE);
        }
        if(request->_tempFile){
            if(len) request->_tempFile.write(data, len);
            if(final) request->_tempFile.close();
        }
    });

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

    server.on("/api/delete", HTTP_POST, [](AsyncWebServerRequest *request){
        if(request->hasParam("path", true)){
            String p = request->getParam("path", true)->value();
            if(!p.startsWith("/")) p = "/" + p;
            if(SD.remove(p)) request->send(200, "text/plain", "OK");
            else request->send(500, "text/plain", "Error");
        } else {
            request->send(400);
        }
    });

    // Serwowanie plików z karty SD pod adresem IP_ROBOTA/fs/
    server.serveStatic("/fs/", SD, "/");
}

void NetworkManager::process() {
    if(isAPMode) dnsServer.processNextRequest();
    ws.cleanupClients();
}

void NetworkManager::broadcastText(String msg) {
    ws.textAll(msg);
}