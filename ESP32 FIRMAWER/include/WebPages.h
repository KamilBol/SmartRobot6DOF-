#ifndef WEB_PAGES_H
#define WEB_PAGES_H

#include <Arduino.h>

// ==============================================================================
// 🌐 KOD HTML/CSS STRONY KONFIGURACYJNEJ (CAPTIVE PORTAL) W PAMIĘCI PROGMEM
// ==============================================================================
// Interfejs w Dark Mode, responsywny (RWD), bez zewnętrznych skryptów.
const char SETUP_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pl">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>SmartRobot Setup</title>
    <style>
        body { background: #121212; color: #00ffcc; font-family: sans-serif; text-align: center; padding: 20px; }
        .card { background: #1e1e1e; border: 1px solid #00ffcc; border-radius: 12px; padding: 20px; max-width: 400px; margin: 0 auto; box-shadow: 0 4px 15px rgba(0,255,204,0.2); }
        input[type="text"], input[type="password"] { width: 90%; padding: 10px; margin: 10px 0; border-radius: 5px; border: 1px solid #555; background: #333; color: #fff; }
        input[type="submit"] { background: #00ffcc; color: #121212; border: none; padding: 12px 20px; border-radius: 5px; cursor: pointer; font-weight: bold; width: 100%; transition: 0.3s; }
        input[type="submit"]:hover { background: #00ccaa; }
    </style>
</head>
<body>
    <div class="card">
        <h2>🤖 Konfiguracja Sieci</h2>
        <p>Wprowadź dane swojego domowego Wi-Fi</p>
        <form action="/connect" method="POST">
            <input type="text" name="ssid" placeholder="Nazwa sieci Wi-Fi (SSID)" required><br>
            <input type="password" name="pass" placeholder="Hasło Wi-Fi" required><br>
            <input type="submit" value="POŁĄCZ Z BAZĄ">
        </form>
    </div>
</body>
</html>
)rawliteral";

#endif