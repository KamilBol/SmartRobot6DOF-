#ifndef WEB_PAGES_H
#define WEB_PAGES_H

#include <Arduino.h>

const char SETUP_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html lang="pl"><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1.0"><title>Wally Setup</title><style>body{background:#121212;color:#00ffcc;font-family:sans-serif;text-align:center;padding:20px;}input{width:90%;padding:12px;margin:10px 0;border-radius:8px;border:1px solid #444;background:#2a2a2a;color:#fff;}input[type="submit"]{background:#00ffcc;color:#121212;cursor:pointer;font-weight:bold;}</style></head><body><h2>🤖 WAKE UP, WALLY</h2><form action="/connect" method="POST"><input type="text" name="ssid" placeholder="Nazwa Wi-Fi" required><br><input type="password" name="pass" placeholder="Hasło Wi-Fi" required><br><input type="submit" value="POŁĄCZ"></form></body></html>
)rawliteral";

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pl">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Wally 6DOF OS</title>
    <style>
        :root { --c-main: #00ffcc; --bg: #121212; --card: #1e1e1e; }
        body { background: var(--bg); color: #fff; font-family: 'Segoe UI', Tahoma, sans-serif; margin: 0; padding: 10px; }
        
        /* NAWIGACJA */
        .nav { display: flex; flex-wrap: wrap; justify-content: center; gap: 10px; margin-bottom: 20px; background: #0a0a0a; padding: 10px; border-radius: 12px; border: 1px solid #333; }
        .tab-btn { background: #222; color: #888; border: 1px solid #444; padding: 10px 15px; border-radius: 8px; cursor: pointer; font-weight: bold; transition: 0.3s; }
        .tab-btn.active { background: var(--card); color: var(--c-main); border-color: var(--c-main); box-shadow: 0 0 10px rgba(0,255,204,0.3); }
        .tab-content { display: none; animation: fadeIn 0.3s; }
        .tab-content.active { display: block; }
        @keyframes fadeIn { from { opacity: 0; } to { opacity: 1; } }

        /* WSPÓLNE */
        .card { background: var(--card); border: 1px solid #333; border-radius: 12px; padding: 20px; margin-bottom: 20px; }
        h3 { margin-top: 0; border-bottom: 1px solid #333; padding-bottom: 10px; color: var(--c-main); }
        input[type="text"], input[type="password"] { width: 100%; padding: 10px; margin: 5px 0 15px; border-radius: 5px; border: 1px solid #444; background: #111; color: #fff; box-sizing: border-box; }
        .btn { background: var(--c-main); color: #000; border: none; padding: 10px 20px; border-radius: 5px; font-weight: bold; cursor: pointer; width: 100%; }
        
        /* AI & SUPLA */
        .chat-box { background: #000; height: 250px; overflow-y: auto; padding: 10px; border: 1px solid #333; border-radius: 8px; margin-bottom: 15px; font-family: monospace; }
        .btn-mic { background: #ff0055; color: #fff; padding: 15px; font-size: 18px; border-radius: 30px; border:none; width: 100%; cursor: pointer; font-weight: bold; display: flex; justify-content: center; align-items: center; gap:10px; }
        .btn-mic.listening { background: #00aa88; animation: pulse 1s infinite; }
        @keyframes pulse { 0% { transform: scale(1); } 50% { transform: scale(1.02); } 100% { transform: scale(1); } }

        /* KINEMATYKA & WIZUALIZACJA */
        .kinematics-grid { display: grid; grid-template-columns: 1fr 250px 1fr; gap: 10px; align-items: center; }
        @media(max-width: 800px) { .kinematics-grid { grid-template-columns: 1fr; } }
        
        .control-group { background: #222; padding: 10px; border-radius: 8px; border: 1px solid #444; margin-bottom: 10px; text-align: center; }
        .control-group label { color: var(--c-main); font-weight: bold; font-size: 14px; }
        input[type="range"] { width: 100%; accent-color: var(--c-main); }
        .limits-row { display: flex; justify-content: space-between; gap: 5px; margin-top: 5px; }
        .limits-row input { width: 30%; padding: 5px; font-size: 12px; text-align: center; margin:0; }

        /* SVG ROBOT (Zgodnie ze zdjęciem) */
        svg { width: 100%; max-height: 400px; background: #0a0a0a; border-radius: 12px; border: 1px solid #333; }
        .svg-head { fill: #d1428f; } /* Różowa głowa */
        .svg-body { fill: #ffffff; } /* Biały korpus */
        .svg-arm { fill: #2bc1e6; } /* Niebieskie ręce */
        .svg-leg { fill: #2bc1e6; } /* Niebieskie nogi */
        .svg-foot { fill: #ffffff; } /* Białe stopy */
        .part { transition: transform 0.2s ease-out; }

        /* SENSORY ANIMACJE */
        .sensor-dash { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 20px; text-align: center; }
        .sensor-box { background: #111; border: 2px solid #333; padding: 20px; border-radius: 50%; width: 150px; height: 150px; margin: 0 auto; display: flex; flex-direction: column; justify-content: center; align-items: center; position: relative; }
        
        /* Sonar Radar */
        .radar-wave { position: absolute; width: 100%; height: 100%; border-radius: 50%; border: 2px solid var(--c-main); opacity: 0; }
        .radar-active .radar-wave { animation: radarAnim 1s ease-out; }
        @keyframes radarAnim { 0% { transform: scale(0.5); opacity: 1; } 100% { transform: scale(1.5); opacity: 0; } }
        
        /* Touch Glow */
        .touch-active { background: #ffaa00 !important; box-shadow: 0 0 30px #ffaa00; transition: 0.1s; }
        
        /* Mic Bar */
        .mic-bar { width: 20px; height: 10px; background: #ff0055; border-radius: 5px; transition: height 0.1s; }

    </style>
</head>
<body>

    <div class="nav">
        <button class="tab-btn active" onclick="switchTab('ai')">🧠 Asystent AI</button>
        <button class="tab-btn" onclick="switchTab('kine')">🦾 Studio Kalibracji</button>
        <button class="tab-btn" onclick="switchTab('sens')">📊 Sensory & Stan</button>
        <button class="tab-btn" onclick="switchTab('sd')">💾 Karta SD</button>
    </div>

    <div id="tab-ai" class="tab-content active">
        <div class="card">
            <h3>💬 Komunikator</h3>
            <button id="btn-speak" class="btn-mic" onclick="startDictation()">🎤 Mów do Wally'ego</button>
            <div id="chat-log" class="chat-box" style="margin-top:15px;"></div>
        </div>
        <div class="card">
            <h3>🔑 Pamięć Stała Kluczy (Zapisywane w przeglądarce)</h3>
            <label>Gemini API Key (AQ...)</label>
            <input type="password" id="api-gemini" placeholder="Wklej klucz Google AI">
            <label>Supla Personal Access Token</label>
            <input type="password" id="api-supla" placeholder="Wklej token Supli">
            <label>Supla Server</label>
            <input type="text" id="supla-server" placeholder="np. svr12.supla.org">
            <button class="btn" onclick="saveApiKeys()">💾 ZAPISZ KLUCZE NA STAŁE</button>
        </div>
    </div>

    <div id="tab-kine" class="tab-content">
        <div class="card">
            <h3>🎛️ Wizualizacja i Twarde Limity (Flash)</h3>
            <p style="font-size:12px; color:#aaa;">Ustaw odpowiednie granice, przetestuj suwakiem, a następnie zapisz na stałe w pamięci ESP32.</p>
            
            <div class="kinematics-grid">
                
                <div>
                    <div class="control-group">
                        <label>L. RĘKA (S4) - Wartość: <span id="v4">307</span></label>
                        <input type="range" id="sl4" min="150" max="450" value="307" oninput="s(4, this.value)">
                        <div class="limits-row">
                            <input type="text" id="min4" title="MIN" value="190">
                            <input type="text" id="home4" title="HOME" value="307">
                            <input type="text" id="max4" title="MAX" value="410">
                        </div>
                    </div>
                    <div class="control-group">
                        <label>L. NOGA (S2) - Wartość: <span id="v2">307</span></label>
                        <input type="range" id="sl2" min="150" max="450" value="307" oninput="s(2, this.value)">
                        <div class="limits-row">
                            <input type="text" id="min2" value="190"><input type="text" id="home2" value="307"><input type="text" id="max2" value="410">
                        </div>
                    </div>
                    <div class="control-group">
                        <label>L. STOPA (S0) - Wartość: <span id="v0">307</span></label>
                        <input type="range" id="sl0" min="150" max="450" value="307" oninput="s(0, this.value)">
                        <div class="limits-row">
                            <input type="text" id="min0" value="190"><input type="text" id="home0" value="307"><input type="text" id="max0" value="410">
                        </div>
                    </div>
                </div>

                <div>
                    <svg viewBox="0 0 300 400" xmlns="http://www.w3.org/2000/svg">
                        <g id="svg-4" class="part" style="transform-origin: 100px 180px;">
                            <rect x="20" y="165" width="80" height="30" rx="5" class="svg-arm" stroke="#000" stroke-width="2"/>
                            <circle cx="100" cy="180" r="8" fill="#111"/>
                        </g>
                        <g id="svg-5" class="part" style="transform-origin: 200px 180px;">
                            <rect x="200" y="165" width="80" height="30" rx="5" class="svg-arm" stroke="#000" stroke-width="2"/>
                            <circle cx="200" cy="180" r="8" fill="#111"/>
                        </g>

                        <rect x="100" y="150" width="100" height="80" rx="10" class="svg-body" stroke="#000" stroke-width="2"/>
                        
                        <rect x="110" y="50" width="80" height="80" rx="15" class="svg-head" stroke="#000" stroke-width="2"/>
                        <circle cx="130" cy="90" r="12" fill="#fff" stroke="#000" stroke-width="3"/> <circle cx="130" cy="90" r="5" fill="#111"/>
                        <circle cx="170" cy="90" r="12" fill="#fff" stroke="#000" stroke-width="3"/> <circle cx="170" cy="90" r="5" fill="#111"/>

                        <g id="svg-2" class="part" style="transform-origin: 125px 230px;">
                            <rect x="110" y="230" width="30" height="90" rx="5" class="svg-leg" stroke="#000" stroke-width="2"/>
                            <circle cx="125" cy="230" r="6" fill="#111"/>
                            
                            <g id="svg-0" class="part" style="transform-origin: 125px 320px;">
                                <path d="M 90 340 L 140 340 L 130 320 L 100 320 Z" class="svg-foot" stroke="#000" stroke-width="2"/>
                                <circle cx="125" cy="320" r="5" fill="#111"/>
                            </g>
                        </g>

                        <g id="svg-3" class="part" style="transform-origin: 175px 230px;">
                            <rect x="160" y="230" width="30" height="90" rx="5" class="svg-leg" stroke="#000" stroke-width="2"/>
                            <circle cx="175" cy="230" r="6" fill="#111"/>
                            
                            <g id="svg-1" class="part" style="transform-origin: 175px 320px;">
                                <path d="M 160 340 L 210 340 L 200 320 L 170 320 Z" class="svg-foot" stroke="#000" stroke-width="2"/>
                                <circle cx="175" cy="320" r="5" fill="#111"/>
                            </g>
                        </g>
                    </svg>
                    <button class="btn" style="background:#ff3366; margin-top:10px;" onclick="saveLimitsToFlash()">💾 ZAPISZ KALIBRACJĘ DO FLASH</button>
                </div>

                <div>
                    <div class="control-group">
                        <label>P. RĘKA (S5) - Wartość: <span id="v5">307</span></label>
                        <input type="range" id="sl5" min="150" max="450" value="307" oninput="s(5, this.value)">
                        <div class="limits-row">
                            <input type="text" id="min5" value="190"><input type="text" id="home5" value="307"><input type="text" id="max5" value="410">
                        </div>
                    </div>
                    <div class="control-group">
                        <label>P. NOGA (S3) - Wartość: <span id="v3">307</span></label>
                        <input type="range" id="sl3" min="150" max="450" value="307" oninput="s(3, this.value)">
                        <div class="limits-row">
                            <input type="text" id="min3" value="190"><input type="text" id="home3" value="307"><input type="text" id="max3" value="410">
                        </div>
                    </div>
                    <div class="control-group">
                        <label>P. STOPA (S1) - Wartość: <span id="v1">307</span></label>
                        <input type="range" id="sl1" min="150" max="450" value="307" oninput="s(1, this.value)">
                        <div class="limits-row">
                            <input type="text" id="min1" value="190"><input type="text" id="home1" value="307"><input type="text" id="max1" value="410">
                        </div>
                    </div>
                </div>
            </div>
        </div>
    </div>

    <div id="tab-sens" class="tab-content">
        <div class="card">
            <h3>📡 Telemetria i Odruchy Na Żywo</h3>
            <div style="margin-bottom: 20px; font-family:monospace; color:var(--c-main);">
                Pamięć RAM (PSRAM): <span id="sys-ram">Czekam na dane...</span>
            </div>
            
            <div class="sensor-dash">
                <div class="sensor-box" id="sens-sonar">
                    <div class="radar-wave"></div>
                    <div class="radar-wave" style="animation-delay: 0.5s;"></div>
                    <div style="z-index:2; font-weight:bold; font-size:24px;" id="val-sonar">-- cm</div>
                    <div style="z-index:2; font-size:12px; color:#aaa;">SONAR</div>
                </div>
                
                <div class="sensor-box" id="sens-touch">
                    <div style="font-size:40px;">🖐️</div>
                    <div style="font-weight:bold; margin-top:5px;" id="val-touch">BRAK</div>
                    <div style="font-size:12px; color:#aaa;">DOTYK GŁOWY</div>
                </div>

                <div class="sensor-box" id="sens-mic">
                    <div class="mic-bar" id="val-mic"></div>
                    <div style="font-weight:bold; margin-top:15px;">I2S MIC</div>
                    <div style="font-size:12px; color:#aaa;">HAŁAS</div>
                </div>
            </div>
        </div>
    </div>

    <div id="tab-sd" class="tab-content">
        <div class="card">
            <h3>💾 Zarządzanie Plikami (Karta MicroSD)</h3>
            <input type="file" id="fileInput" accept=".mp3,.wav" style="background:#222; border:none;">
            <button class="btn" onclick="uploadFile()">WGRAJ PLIK AUDIO</button>
            <p id="uploadStatus" style="margin-top: 15px;"></p>
        </div>
    </div>

    <script>
        // --- 1. ZARZĄDZANIE UI ---
        function switchTab(t) {
            document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
            document.querySelectorAll('.tab-content').forEach(c => c.classList.remove('active'));
            event.currentTarget.classList.add('active');
            document.getElementById('tab-' + t).classList.add('active');
        }

        function saveApiKeys() {
            localStorage.setItem('gem', document.getElementById('api-gemini').value);
            localStorage.setItem('sup', document.getElementById('api-supla').value);
            localStorage.setItem('srv', document.getElementById('supla-server').value);
            alert("✅ Zapisano w pamięci przeglądarki!");
        }

        window.onload = () => {
            if(localStorage.getItem('gem')) document.getElementById('api-gemini').value = localStorage.getItem('gem');
            if(localStorage.getItem('sup')) document.getElementById('api-supla').value = localStorage.getItem('sup');
            if(localStorage.getItem('srv')) document.getElementById('supla-server').value = localStorage.getItem('srv');
            initWS();
        };

        // --- 2. WEBSOCKETS & SENSORY ---
        var ws;
        function initWS() {
            ws = new WebSocket(`ws://${window.location.hostname}/ws`);
            ws.onmessage = (e) => {
                let msg = e.data;
                // PARSOWANIE TELEMETRII
                if(msg.startsWith("RAM:")) document.getElementById('sys-ram').innerText = msg.split(":")[1] + " B";
                if(msg.startsWith("SONAR:")) {
                    let dist = msg.split(":")[1];
                    document.getElementById('val-sonar').innerText = dist + " cm";
                    let sb = document.getElementById('sens-sonar');
                    sb.classList.add('radar-active');
                    setTimeout(()=>sb.classList.remove('radar-active'), 500);
                }
                if(msg === "TOUCH:1") {
                    let tb = document.getElementById('sens-touch');
                    document.getElementById('val-touch').innerText = "WYKRYTO!";
                    tb.classList.add('touch-active');
                    setTimeout(()=>{tb.classList.remove('touch-active'); document.getElementById('val-touch').innerText="BRAK";}, 1000);
                }
                if(msg === "MIC:PEAK") {
                    document.getElementById('val-mic').style.height = "60px";
                    setTimeout(()=>document.getElementById('val-mic').style.height = "10px", 200);
                }
            };
        }

        // --- 3. KINEMATYKA & SVG ANIMACJA ---
        function s(id, val) {
            document.getElementById('v'+id).innerText = val;
            if(ws && ws.readyState === WebSocket.OPEN) ws.send("S"+id+":"+val);

            // Wizualizacja SVG
            let svg = document.getElementById('svg-'+id);
            if(!svg) return;
            
            let angle = (val - 307) * 0.7; // Przelicznik Ticks na Stopnie
            
            // Specyficzne osie obrotu zależą od stawu
            if(id === 4) svg.style.transform = `rotate(${angle}deg)`; // Lewa Ręka (Oś Z)
            if(id === 5) svg.style.transform = `rotate(${-angle}deg)`; // Prawa ręka (Oś Z odwrotnie)
            if(id === 2 || id === 3) svg.style.transform = `skewX(${angle/2}deg)`; // Nogi (Oś Y symulowana Skew)
            if(id === 0 || id === 1) svg.style.transform = `translateY(${angle/5}px) rotate(${angle/3}deg)`; // Stopy
        }

        function saveLimitsToFlash() {
            // Zbieranie danych z pól input i wysłanie paczki JSON/String przez WS do zapisania w NVS
            for(let i=0; i<6; i++) {
                let min = document.getElementById('min'+i).value;
                let max = document.getElementById('max'+i).value;
                let hom = document.getElementById('home'+i).value;
                if(ws && ws.readyState === WebSocket.OPEN) {
                    ws.send(`CFG:${i}:${min}:${max}:${hom}`);
                }
            }
            alert("✅ Wysłano żądanie zapisu kalibracji do układu ESP32 NVS.");
        }

        // --- 4. AI & SUPLA ---
        function logC(w, t) {
            let b = document.getElementById('chat-log');
            b.innerHTML += `<div><strong>${w}:</strong> ${t}</div><br>`;
            b.scrollTop = b.scrollHeight;
        }

        function startDictation() {
            if(!window.webkitSpeechRecognition) return alert("Brak obsługi WebSpeech API w tej przeglądarce!");
            var r = new webkitSpeechRecognition();
            r.lang = "pl-PL";
            document.getElementById('btn-speak').classList.add('listening');
            r.start();
            r.onresult = (e) => {
                document.getElementById('btn-speak').classList.remove('listening');
                let txt = e.results[0][0].transcript;
                logC("TY", txt);
                askAI(txt);
            };
            r.onerror = () => document.getElementById('btn-speak').classList.remove('listening');
        }

        async function askAI(txt) {
            let key = localStorage.getItem('gem');
            if(!key) return logC("SYS", "Brak klucza Gemini!");
            logC("WALLY", "Myślę...");

            const url = `https://generativelanguage.googleapis.com/v1beta/models/gemini-3.5-flash:generateContent?key=${key}`;
            const p = {
                systemInstruction: { parts: [{text: "Jesteś robotem Wally. Zawsze daj tag np [RUCH:RADOSC]. Dla smart home daj [SUPLA:ID:TURN_ON/TURN_OFF]."}] },
                contents: [{role:"user", parts:[{text:txt}]}]
            };

            try {
                let res = await fetch(url, { method:'POST', body:JSON.stringify(p) });
                let data = await res.json();
                procAI(data.candidates[0].content.parts[0].text);
            } catch(e) { logC("ERR", "Błąd chmury AI"); }
        }

        function procAI(txt) {
            let cTxt = txt.replace(/\[RUCH:.*?\]/g, ''); // Czyszczenie
            
            // Obsługa Supli
            let sMatch = txt.match(/\[SUPLA:(\d+):(TURN_ON|TURN_OFF)\]/);
            if(sMatch) runSupla(sMatch[1], sMatch[2]);

            // Animacja gadania
            s(4, 250); s(5, 350); setTimeout(()=> { s(4,307); s(5,307); }, 1500);

            // Wyświetlenie i mowa
            document.getElementById('chat-log').lastChild.previousSibling.innerHTML = `<strong>WALLY:</strong> ${cTxt}`;
            let u = new SpeechSynthesisUtterance(cTxt); u.lang='pl-PL'; u.pitch=1.3; window.speechSynthesis.speak(u);
        }

        async function runSupla(id, act) {
            let tok = localStorage.getItem('sup');
            let srv = localStorage.getItem('srv');
            if(!tok || !srv) return logC("SYS", "Brak danych Supli!");
            try {
                await fetch(`https://${srv}/api/v2.3.0/channels/${id}`, {
                    method:'PATCH', headers:{'Authorization':`Bearer ${tok}`, 'Content-Type':'application/json'}, body:JSON.stringify({action:act})
                });
                logC("SUPLA", `Wykonano ${act} na kanale ${id}`);
            } catch(e) { logC("ERR", "Błąd sieci z Suplą"); }
        }

        // --- 5. SD UPLOAD ---
        function uploadFile() {
            let fi = document.getElementById('fileInput').files[0];
            if(!fi) return;
            let fd = new FormData(); fd.append("file", fi, "/"+fi.name);
            document.getElementById('uploadStatus').innerText = "Wgrywanie...⏳";
            fetch('/api/upload', {method:'POST', body:fd}).then(()=>document.getElementById('uploadStatus').innerText="✅ Wgrano!");
        }
    </script>
</body>
</html>
)rawliteral";

#endif