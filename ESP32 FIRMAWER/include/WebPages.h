#ifndef WEB_PAGES_H
#define WEB_PAGES_H

#include <Arduino.h>

// --- STRONA KONFIGURACYJNA (TRYB AP) ---
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
        <form action="/connect" method="POST">
            <input type="text" name="ssid" placeholder="Nazwa sieci Wi-Fi (SSID)" required><br>
            <input type="password" name="pass" placeholder="Hasło Wi-Fi" required><br>
            <input type="submit" value="POŁĄCZ Z BAZĄ">
        </form>
    </div>
</body>
</html>
)rawliteral";

// --- GŁÓWNY PANEL STEROWANIA WEBSOCKETS (TRYB DOMOWY) ---
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pl">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>SmartRobot 6DOF - Centrum Dowodzenia</title>
    <style>
        body { background: #121212; color: #00ffcc; font-family: sans-serif; text-align: center; margin: 0; padding: 20px; }
        .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(250px, 1fr)); gap: 20px; margin-top: 20px; }
        .card { background: #1e1e1e; border: 1px solid #00ffcc; border-radius: 12px; padding: 20px; box-shadow: 0 4px 15px rgba(0,255,204,0.1); }
        input[type=range] { width: 100%; margin-top: 10px; accent-color: #00ffcc; }
        input[type=file] { color: #fff; margin-bottom: 15px; }
        button { background: #00ffcc; color: #121212; border: none; padding: 10px 20px; border-radius: 5px; cursor: pointer; font-weight: bold; }
        .status { padding: 10px; border-radius: 8px; font-weight: bold; margin-bottom: 20px; background: #222; }
    </style>
</head>
<body>
    <h2>🤖 Panel Sterowania Wally</h2>
    <div id="ws-status" class="status" style="color: yellow;">Łączenie z systemem nerwowym (WebSockets)...</div>

    <div class="grid">
        <div class="card">
            <h3>⚙️ Kinematyka (Live)</h3>
            <label>Lewa Stopa</label>
            <input type="range" min="180" max="430" value="307" oninput="sendServo(0, this.value)">
            <br><br><label>Prawa Stopa</label>
            <input type="range" min="180" max="430" value="307" oninput="sendServo(1, this.value)">
            <br><br><label>Lewa Noga</label>
            <input type="range" min="180" max="430" value="307" oninput="sendServo(2, this.value)">
            <br><br><label>Prawa Noga</label>
            <input type="range" min="180" max="430" value="307" oninput="sendServo(3, this.value)">
            <br><br><label>Lewa Ręka</label>
            <input type="range" min="180" max="430" value="307" oninput="sendServo(4, this.value)">
            <br><br><label>Prawa Ręka</label>
            <input type="range" min="180" max="430" value="307" oninput="sendServo(5, this.value)">
        </div>

        <div class="card">
            <h3>🎵 Asynchroniczny Serwer Plików (SD)</h3>
            <p style="font-size: 0.9em; color: #ccc;">Wgraj plik MP3 lub WAV bezpośrednio na kartę pamięci robota po sieci Wi-Fi.</p>
            <input type="file" id="fileInput" accept=".mp3,.wav"><br>
            <button onclick="uploadFile()">Wgraj Plik</button>
            <p id="uploadStatus" style="margin-top: 15px; color: #fff;"></p>
        </div>
    </div>

    <script>
        var gateway = `ws://${window.location.hostname}/ws`;
        var websocket;
        
        function initWebSocket() {
            websocket = new WebSocket(gateway);
            websocket.onopen = () => { document.getElementById('ws-status').innerText = '✅ POŁĄCZONO Z ROBOTEM (0 ms)'; document.getElementById('ws-status').style.color = '#00ffcc'; };
            websocket.onclose = () => { document.getElementById('ws-status').innerText = '❌ UTRACONO ZASILANIE / POŁĄCZENIE'; document.getElementById('ws-status').style.color = 'red'; setTimeout(initWebSocket, 2000); };
            websocket.onmessage = (e) => { console.log(e.data); };
        }

        function sendServo(id, val) {
            if (websocket && websocket.readyState === WebSocket.OPEN) {
                websocket.send("S" + id + ":" + val); // Wysyłanie superlekkiej ramki "S0:307"
            }
        }

        function uploadFile() {
            var fileInput = document.getElementById('fileInput').files;
            if(fileInput.length === 0) return alert("Wybierz plik MP3 lub WAV!");
            
            var file = fileInput[0];
            var formData = new FormData();
            formData.append("file", file, "/" + file.name); 
            
            document.getElementById('uploadStatus').innerText = "Wgrywanie (nie wyłączaj robota)... ⏳";
            
            fetch('/api/upload', { method: 'POST', body: formData })
            .then(response => { document.getElementById('uploadStatus').innerText = "✅ Plik zapisany na karcie SD!"; document.getElementById('fileInput').value = ""; })
            .catch(error => { document.getElementById('uploadStatus').innerText = "❌ Wystąpił błąd transferu."; });
        }

        window.addEventListener('load', initWebSocket);
    </script>
</body>
</html>
)rawliteral";

#endif