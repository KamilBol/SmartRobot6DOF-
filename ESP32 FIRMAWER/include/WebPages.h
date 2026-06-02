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

// --- GŁÓWNY PANEL STEROWANIA I MOST AI (TRYB DOMOWY) ---
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pl">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>SmartRobot 6DOF - Centrum Dowodzenia</title>
    <style>
        body { background: #121212; color: #00ffcc; font-family: sans-serif; margin: 0; padding: 20px; }
        h2 { text-align: center; }
        .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 20px; margin-top: 20px; }
        .card { background: #1e1e1e; border: 1px solid #00ffcc; border-radius: 12px; padding: 20px; box-shadow: 0 4px 15px rgba(0,255,204,0.1); }
        input[type=range] { width: 100%; margin-top: 10px; accent-color: #00ffcc; }
        input[type=text], input[type=password] { width: calc(100% - 22px); padding: 10px; margin-top: 10px; border-radius: 5px; border: 1px solid #555; background: #333; color: #fff; }
        button { background: #00ffcc; color: #121212; border: none; padding: 10px 20px; border-radius: 5px; cursor: pointer; font-weight: bold; margin-top: 10px; width: 100%; transition: 0.3s; }
        button:hover { background: #00ccaa; }
        .btn-mic { background: #ff0055; color: white; font-size: 1.2em; padding: 15px; border-radius: 30px; }
        .btn-mic:hover { background: #cc0044; }
        .chat-box { background: #0a0a0a; border: 1px solid #333; height: 200px; overflow-y: auto; padding: 10px; border-radius: 5px; margin-top: 15px; font-family: monospace; color: #ccc; }
        .chat-user { color: #00ffcc; }
        .chat-ai { color: #ffaa00; }
        .chat-err { color: #ff3333; }
        .status { text-align: center; padding: 10px; border-radius: 8px; font-weight: bold; margin-bottom: 20px; background: #222; }
    </style>
</head>
<body>
    <h2>🤖 Panel Sterowania Wally</h2>
    <div id="ws-status" class="status" style="color: yellow;">Łączenie z systemem nerwowym (WebSockets)...</div>

    <div class="grid">
        <div class="card">
            <h3>🧠 Asystent AI (Gemini 1.5) & Supla</h3>
            <button id="btn-speak" class="btn-mic" onclick="startDictation()">🎤 Mów do Wally'ego</button>
            
            <div id="chat-log" class="chat-box"></div>
            
            <input type="password" id="api-gemini" placeholder="Gemini API Key (np. AQ.Ab8RN6...)">
            <input type="password" id="api-supla" placeholder="Supla Personal Access Token">
            <input type="text" id="supla-server" placeholder="Twój serwer Supla (np. svr12.supla.org)">
        </div>

        <div class="card">
            <h3>⚙️ Kinematyka (Live)</h3>
            <label>Lewa Stopa</label>
            <input type="range" min="180" max="430" value="307" oninput="sendServo(0, this.value)">
            <br><label>Prawa Stopa</label>
            <input type="range" min="180" max="430" value="307" oninput="sendServo(1, this.value)">
            <br><label>Lewa Noga</label>
            <input type="range" min="180" max="430" value="307" oninput="sendServo(2, this.value)">
            <br><label>Prawa Noga</label>
            <input type="range" min="180" max="430" value="307" oninput="sendServo(3, this.value)">
            <br><label>Lewa Ręka</label>
            <input type="range" min="180" max="430" value="307" oninput="sendServo(4, this.value)">
            <br><label>Prawa Ręka</label>
            <input type="range" min="180" max="430" value="307" oninput="sendServo(5, this.value)">
        </div>
    </div>

    <script>
        // --- 1. WEBSOCKETS ---
        var gateway = `ws://${window.location.hostname}/ws`;
        var websocket;
        
        function initWebSocket() {
            websocket = new WebSocket(gateway);
            websocket.onopen = () => { 
                document.getElementById('ws-status').innerText = '✅ POŁĄCZONO Z ROBOTEM'; 
                document.getElementById('ws-status').style.color = '#00ffcc'; 
            };
            websocket.onclose = () => { 
                document.getElementById('ws-status').innerText = '❌ UTRACONO POŁĄCZENIE'; 
                document.getElementById('ws-status').style.color = 'red'; 
                setTimeout(initWebSocket, 2000); 
            };
        }

        function sendServo(id, val) {
            if (websocket && websocket.readyState === WebSocket.OPEN) {
                websocket.send("S" + id + ":" + val);
            }
        }

        // --- 2. LOGIKA CZATU ---
        function logChat(who, text, type) {
            const box = document.getElementById('chat-log');
            let colorClass = type === 'err' ? 'chat-err' : (who === 'Ty' ? 'chat-user' : 'chat-ai');
            box.innerHTML += `<strong class="${colorClass}">${who}:</strong> ${text}<br><br>`;
            box.scrollTop = box.scrollHeight;
        }

        // --- 3. SPEECH TO TEXT (Rozpoznawanie mowy) ---
        function startDictation() {
            if (window.hasOwnProperty('webkitSpeechRecognition')) {
                var recognition = new webkitSpeechRecognition();
                recognition.continuous = false;
                recognition.interimResults = false;
                recognition.lang = "pl-PL";

                document.getElementById('btn-speak').innerText = "🔴 Nasłuchiwanie...";
                recognition.start();

                recognition.onresult = function(e) {
                    document.getElementById('btn-speak').innerText = "🎤 Mów do Wally'ego";
                    let userText = e.results[0][0].transcript;
                    logChat("Ty", userText, 'user');
                    askGemini(userText);
                };

                recognition.onerror = function(e) {
                    document.getElementById('btn-speak').innerText = "🎤 Mów do Wally'ego";
                    logChat("System", "Błąd mikrofonu.", 'err');
                }
            } else {
                logChat("System", "Twoja przeglądarka nie obsługuje Web Speech API.", 'err');
            }
        }

        // --- 4. ZAPYTANIE DO GEMINI API (POPRAWIONY ENDPOINT I FORMAT) ---
        async function askGemini(promptText) {
            const apiKey = document.getElementById('api-gemini').value.trim();
            if(!apiKey) {
                logChat("Błąd", "Wprowadź klucz API Gemini!", 'err');
                return;
            }

            logChat("Wally", "Myślę...", 'ai');

            // PANCERNY ENDPOINT DLA GEMINI 2.5 FLASH:
            const url = `https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-flash:generateContent?key=${apiKey}`;

            // Instrukcja systemowa instruująca AI jak ma się zachować (w tym obsługa Supli)
            const payload = {
                systemInstruction: {
                    parts: [{ text: "Jesteś sympatycznym robotem o imieniu Wally. Stworzonym by bawić i pomagać Gabrysi. Jesteś krótki, zwięzły i zabawny. ZAWSZE rozpoczynaj odpowiedź od tagu ruchu, np. [RUCH:RADOSC] lub [RUCH:ZASKOCZENIE]. Jeśli prośba dotyczy włączenia/wyłączenia czegoś w domu, dodaj na końcu [SUPLA:ID:AKCJA] (gdzie AKCJA to TURN_ON lub TURN_OFF)." }]
                },
                contents: [{ role: "user", parts: [{ text: promptText }] }]
            };

            try {
                const response = await fetch(url, {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify(payload)
                });

                if (!response.ok) {
                    const errObj = await response.json();
                    logChat("Błąd API", `Odrzut Serwera (HTTP ${response.status}): ${errObj.error.message}`, 'err');
                    return;
                }

                const data = await response.json();
                const aiResponse = data.candidates[0].content.parts[0].text;
                processAIResponse(aiResponse);

            } catch (error) {
                logChat("Błąd Sieci", error.message, 'err');
            }
        }

        // --- 5. PARSOWANIE ODPOWIEDZI, RUCH, TTS I SUPLA ---
        function processAIResponse(text) {
            // Szukanie tagów ruchu np. [RUCH:RADOSC]
            let cleanText = text;
            const ruchMatch = text.match(/\[RUCH:(.*?)\]/);
            if(ruchMatch) {
                cleanText = cleanText.replace(ruchMatch[0], '').trim();
                let emocja = ruchMatch[1];
                // Wykonanie prostego ruchu serwami po rozpoznaniu tagu
                if(emocja === "RADOSC") {
                    sendServo(4, 200); sendServo(5, 410); // Ręce w górę
                    setTimeout(() => { sendServo(4, 307); sendServo(5, 307); }, 2000);
                } else {
                    sendServo(4, 250); sendServo(5, 350); 
                    setTimeout(() => { sendServo(4, 307); sendServo(5, 307); }, 2000);
                }
            }

            // Szukanie tagów Supla np. [SUPLA:142:TURN_ON]
            const suplaMatch = text.match(/\[SUPLA:(\d+):(TURN_ON|TURN_OFF|TOGGLE)\]/);
            if(suplaMatch) {
                cleanText = cleanText.replace(suplaMatch[0], '').trim();
                let channelId = suplaMatch[1];
                let action = suplaMatch[2];
                executeSuplaAction(channelId, action);
            }

            logChat("Wally", cleanText, 'ai');
            speakText(cleanText);
        }

        // --- 6. TEXT TO SPEECH (Synteza mowy w przeglądarce) ---
        function speakText(text) {
            if ('speechSynthesis' in window) {
                var utterance = new SpeechSynthesisUtterance(text);
                utterance.lang = 'pl-PL';
                utterance.rate = 1.1; // Lekko przyspieszony, robotyczny rytm
                window.speechSynthesis.speak(utterance);
            }
        }

        // --- 7. SUPLA API EXECUTION ---
        async function executeSuplaAction(channelId, action) {
            const token = document.getElementById('api-supla').value.trim();
            const server = document.getElementById('supla-server').value.trim();
            
            if(!token || !server) {
                logChat("System", "Brak danych serwera Supla lub Tokenu do wykonania akcji!", 'err');
                return;
            }

            const url = `https://${server}/api/v2.3.0/channels/${channelId}`;
            try {
                const response = await fetch(url, {
                    method: 'PATCH',
                    headers: {
                        'Authorization': `Bearer ${token}`,
                        'Content-Type': 'application/json',
                        'Accept': 'application/json'
                    },
                    body: JSON.stringify({ action: action })
                });

                if(response.ok) {
                    logChat("Supla", `Akcja ${action} na kanale ${channelId} wykonana.`, 'user');
                } else {
                    logChat("Supla Błąd", `Nieudana próba: HTTP ${response.status}`, 'err');
                }
            } catch (error) {
                logChat("Supla Sieć", error.message, 'err');
            }
        }

        window.addEventListener('load', initWebSocket);
    </script>
</body>
</html>
)rawliteral";

#endif