#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include "NvsManager.h"
#include "WebPages.h"

// ==============================================================================
// KLASA ZARZĄDZAJĄCA KOMUNIKACJĄ SIECIOWĄ (Zasada izolacji domen)
// ==============================================================================
class NetworkManager {
private:
    // Enkapsulacja obiektów - nikt spoza tej klasy nie ma do nich dostępu
    AsyncWebServer server;
    DNSServer dnsServer;
    NvsManager* nvsRef; // Wskaźnik do naszego menedżera pamięci
    
    bool isAPMode;
    const byte DNS_PORT = 53;

    // Metody prywatne rozruchowe
    void setupAP();
    void setupEndpoints();

public:
    // Konstruktor inicjalizuje serwer WWW na porcie 80
    NetworkManager();

    // Główna funkcja startowa wywoływana w main.cpp
    void startSystem(NvsManager* nvs);

    // Taktowanie serwera DNS w pętli loop() - wymagane dla Captive Portal
    void process();
};

#endif