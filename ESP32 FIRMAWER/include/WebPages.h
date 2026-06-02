#ifndef WEB_PAGES_H
#define WEB_PAGES_H

#include <Arduino.h>

const char SETUP_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html lang="pl"><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1.0"><title>SmartRobot Setup</title><style>body { background: #121212; color: #00ffcc; font-family: sans-serif; text-align: center; padding: 20px; }.card { background: #1e1e1e; border: 1px solid #00ffcc; border-radius: 12px; padding: 20px; max-width: 400px; margin: 0 auto; box-shadow: 0 4px 15px rgba(0,255,204,0.2); }input[type="text"], input[type="password"] { width: 90%; padding: 10px; margin: 10px 0; border-radius: 5px; border: 1px solid #555; background: #333; color: #fff; }input[type="submit"] { background: #00ffcc; color: #121212; border: none; padding: 12px 20px; border-radius: 5px; cursor: pointer; font-weight: bold; width: 100%; transition: 0.3s; }input[type="submit"]:hover { background: #00ccaa; }</style></head><body><div class="card"><h2>🤖 Konfiguracja Sieci</h2><form action="/connect" method="POST"><input type="text" name="ssid" placeholder="Nazwa sieci Wi-Fi (SSID)" required><br><input type="password" name="pass" placeholder="Hasło Wi-Fi" required><br><input type="submit" value="POŁĄCZ Z BAZĄ"></form></div></body></html>
)rawliteral";

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pl">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Wally 6DOF - AI Core</title>
    <style>
        body { background: #121212; color: #00ffcc; font-family: sans-serif; text-align: center; margin: 0; padding: 20px; }
        .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 20px; margin-top: 20px; }
        .card { background: #1e1e1e; border: 1px solid #00ffcc; border-radius: 12px; padding: 20px; box-shadow: 0 4px 15px rgba(0,255,204,0.1); }
        input[type=range] { width: 100%; margin-top: 10px; accent-color: #00ffcc; }
        input[type=text], input[type=password] { width: 90%; padding: 8px; margin: 5px 0; border-radius: 5px; border: 1px solid #555; background: #333; color: #fff; }
        button { background: #00ffcc; color: #121212; border: none; padding: 12px 20px; border-radius: 5px; cursor: pointer; font-weight: bold; font-size: 1.1em; transition: 0.3s; margin-top: 10px; }
        button:hover { background: #00ccaa; }
        .mic-btn { background: #ff0055; color: white; border-radius: 50px; padding: 15px 30px; font-size: 1.2em; animation: pulse 2s infinite; }
        @keyframes pulse { 0% { box-shadow: 0 0 0 0 rgba(255,0,85,0.7); } 70% { box-shadow: 0 0 0 15px rgba(255,0,85,0); } 100% { box-shadow: 0 0 0 0 rgba(255,0,85,0); } }
        .status { padding: 10px; border-radius: 8px; font-weight: bold; margin-bottom: 20px; background: #222; }
        .chat-box { background: #000; border: 1px solid #333; padding: 10px; height: 150px; overflow-y: auto; text-align: left; font-size: 0.9em; color: #ccc; margin-top: 10px; }
    </style>
</head>
<body>
    <h2>🤖 Wally 6DOF - Centrum AI & Supla</h2>
    <div id="ws-status" class="status" style="color: yellow;">Łączenie z systemem nerwowym...</div>

    <div class="grid">
        <div class="card">
            <h3>🧠 Asystent Głosowy (STT -> LLM -> TTS)</h3>
            <button id="micBtn" class="mic-btn" onclick="toggleDictation()">🎙️ Mów do Wally'ego</button>
            <div id="chat" class="chat-box">Rozpocznij rozmowę...</div>
            <hr style="border: 0; border-top: 1px solid #333; margin: 15px 0;">
            <input type="text" id="aiUrl" placeholder="URL API (np. OpenAI / Ollama)" value="https://api.openai.com/v1/chat/completions">
            <input type="password" id="aiKey" placeholder="Klucz API (OpenAI Bearer)">
            <input type="password" id="suplaToken" placeholder="Supla Personal Access Token">
            <input type="text" id="suplaServer" placeholder="Serwer Supli (np. cloud.supla.org)" value="cloud.supla.org">
        </div>

        <div class="card">
            <h3>⚙️ Kalibracja Kinematyki</h3>
            <label>L. Stopa</label> <input type="range" min="180" max="430" value="307" oninput="sendS(0, this.value)">
            <label>P. Stopa</label> <input type="range" min="180" max="430" value="307" oninput="sendS(1, this.value)">
            <label>L. Noga</label>  <input type="range" min="180" max="430" value="307" oninput="sendS(2, this.value)">
            <label>P. Noga</label>  <input type="range" min="180" max="430" value="307" oninput="sendS(3, this.value)">
            <label>L. Ręka</label>  <input type="range" min="180" max="430" value="307" oninput="sendS(4, this.value)">
            <label>P. Ręka</label>  <input type="range" min="180" max="430" value="307" oninput="sendS(5, this.value)">
        </div>
    </div>

    <script>
        // --- WEBSOCKETS (Ruch i Telemetria) ---
        let ws;
        function initWS() {
            ws = new WebSocket(`ws://${window.location.hostname}/ws`);
            ws.onopen = () => document.getElementById('ws-status').innerHTML = '✅ SYSTEM ONLINE';
            ws.onclose = () => setTimeout(initWS, 2000);
        }
        function sendS(id, val) { if (ws.readyState === 1) ws.send(`S${id}:${val}`); }
        
        // --- SPEECH-TO-TEXT (STT) ---
        let recognizing = false;
        const SpeechRecognition = window.SpeechRecognition || window.webkitSpeechRecognition;
        const recognition = new SpeechRecognition();
        recognition.lang = 'pl-PL';
        recognition.continuous = false;
        
        function toggleDictation() {
            if (recognizing) { recognition.stop(); return; }
            recognition.start();
        }
        
        recognition.onstart = () => { recognizing = true; document.getElementById('micBtn').innerText = '🔴 Nasłuchuję...'; };
        recognition.onend = () => { recognizing = false; document.getElementById('micBtn').innerText = '🎙️ Mów do Wally\'ego'; };
        
        recognition.onresult = (event) => {
            const transcript = event.results[0][0].transcript;
            logChat('👤 Ty', transcript, '#00ffcc');
            processAI(transcript);
        };

        // --- AI PROCESSING (LLM FETCH) ---
        async function processAI(text) {
            const aiUrl = document.getElementById('aiUrl').value;
            const aiKey = document.getElementById('aiKey').value;
            logChat('🤖 Wally', 'Myślę...', '#aaa');
            
            // Animacja myślenia (Ruch Ręką)
            sendS(4, 250); sendS(5, 360);

            try {
                // Konfiguracja pod standard OpenAI API (działa też z vLLM/Ollama API)
                const response = await fetch(aiUrl, {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json', 'Authorization': `Bearer ${aiKey}` },
                    body: JSON.stringify({
                        model: "gpt-4o-mini", // Lub dowolny lokalny
                        messages: [
                            { role: "system", content: "Jesteś robotem Wally. Bądź krótki. Jeśli użytkownik chce zapalić/zgasić światło w salonie (ID 142), dodaj na końcu tag [SUPLA:142:TURN_ON] lub [SUPLA:142:TURN_OFF]. Jeśli się witasz, dodaj tag [RUCH:RADOSC]." },
                            { role: "user", content: text }
                        ]
                    })
                });
                
                const data = await response.json();
                const reply = data.choices[0].message.content;
                parseAndExecute(reply);
            } catch (err) {
                logChat('❌ Błąd', 'Brak połączenia z API AI.', 'red');
            }
        }

        // --- PARSER TAGÓW (SUPLA & RUCH) ---
        function parseAndExecute(replyText) {
            let cleanText = replyText;

            // Ekstrakcja Supli [SUPLA:ID:AKCJA]
            const suplaMatch = replyText.match(/\[SUPLA:(\d+):([A-Z_]+)\]/);
            if (suplaMatch) {
                callSupla(suplaMatch[1], suplaMatch[2]);
                cleanText = cleanText.replace(suplaMatch[0], ''); // Usuń tag z tekstu mówionego
            }

            // Ekstrakcja Ruchu [RUCH:EMOCJA]
            const moveMatch = replyText.match(/\[RUCH:([A-Z_]+)\]/);
            if (moveMatch) {
                if (moveMatch[1] === 'RADOSC') { sendS(4, 400); sendS(5, 200); }
                cleanText = cleanText.replace(moveMatch[0], '');
            }

            logChat('🤖 Wally', cleanText, '#fff');
            speakText(cleanText);
        }

        // --- SUPLA REST API FETCH ---
        async function callSupla(channelId, action) {
            const token = document.getElementById('suplaToken').value;
            const server = document.getElementById('suplaServer').value;
            if(!token) return logChat('⚠️ Supla', 'Brak tokena!', 'orange');
            
            try {
                await fetch(`https://${server}/api/v2.3.0/channels/${channelId}`, {
                    method: 'PATCH',
                    headers: { 'Content-Type': 'application/json', 'Authorization': `Bearer ${token}` },
                    body: JSON.stringify({ action: action })
                });
                logChat('🏠 Supla', `Wykonano akcję: ${action} na kanale ${channelId}`, '#0f0');
            } catch(e) { logChat('❌ Supla', 'Błąd połączenia', 'red'); }
        }

        // --- TEXT-TO-SPEECH (TTS) ---
        function speakText(text) {
            const utterance = new SpeechSynthesisUtterance(text);
            utterance.lang = 'pl-PL';
            utterance.onend = () => { sendS(4, 307); sendS(5, 307); }; // Powrót rąk po zakończeniu mówienia
            window.speechSynthesis.speak(utterance);
        }

        function logChat(sender, msg, color) {
            const chat = document.getElementById('chat');
            chat.innerHTML += `<div style="color:${color}; margin-bottom:5px;"><b>${sender}:</b> ${msg}</div>`;
            chat.scrollTop = chat.scrollHeight;
        }

        window.onload = initWS;
    </script>
</body>
</html>
)rawliteral";

#endif