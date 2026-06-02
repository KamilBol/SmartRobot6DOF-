#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include "NvsManager.h"
#include "ServoManager.h"
#include "WebPages.h"

class NetworkManager {
private:
    AsyncWebServer server;
    AsyncWebSocket ws; // NOWOŚĆ: Serwer czasu rzeczywistego (Etap 3)
    DNSServer dnsServer;
    
    NvsManager* nvsRef; 
    ServoManager* servoRef; // NOWOŚĆ: Wskaźnik, żeby sieć mogła ruszać ramionami
    
    bool isAPMode;
    const byte DNS_PORT = 53;

    void setupAP();
    void setupSTA(String ssid, String pass);
    void setupEndpoints();

public:
    NetworkManager();
    void startSystem(NvsManager* nvs, ServoManager* servo);
    void process(); 
    void broadcastText(String msg); // Wysyłanie logów na ekran komputera
};

#endif