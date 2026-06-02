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
    <title>Wally Setup</title>
    <style>
        body { background: #121212; color: #00ffcc; font-family: 'Segoe UI', sans-serif; text-align: center; padding: 20px; }
        .card { background: #1e1e1e; border: 1px solid #00ffcc; border-radius: 16px; padding: 30px; max-width: 400px; margin: 0 auto; box-shadow: 0 0 20px rgba(0,255,204,0.2); }
        input { width: 90%; padding: 12px; margin: 10px 0; border-radius: 8px; border: 1px solid #444; background: #2a2a2a; color: #fff; font-size: 16px; outline: none; transition: 0.3s; }
        input:focus { border-color: #00ffcc; box-shadow: 0 0 8px rgba(0,255,204,0.5); }
        input[type="submit"] { background: #00ffcc; color: #121212; border: none; font-weight: bold; cursor: pointer; margin-top: 20px; }
        input[type="submit"]:hover { background: #00ccaa; transform: translateY(-2px); }
    </style>
</head>
<body>
    <div class="card">
        <h2>🤖 WAKE UP, WALLY</h2>
        <p style="color: #888;">Konfiguracja sieciowa systemu 6DOF</p>
        <form action="/connect" method="POST">
            <input type="text" name="ssid" placeholder="Nazwa domowego Wi-Fi (SSID)" required><br>
            <input type="password" name="pass" placeholder="Hasło Wi-Fi" required><br>
            <input type="submit" value="INICJALIZUJ POŁĄCZENIE">
        </form>
    </div>
</body>
</html>
)rawliteral";

// --- GŁÓWNY PANEL STEROWANIA (SPA HTML5 + CSS3 + JS) ---
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pl">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Wally 6DOF Dashboard</title>
    <style>
        :root { --main-color: #00ffcc; --bg-dark: #121212; --bg-card: #1e1e1e; --text: #eee; }
        body { background: var(--bg-dark); color: var(--text); font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; margin: 0; padding: 20px; }
        
        /* HEADER & STATUS */
        .header { text-align: center; margin-bottom: 20px; }
        .header h1 { margin: 0; color: var(--main-color); text-transform: uppercase; letter-spacing: 2px; }
        .status-badge { display: inline-block; padding: 5px 15px; border-radius: 20px; background: #333; font-size: 14px; font-weight: bold; margin-top: 10px; border: 1px solid #555; }
        .status-badge.ok { border-color: var(--main-color); color: var(--main-color); box-shadow: 0 0 10px rgba(0,255,204,0.3); }
        .status-badge.err { border-color: #ff3366; color: #ff3366; box-shadow: 0 0 10px rgba(255,51,102,0.3); }

        /* TABS (SPA NAV) */
        .nav { display: flex; justify-content: center; gap: 10px; margin-bottom: 30px; }
        .tab-btn { background: #2a2a2a; color: #888; border: 1px solid #444; padding: 12px 24px; border-radius: 8px; cursor: pointer; font-size: 16px; font-weight: bold; transition: 0.3s; }
        .tab-btn:hover { color: #fff; background: #333; }
        .tab-btn.active { background: var(--bg-card); color: var(--main-color); border-color: var(--main-color); box-shadow: 0 0 15px rgba(0,255,204,0.2); }
        .tab-content { display: none; animation: fadeIn 0.3s ease-in-out; }
        .tab-content.active { display: block; }

        @keyframes fadeIn { from { opacity: 0; transform: translateY(10px); } to { opacity: 1; transform: translateY(0); } }

        /* CARDS & GRIDS */
        .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(320px, 1fr)); gap: 20px; }
        .card { background: var(--bg-card); border: 1px solid #333; border-radius: 16px; padding: 20px; position: relative; overflow: hidden; }
        .card::before { content: ""; position: absolute; top: 0; left: 0; width: 100%; height: 4px; background: var(--main-color); opacity: 0.8; }
        .card h3 { margin-top: 0; color: #fff; border-bottom: 1px solid #333; padding-bottom: 10px; }

        /* FORMS & INPUTS */
        input[type="text"], input[type="password"] { width: 100%; padding: 12px; margin: 10px 0; border-radius: 8px; border: 1px solid #444; background: #2a2a2a; color: #fff; box-sizing: border-box; outline: none; }
        input:focus { border-color: var(--main-color); }
        
        /* CHAT & MIC */
        .btn-mic { background: linear-gradient(135deg, #ff0055, #cc0044); color: white; border: none; padding: 15px; border-radius: 30px; cursor: pointer; font-size: 18px; font-weight: bold; width: 100%; box-shadow: 0 4px 15px rgba(255,0,85,0.4); transition: 0.3s; display: flex; align-items: center; justify-content: center; gap: 10px; }
        .btn-mic:hover { transform: scale(1.02); box-shadow: 0 6px 20px rgba(255,0,85,0.6); }
        .btn-mic.listening { animation: pulse 1.5s infinite; background: linear-gradient(135deg, #00ffcc, #00aa88); box-shadow: 0 4px 15px rgba(0,255,204,0.4); }
        @keyframes pulse { 0% { transform: scale(1); } 50% { transform: scale(1.05); } 100% { transform: scale(1); } }
        
        .chat-box { background: #0a0a0a; border: 1px solid #222; border-radius: 8px; height: 300px; overflow-y: auto; padding: 15px; margin-top: 15px; font-family: 'Consolas', monospace; font-size: 14px; line-height: 1.5; }
        .msg { margin-bottom: 15px; }
        .msg.user { color: #aaa; }
        .msg.user strong { color: var(--main-color); }
        .msg.ai { color: #ddd; }
        .msg.ai strong { color: #ffaa00; }
        .msg.err { color: #ff3366; }

        /* KINEMATYKA & SVG */
        .kinematics-container { display: grid; grid-template-columns: 1fr 1fr; gap: 20px; align-items: center; }
        @media (max-width: 768px) { .kinematics-container { grid-template-columns: 1fr; } }
        
        .sliders-group { display: flex; flex-direction: column; gap: 15px; }
        .slider-wrapper { background: #222; padding: 10px 15px; border-radius: 8px; border-left: 3px solid #555; }
        .slider-wrapper.left { border-color: #00bfff; }
        .slider-wrapper.right { border-color: #ffaa00; }
        .slider-header { display: flex; justify-content: space-between; font-size: 14px; font-weight: bold; margin-bottom: 8px; color: #ccc; }
        .val-badge { background: #000; padding: 2px 8px; border-radius: 4px; color: var(--main-color); font-family: monospace; }
        
        input[type="range"] { width: 100%; -webkit-appearance: none; background: transparent; }
        input[type="range"]::-webkit-slider-runnable-track { width: 100%; height: 6px; background: #444; border-radius: 3px; }
        input[type="range"]::-webkit-slider-thumb { -webkit-appearance: none; height: 18px; width: 18px; border-radius: 50%; background: var(--main-color); margin-top: -6px; cursor: pointer; box-shadow: 0 0 10px rgba(0,255,204,0.5); }
        input[type="range"]:active::-webkit-slider-thumb { transform: scale(1.2); }

        /* SVG ROBOT */
        .svg-container { background: #0a0a0a; border-radius: 16px; border: 1px solid #333; padding: 20px; display: flex; justify-content: center; }
        svg { width: 100%; max-width: 300px; height: auto; overflow: visible; }
        .bot-part { transition: transform 0.15s cubic-bezier(0.4, 0, 0.2, 1); }
    </style>
</head>
<body>

    <div class="header">
        <h1>WALLY 6DOF</h1>
        <div id="ws-status" class="status-badge err">Rozłączony z Rdzeniem</div>
    </div>

    <div class="nav">
        <button class="tab-btn active" onclick="switchTab('ai')">🧠 Asystent AI</button>
        <button class="tab-btn" onclick="switchTab('kinematics')">🦾 Studio Ruchu</button>
    </div>

    <div id="tab-ai" class="tab-content active">
        <div class="grid">
            <div class="card">
                <h3>🗣️ Komunikator Głosowy</h3>
                <button id="btn-speak" class="btn-mic" onclick="startDictation()">
                    <span id="mic-icon">🎤</span> <span id="mic-text">Mów do Wally'ego</span>
                </button>
                <div id="chat-log" class="chat-box"></div>
            </div>
            
            <div class="card">
                <h3>⚙️ Konfiguracja Kluczy API</h3>
                <label style="color:#aaa; font-size:12px;">Gemini 3.5 Flash API Key</label>
                <input type="password" id="api-gemini" placeholder="AQ.Ab8RN6...">
                
                <label style="color:#aaa; font-size:12px; margin-top:10px; display:block;">Supla Personal Access Token</label>
                <input type="password" id="api-supla" placeholder="Wklej token z Cloud Supla">
                
                <label style="color:#aaa; font-size:12px; margin-top:10px; display:block;">Supla Server (np. svr12.supla.org)</label>
                <input type="text" id="supla-server" placeholder="Twój serwer Supla">
                
                <button onclick="saveApiKeys()" style="width:100%; padding:12px; background:#333; color:#00ffcc; border:1px solid #00ffcc; border-radius:8px; margin-top:15px; cursor:pointer; font-weight:bold;">ZAPISZ KLUCZE W PRZEGLĄDARCE</button>
            </div>
        </div>
    </div>

    <div id="tab-kinematics" class="tab-content">
        <div class="card">
            <h3>🎛️ Live Kinematics (Zero-Latency)</h3>
            <div class="kinematics-container">
                
                <div class="sliders-group">
                    <div class="slider-wrapper left">
                        <div class="slider-header"><span>L. Ręka (S4)</span> <span class="val-badge" id="val-4">307</span></div>
                        <input type="range" min="180" max="430" value="307" oninput="sendServo(4, this.value)">
                    </div>
                    <div class="slider-wrapper right">
                        <div class="slider-header"><span>P. Ręka (S5)</span> <span class="val-badge" id="val-5">307</span></div>
                        <input type="range" min="180" max="430" value="307" oninput="sendServo(5, this.value)">
                    </div>
                    <div class="slider-wrapper left">
                        <div class="slider-header"><span>L. Noga (S2)</span> <span class="val-badge" id="val-2">307</span></div>
                        <input type="range" min="180" max="430" value="307" oninput="sendServo(2, this.value)">
                    </div>
                    <div class="slider-wrapper right">
                        <div class="slider-header"><span>P. Noga (S3)</span> <span class="val-badge" id="val-3">307</span></div>
                        <input type="range" min="180" max="430" value="307" oninput="sendServo(3, this.value)">
                    </div>
                    <div class="slider-wrapper left">
                        <div class="slider-header"><span>L. Stopa (S0)</span> <span class="val-badge" id="val-0">307</span></div>
                        <input type="range" min="180" max="430" value="307" oninput="sendServo(0, this.value)">
                    </div>
                    <div class="slider-wrapper right">
                        <div class="slider-header"><span>P. Stopa (S1)</span> <span class="val-badge" id="val-1">307</span></div>
                        <input type="range" min="180" max="430" value="307" oninput="sendServo(1, this.value)">
                    </div>
                    <button onclick="resetAllServos()" style="padding:12px; background:#ff3366; color:#fff; border:none; border-radius:8px; cursor:pointer; font-weight:bold;">RESET DO PIONU (HOME)</button>
                </div>

                <div class="svg-container">
                    <svg viewBox="0 0 400 500" xmlns="http://www.w3.org/2000/svg">
                        <defs>
                            <linearGradient id="neonCyan" x1="0%" y1="0%" x2="100%" y2="100%">
                                <stop offset="0%" stop-color="#00ffcc" />
                                <stop offset="100%" stop-color="#00aa88" />
                            </linearGradient>
                            <linearGradient id="neonOrange" x1="0%" y1="0%" x2="100%" y2="100%">
                                <stop offset="0%" stop-color="#ffaa00" />
                                <stop offset="100%" stop-color="#cc5500" />
                            </linearGradient>
                        </defs>

                        <rect x="130" y="150" width="140" height="160" rx="15" fill="#222" stroke="url(#neonCyan)" stroke-width="4" />
                        <rect x="150" y="50" width="100" height="80" rx="20" fill="#222" stroke="#fff" stroke-width="3" />
                        <circle cx="175" cy="90" r="15" fill="url(#neonCyan)" />
                        <circle cx="225" cy="90" r="15" fill="url(#neonCyan)" />

                        <g id="svg-s4" class="bot-part" style="transform-origin: 120px 170px;">
                            <rect x="90" y="160" width="30" height="110" rx="10" fill="#333" stroke="#00bfff" stroke-width="3" />
                            <circle cx="105" cy="170" r="8" fill="#00bfff" /> </g>

                        <g id="svg-s5" class="bot-part" style="transform-origin: 280px 170px;">
                            <rect x="280" y="160" width="30" height="110" rx="10" fill="#333" stroke="#ffaa00" stroke-width="3" />
                            <circle cx="295" cy="170" r="8" fill="#ffaa00" />
                        </g>

                        <g id="svg-s2" class="bot-part" style="transform-origin: 160px 320px;">
                            <rect x="145" y="320" width="30" height="100" rx="8" fill="#333" stroke="#00bfff" stroke-width="3" />
                            <circle cx="160" cy="330" r="8" fill="#00bfff" />
                            
                            <g id="svg-s0" class="bot-part" style="transform-origin: 160px 420px;">
                                <rect x="120" y="420" width="80" height="25" rx="10" fill="#111" stroke="#00bfff" stroke-width="2" />
                                <circle cx="160" cy="425" r="5" fill="#fff" />
                            </g>
                        </g>

                        <g id="svg-s3" class="bot-part" style="transform-origin: 240px 320px;">
                            <rect x="225" y="320" width="30" height="100" rx="8" fill="#333" stroke="#ffaa00" stroke-width="3" />
                            <circle cx="240" cy="330" r="8" fill="#ffaa00" />
                            
                            <g id="svg-s1" class="bot-part" style="transform-origin: 240px 420px;">
                                <rect x="200" y="420" width="80" height="25" rx="10" fill="#111" stroke="#ffaa00" stroke-width="2" />
                                <circle cx="240" cy="425" r="5" fill="#fff" />
                            </g>
                        </g>
                    </svg>
                </div>
            </div>
        </div>
    </div>

    <script>
        // ==========================================
        // 1. ZARZĄDZANIE UI I PAMIĘCIĄ PRZEGLĄDARKI
        // ==========================================
        function switchTab(tabName) {
            document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
            document.querySelectorAll('.tab-content').forEach(c => c.classList.remove('active'));
            
            event.currentTarget.classList.add('active');
            document.getElementById('tab-' + tabName).classList.add('active');
        }

        function saveApiKeys() {
            localStorage.setItem('api-gemini', document.getElementById('api-gemini').value);
            localStorage.setItem('api-supla', document.getElementById('api-supla').value);
            localStorage.setItem('supla-server', document.getElementById('supla-server').value);
            alert("Klucze zapisane lokalnie w przeglądarce!");
        }

        window.onload = () => {
            if(localStorage.getItem('api-gemini')) document.getElementById('api-gemini').value = localStorage.getItem('api-gemini');
            if(localStorage.getItem('api-supla')) document.getElementById('api-supla').value = localStorage.getItem('api-supla');
            if(localStorage.getItem('supla-server')) document.getElementById('supla-server').value = localStorage.getItem('supla-server');
            initWebSocket();
        };

        // ==========================================
        // 2. WEBSOCKETS & KINEMATYKA (Z SVG)
        // ==========================================
        var gateway = `ws://${window.location.hostname}/ws`;
        var websocket;
        
        function initWebSocket() {
            websocket = new WebSocket(gateway);
            websocket.onopen = () => { 
                let b = document.getElementById('ws-status');
                b.innerText = 'ONLINE: SYSTEM ZAZBROJONY'; 
                b.className = 'status-badge ok'; 
            };
            websocket.onclose = () => { 
                let b = document.getElementById('ws-status');
                b.innerText = 'OFFLINE: SZUKAM RDZENIA...'; 
                b.className = 'status-badge err'; 
                setTimeout(initWebSocket, 2000); 
            };
        }

        function sendServo(id, val) {
            // Aktualizacja tekstu
            document.getElementById('val-'+id).innerText = val;
            
            // MAPOWANIE NA KĄTY SVG
            // Środek Ticks = 307. Zakres ~180 do 430. Różnica +- 120 Ticks.
            // 120 Ticks to około 90 stopni fizycznego obrotu SG90.
            let angle = (val - 307) * 0.75; 
            
            // Opcje lustrzanego odbicia dla naturalnego wyglądu (lewa vs prawa strona)
            let invert = 1;
            if(id === 4 || id === 2 || id === 0) invert = -1; // Lewa strona kręci się odwrotnie w SVG dla symetrii

            let svgEl = document.getElementById('svg-s' + id);
            if(svgEl) {
                svgEl.style.transform = `rotate(${angle * invert}deg)`;
            }

            // Wysłanie natychmiastowe do ESP32-S3
            if (websocket && websocket.readyState === WebSocket.OPEN) {
                websocket.send("S" + id + ":" + val);
            }
        }

        function resetAllServos() {
            for(let i=0; i<6; i++) {
                document.querySelector(`input[oninput="sendServo(${i}, this.value)"]`).value = 307;
                sendServo(i, 307);
            }
        }

        // ==========================================
        // 3. LOGIKA AI, MOWY I SUPLI
        // ==========================================
        function logChat(who, text, type) {
            const box = document.getElementById('chat-log');
            let div = document.createElement('div');
            div.className = 'msg ' + type;
            div.innerHTML = `<strong>[${who}]</strong> ${text}`;
            box.appendChild(div);
            box.scrollTop = box.scrollHeight;
        }

        function startDictation() {
            if (window.hasOwnProperty('webkitSpeechRecognition')) {
                var recognition = new webkitSpeechRecognition();
                recognition.continuous = false;
                recognition.interimResults = false;
                recognition.lang = "pl-PL";

                let btn = document.getElementById('btn-speak');
                btn.classList.add('listening');
                document.getElementById('mic-text').innerText = "Nasłuchuję...";

                recognition.start();

                recognition.onresult = function(e) {
                    btn.classList.remove('listening');
                    document.getElementById('mic-text').innerText = "Mów do Wally'ego";
                    let userText = e.results[0][0].transcript;
                    logChat("TY", userText, 'user');
                    askGemini(userText);
                };

                recognition.onerror = function(e) {
                    btn.classList.remove('listening');
                    document.getElementById('mic-text').innerText = "Mów do Wally'ego";
                    logChat("SYS", "Przerwano nasłuch (Błąd mikrofonu).", 'err');
                }
            } else {
                logChat("SYS", "Twoja przeglądarka nie obsługuje Web Speech API.", 'err');
            }
        }

        async function askGemini(promptText) {
            const apiKey = document.getElementById('api-gemini').value.trim();
            if(!apiKey) return logChat("SYS", "Brak klucza Gemini! Wpisz go w ustawieniach.", 'err');

            logChat("WALLY", "Przetwarzanie...", 'ai');

            // POTWIERDZONY, BŁYSKAWICZNY MODEL
            const url = `https://generativelanguage.googleapis.com/v1beta/models/gemini-3.5-flash:generateContent?key=${apiKey}`;

            const payload = {
                systemInstruction: {
                    parts: [{ text: "Jesteś sympatycznym robotem o imieniu Wally. Stworzonym by bawić i pomagać Gabrysi. Jesteś krótki, zwięzły i zabawny. ZAWSZE rozpoczynaj odpowiedź od tagu ruchu, np. [RUCH:RADOSC] lub [RUCH:ZASKOCZENIE]. Jeśli prośba dotyczy włączenia/wyłączenia czegoś w domu, dodaj na końcu [SUPLA:ID:AKCJA] (gdzie AKCJA to TURN_ON lub TURN_OFF)." }]
                },
                contents: [{ role: "user", parts: [{ text: promptText }] }]
            };

            try {
                const response = await fetch(url, { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify(payload) });
                
                if (!response.ok) {
                    if(response.status === 503) return logChat("SYS", "Serwery Gemini są przeciążone. Spróbuj za chwilę.", 'err');
                    throw new Error(`HTTP ${response.status}`);
                }

                const data = await response.json();
                processAIResponse(data.candidates[0].content.parts[0].text);
            } catch (error) {
                logChat("SYS", "Błąd połączenia z mózgiem AI: " + error.message, 'err');
            }
        }

        function processAIResponse(text) {
            let cleanText = text;
            
            // Obsługa animacji ruchu
            const ruchMatch = text.match(/\[RUCH:(.*?)\]/);
            if(ruchMatch) {
                cleanText = cleanText.replace(ruchMatch[0], '').trim();
                let emocja = ruchMatch[1];
                if(emocja === "RADOSC") {
                    sendServo(4, 180); sendServo(5, 430); 
                    setTimeout(() => resetAllServos(), 2500);
                } else {
                    sendServo(4, 250); sendServo(5, 350); 
                    setTimeout(() => resetAllServos(), 2000);
                }
            }

            // Obsługa Supli
            const suplaMatch = text.match(/\[SUPLA:(\d+):(TURN_ON|TURN_OFF|TOGGLE)\]/);
            if(suplaMatch) {
                cleanText = cleanText.replace(suplaMatch[0], '').trim();
                executeSuplaAction(suplaMatch[1], suplaMatch[2]);
            }

            // Usunięcie poprzedniego "Przetwarzanie..." i dodanie ostatecznej wiadomości
            let chatBox = document.getElementById('chat-log');
            if (chatBox.lastChild.innerHTML.includes("Przetwarzanie...")) chatBox.removeChild(chatBox.lastChild);
            
            logChat("WALLY", cleanText, 'ai');
            speakText(cleanText);
        }

        function speakText(text) {
            if ('speechSynthesis' in window) {
                var utterance = new SpeechSynthesisUtterance(text);
                utterance.lang = 'pl-PL';
                utterance.rate = 1.1; 
                utterance.pitch = 1.2; // Trochę wyższy, bardziej "roboci" głos
                window.speechSynthesis.speak(utterance);
            }
        }

        async function executeSuplaAction(channelId, action) {
            const token = document.getElementById('api-supla').value.trim();
            const server = document.getElementById('supla-server').value.trim();
            if(!token || !server) return logChat("SYS", "Brak danych serwera Supla! Wpisz je w ustawieniach.", 'err');

            const url = `https://${server}/api/v2.3.0/channels/${channelId}`;
            try {
                const response = await fetch(url, {
                    method: 'PATCH',
                    headers: { 'Authorization': `Bearer ${token}`, 'Content-Type': 'application/json', 'Accept': 'application/json' },
                    body: JSON.stringify({ action: action })
                });
                if(response.ok) logChat("SUPLA", `Akcja (${action}) na kanale ID:${channelId} wysłana.`, 'user');
                else logChat("SYS", `Błąd Supli: Odrzucono (HTTP ${response.status})`, 'err');
            } catch (error) {
                logChat("SYS", "Problem sieciowy z połączeniem do Supli.", 'err');
            }
        }
    </script>
</body>
</html>
)rawliteral";

#endif