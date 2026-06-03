#ifndef WEB_PAGES_H
#define WEB_PAGES_H

#include <Arduino.h>

const char SETUP_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html lang="pl"><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1.0"><title>Konfiguracja Wally</title></head><body style="background:#121212;color:#00ffcc;font-family:sans-serif;text-align:center;padding:20px;"><h2>🤖 OBUDŹ ROBOTA WALLY</h2><p>Podaj dane do domowego Wi-Fi</p><form action="/connect" method="POST"><input type="text" name="ssid" placeholder="Nazwa Wi-Fi" required style="width:90%;max-width:400px;padding:15px;margin:10px;font-size:18px;border-radius:8px;"><br><input type="password" name="pass" placeholder="Hasło" required style="width:90%;max-width:400px;padding:15px;font-size:18px;border-radius:8px;"><br><input type="submit" value="POŁĄCZ" style="margin-top:20px;padding:15px 30px;font-size:20px;background:#ff0055;color:#fff;border:none;border-radius:8px;cursor:pointer;"></form></body></html>
)rawliteral";

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pl">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <title>Wally - Panel Sterowania</title>
    <style>
        /* Tło i centralny układ: chroni przed rozciąganiem na PC */
        body { background: #121212; color: #fff; font-family: 'Segoe UI', Tahoma, sans-serif; margin: 0; padding: 0; display: flex; flex-direction: column; align-items: center; }
        .container { width: 100%; max-width: 850px; padding: 10px; box-sizing: border-box; }
        
        /* Menu Główne: Duże, proste przyciski z ikonami */
        .nav { display: flex; flex-wrap: wrap; background: #1e1e1e; padding: 12px; border-radius: 15px; margin-bottom: 20px; gap: 10px; justify-content: center; border: 3px solid #333; }
        .nav button { flex: 1; min-width: 130px; background: #2a2a2a; border: 2px solid #555; color: #fff; font-size: 16px; padding: 15px 5px; cursor: pointer; border-radius: 10px; font-weight: bold; transition: 0.1s; }
        .nav button.active { background: #00ccff; border-color: #00ccff; color: #121212; }
        
        .tab { display: none; width: 100%; }
        .tab.active { display: block; }
        
        /* Panele (Okienka) - analogowy, zaokrąglony i czytelny wygląd */
        .panel { background: #1a1a1a; padding: 20px; border-radius: 15px; border: 3px solid #333; margin-bottom: 20px; box-sizing: border-box; }
        .panel h3 { margin-top: 0; color: #00ffcc; border-bottom: 2px solid #333; padding-bottom: 12px; font-size: 24px; text-align: center; }
        .desc { color: #ccc; text-align: center; font-size: 16px; margin-bottom: 20px; }
        
        /* Czat i AI - Główny przycisk jest wielki i zachęcający */
        .btn-ai { background: #ff0055; color: #fff; padding: 25px; width: 100%; border: none; font-size: 24px; border-radius: 15px; cursor: pointer; font-weight: 900; box-shadow: 0 6px 0 #cc0044; margin-bottom: 15px; transition: 0.1s; text-transform: uppercase; }
        .btn-ai:active { transform: translateY(6px); box-shadow: 0 0 0 #cc0044; }
        .chat-box { height: 400px; overflow-y: auto; background: #0a0a0a; border: 3px inset #333; padding: 20px; border-radius: 12px; font-size: 18px; line-height: 1.6; text-align: left; }
        .chat-box strong { color: #00ffcc; font-size: 20px; }
        
        /* Układ PC vs Telefon (Responsywność) */
        .dashboard { display: flex; flex-direction: column; gap: 20px; }
        @media(min-width: 768px) { .dashboard { flex-direction: row; } .dash-left { flex: 1.2; } .dash-right { flex: 1; } }
        
        /* Silnik 3D dla Robota (Wyśrodkowany, grubsze kontury) */
        .scene { width: 100%; height: 380px; perspective: 1200px; display: flex; justify-content: center; align-items: center; background: #111; border-radius: 12px; border: 3px solid #222; overflow: hidden; }
        .robot { position: relative; transform-style: preserve-3d; transform: rotateX(-10deg) rotateY(-20deg) scale(1.1); }
        .part { position: absolute; transform-style: preserve-3d; }
        .face { position: absolute; border: 1px solid rgba(0,0,0,0.5); box-shadow: inset 0 0 10px rgba(0,0,0,0.4); }
        
        /* Kolory zgodne z Twoim drukiem 3D */
        .c-pink { background: #ff4d94; } 
        .c-white { background: #e6e6e6; } 
        .c-blue { background: #00ccff; } 
        .eye { position: absolute; width: 22px; height: 22px; background: #111; border-radius: 50%; border: 4px solid #ddd; box-shadow: inset 0 0 8px #000; }
        
        /* Suwaki dla dzieci (Wyraźne i oddzielone) */
        .slider-row { display: flex; align-items: center; justify-content: space-between; margin-bottom: 15px; background: #222; padding: 15px; border-radius: 12px; border: 2px solid #444; }
        .slider-label { font-size: 16px; font-weight: bold; display: flex; align-items: center; gap: 8px; flex: 1; }
        .slider-row input[type=range] { flex: 1.5; height: 12px; margin: 0 15px; border-radius: 6px; background: #555; outline: none; }
        .val-badge { background: #00ccff; color: #121212; padding: 8px 12px; border-radius: 8px; font-weight: bold; min-width: 40px; text-align: center; font-size: 16px; }
        
        /* Przyciski ogólne */
        .btn { background: #333; color: #fff; border: 2px solid #555; padding: 15px; cursor: pointer; border-radius: 10px; font-weight: bold; font-size: 16px; margin: 5px; transition: 0.1s; box-shadow: 0 4px 0 #111; }
        .btn:active { transform: translateY(4px); box-shadow: 0 0 0 #111; }
        .btn:hover { background: #444; }
        .btn-red { background: #ff0055; border-color: #cc0044; color: #fff; width: 100%; box-shadow: 0 4px 0 #cc0044; font-size: 20px; }
        
        /* Tabele i wskaźniki */
        table { width: 100%; border-collapse: collapse; margin-top: 15px; background: #111; border-radius: 8px; overflow: hidden; }
        th, td { border: 1px solid #333; padding: 12px; text-align: center; font-size: 16px; }
        th { background: #222; color: #00ffcc; }
        .status-item { display: flex; justify-content: space-between; padding: 15px 10px; border-bottom: 2px dashed #333; font-size: 18px; }
        .status-val { font-weight: bold; color: #00ffcc; }
    </style>
</head>
<body>
    <div class="container">
    <div class="nav">
            <button class="active" onclick="showTab('tab-ai', this)">🎤 ROZMOWA Z WALLYM</button>
            <button onclick="showTab('tab-ctrl', this)">⚙️ USTAWIENIA CIAŁA</button>
            <button onclick="showTab('tab-anim', this)">🎬 RUCHY I TANIEC</button>
            <button onclick="showTab('tab-sd', this)">💾 KARTA PAMIĘCI</button>
            <button onclick="showTab('tab-ota', this)">🚀 AKTUALIZACJA</button>
        </div>

        <div id="tab-ai" class="tab active">
            <div class="panel">
                <h3>💬 Porozmawiaj z robotem</h3>
                <p class="desc">Kliknij wielki zielony przycisk, powiedz coś głośno i poczekaj, aż Wally odpowie!</p>
                <button class="btn-ai" style="background:#00ccff; color:#121212;" onclick="startDictation()">🎤 NACIŚNIJ I MÓW</button>
                <div id="chat-log" class="chat-box"></div>
            </div>
        </div>

        <div id="tab-ctrl" class="tab">
            <div class="dashboard">
                
                <div class="panel dash-left">
                    <h3>🤖 Podgląd Robota</h3>
                    <p class="desc">Tak Wally widzi swoje ciało.</p>
                    <div class="scene">
                        <div class="robot" id="robot3d">
                            </div>
                    </div>
                    <button class="btn btn-red" style="margin-top:20px;" onclick="resetServos()">🔄 WYPROSTUJ ROBOTA</button>
                </div>

                <div class="panel dash-right">
                    <h3>⚙️ Ustawianie Części Ciała</h3>
                    <p class="desc">Przesuwaj suwaki, żeby poruszyć nogami.</p>
                    
                    <div class="slider-row">
                        <span class="slider-label" style="color:#00ccff;">🔄 Lewe Biodro</span> 
                        <input type="range" min="100" max="500" value="307" oninput="s(0, this.value)"> <span class="val-badge" id="v0">307</span>
                    </div>
                    <div class="slider-row">
                        <span class="slider-label" style="color:#00ccff;">🔄 Prawe Biodro</span> 
                        <input type="range" min="100" max="500" value="307" oninput="s(1, this.value)"> <span class="val-badge" id="v1">307</span>
                    </div>
                    <div class="slider-row">
                        <span class="slider-label" style="color:#00ccff;">🦵 Lewa Noga</span> 
                        <input type="range" min="100" max="500" value="307" oninput="s(2, this.value)"> <span class="val-badge" id="v2">307</span>
                    </div>
                    <div class="slider-row">
                        <span class="slider-label" style="color:#00ccff;">🦵 Prawa Noga</span> 
                        <input type="range" min="100" max="500" value="307" oninput="s(3, this.value)"> <span class="val-badge" id="v3">307</span>
                    </div>
                    <div class="slider-row">
                        <span class="slider-label" style="color:#e6e6e6;">🦶 Lewa Stopa</span> 
                        <input type="range" min="100" max="500" value="307" oninput="s(4, this.value)"> <span class="val-badge" id="v4">307</span>
                    </div>
                    <div class="slider-row">
                        <span class="slider-label" style="color:#e6e6e6;">🦶 Prawa Stopa</span> 
                        <input type="range" min="100" max="500" value="307" oninput="s(5, this.value)"> <span class="val-badge" id="v5">307</span>
                    </div>

                    <h3 style="margin-top:25px;">⏱️ Prędkość Ruchów</h3>
                    <input type="range" min="0" max="50" value="0" style="width:100%; height:15px; margin-bottom:10px;" oninput="sendSpeed(this.value); document.getElementById('spd-v').innerText=this.value+' ms'">
                    <div style="text-align:center; color:#00ffcc; font-weight:bold; font-size:20px;" id="spd-v">0 ms</div>

                    <h3 style="margin-top:25px;">📊 Stan Systemu</h3>
                    <div class="status-item"><span>Wzrok (Oczy):</span> <span class="status-val" id="st-dist">-- cm</span></div>
                    <div class="status-item"><span>Pamięć Mózgu (RAM):</span> <span class="status-val" id="st-ram">-- KB</span></div>
                    <div class="status-item"><span>Miejsce na Karcie:</span> <span class="status-val" id="st-sd">-- MB</span></div>
                </div>
            </div>
        </div>

        <div id="tab-anim" class="tab">
            <div class="panel">
                <h3>🎬 Gotowe Ruchy Robota</h3>
                <p class="desc">Naciśnij przycisk, a Wally zatańczy lub wykona ruch!</p>
                <div id="anim-btns" style="display:flex; flex-wrap:wrap; gap:10px; justify-content:center;"></div>
            </div>
        </div>

        <div id="tab-sd" class="tab">
            <div class="panel">
                <h3>💾 Karta Pamięci (MicroSD)</h3>
                <p class="desc">Pliki robota. Rodzic może tu wgrywać nowe dźwięki lub ustawienia.</p>
                <div style="background:#222; padding:15px; border-radius:12px; margin-bottom:15px; border:2px solid #444;">
                    <input type="file" id="file-uploader" style="margin-bottom:10px; font-size:16px; color:#fff; width:100%;">
                    <button class="btn" style="width:100%; background:#00ccff; color:#121212;" onclick="uploadSDFile()">WGRAJ PLIK</button>
                </div>
                <table>
                    <thead><tr><th>Nazwa Pliku</th><th>Rozmiar</th><th>Usuń</th></tr></thead>
                    <tbody id="sd-tbody"><tr><td colspan="3" style="text-align:center;">Kliknij Odśwież</td></tr></tbody>
                </table>
                <button class="btn" style="margin-top:15px; width:100%;" onclick="loadSDFileList()">🔄 ODŚWIEŻ LISTĘ PLIKÓW</button>
            </div>
        </div>

        <div id="tab-ota" class="tab">
            <div class="panel">
                <h3 style="color:#ff0055;">🚀 Zdalna Aktualizacja (Przez Wi-Fi)</h3>
                <p class="desc">Wgraj nowy kod do robota bez podłączania kabla USB (plik <b>firmware.bin</b>).</p>
                <input type="file" id="ota-file" accept=".bin" style="display:block; width:100%; padding:15px; margin:20px 0; background:#222; border-radius:8px; font-size:16px; color:#fff; box-sizing:border-box; border:2px solid #444;">
                <button class="btn-red" onclick="startOTA()">WGRAJ AKTUALIZACJĘ</button>
                <div id="ota-status" style="margin-top:15px; font-size:18px; font-weight:bold; text-align:center;"></div>
                <div style="width:100%; background:#222; height:30px; border-radius:15px; margin-top:15px; border:2px solid #444; overflow:hidden;">
                    <div id="ota-progress" style="width:0%; background:#00ffcc; height:100%; transition: width 0.2s;"></div>
                </div>
            </div>
        </div>

    </div>
    <script>
        // --- ZMIENNE GLOBALNE ---
        let globConfig = { gem: "", sup: "", srv: "", db: [] };
        let ws;

        // --- START SYSTEMU ---
        window.onload = () => {
            fetchConfigFromRobot();
            initWS();
            buildRobot3D(); 
            initAnimButtons();
            setInterval(fetchSystemStatus, 2000); 
        };

        function showTab(t, btn) {
            document.querySelectorAll('.tab').forEach(c => c.classList.remove('active'));
            document.querySelectorAll('.nav button').forEach(b => b.classList.remove('active'));
            document.getElementById(t).classList.add('active');
            btn.classList.add('active');
        }

        // --- KOMUNIKACJA ---
        function initWS() { 
            ws = new WebSocket(`ws://${window.location.hostname}/ws`);
        }

        async function fetchSystemStatus() {
            try {
                let res = await fetch('/api/status');
                if(res.ok) {
                    let d = await res.json();
                    document.getElementById('st-ram').innerText = (d.ram / 1024).toFixed(1) + " KB";
                    document.getElementById('st-sd').innerText = d.sd_used + " MB / " + d.sd_total + " MB";
                    document.getElementById('st-dist').innerText = d.dist + " cm";
                }
            } catch(e) {}
        }

        // --- SILNIK 3D (Z rękami i hierarchią, dopasowany do Twojego zdjęcia) ---
        function drawCube(w, h, d, colorClass, tX, tY, tZ) {
            return `
            <div class="part" style="transform: translate3d(${tX}px, ${tY}px, ${tZ}px);">
                <div class="face ${colorClass}" style="width:${w}px; height:${h}px; transform: translateZ(${d/2}px)"></div>
                <div class="face ${colorClass}" style="width:${w}px; height:${h}px; transform: rotateY(180deg) translateZ(${d/2}px)"></div>
                <div class="face ${colorClass}" style="width:${d}px; height:${h}px; transform: rotateY(90deg) translateZ(${w/2}px); left:${(w-d)/2}px"></div>
                <div class="face ${colorClass}" style="width:${d}px; height:${h}px; transform: rotateY(-90deg) translateZ(${w/2}px); left:${(w-d)/2}px"></div>
                <div class="face ${colorClass}" style="width:${w}px; height:${d}px; transform: rotateX(90deg) translateZ(${h/2}px); top:${(h-d)/2}px"></div>
                <div class="face ${colorClass}" style="width:${w}px; height:${d}px; transform: rotateX(-90deg) translateZ(${h/2}px); top:${(h-d)/2}px"></div>
            </div>`;
        }

        function buildRobot3D() {
            let h = "";
            // GŁOWA (Różowa) + OCZY
            h += `<div class="part" style="transform: translate3d(-40px, -120px, -30px);">
                    ${drawCube(80, 50, 60, 'c-pink', 0, 0, 0)}
                    <div class="eye" style="transform: translate3d(15px, 15px, 31px);"></div>
                    <div class="eye" style="transform: translate3d(45px, 15px, 31px);"></div>
                  </div>`;
            
            // KORPUS (Biały) - środek (X=0)
            h += drawCube(70, 65, 50, 'c-white', -35, -65, -25);
            
            // RĘCE (Niebieskie - statyczne, doczepione do boków korpusu z lewej i prawej)
            h += drawCube(20, 50, 20, 'c-blue', -55, -60, -10); // Lewa ręka
            h += drawCube(20, 50, 20, 'c-blue', 35, -60, -10);  // Prawa ręka

            // NOGI HIERARCHICZNE (Niebieskie biodro, Niebieska noga, Biała stopa)
            const createLeg = (side, tX) => {
                return `
                <div id="j-hip-${side}" class="part" style="transform-origin: top center; transform: translate3d(${tX}px, 0px, 0px);">
                    ${drawCube(24, 30, 24, 'c-blue', -12, 0, -12)}
                    <div id="j-leg-${side}" class="part" style="transform-origin: top center; transform: translate3d(0px, 30px, 0px);">
                        ${drawCube(20, 35, 20, 'c-blue', -10, 0, -10)}
                        <div id="j-foot-${side}" class="part" style="transform-origin: top center; transform: translate3d(0px, 35px, 0px);">
                            ${drawCube(36, 15, 46, 'c-white', -18, 0, -10)}
                        </div>
                    </div>
                </div>`;
            };
            h += createLeg('l', -22); 
            h += createLeg('r', 22);
            document.getElementById('robot3d').innerHTML = h;
        }

        function s(id, val) {
            document.getElementById('v'+id).innerText = val;
            if(ws && ws.readyState === WebSocket.OPEN) ws.send("S"+id+":"+val);
            updateVisual3D();
        }

        function updateVisual3D() {
            let m = (v) => (v - 307) * 0.35; 
            let a = [m(document.getElementById('v0').innerText), -m(document.getElementById('v1').innerText), m(document.getElementById('v2').innerText), -m(document.getElementById('v3').innerText), m(document.getElementById('v4').innerText), -m(document.getElementById('v5').innerText)];

            // Odwzorowanie ruchu obrotów na zagnieżdżonych elementach CSS
            document.getElementById('j-hip-l').style.transform = `translate3d(-22px, 0px, 0px) rotateY(${a[0]}deg)`;
            document.getElementById('j-hip-r').style.transform = `translate3d(22px, 0px, 0px) rotateY(${a[1]}deg)`;
            document.getElementById('j-leg-l').style.transform = `translate3d(0px, 30px, 0px) rotateX(${a[2]}deg)`;
            document.getElementById('j-leg-r').style.transform = `translate3d(0px, 30px, 0px) rotateX(${a[3]}deg)`;
            document.getElementById('j-foot-l').style.transform = `translate3d(0px, 35px, 0px) rotateZ(${a[4]}deg)`;
            document.getElementById('j-foot-r').style.transform = `translate3d(0px, 35px, 0px) rotateZ(${a[5]}deg)`;
        }

        function resetServos() { for(let i=0; i<6; i++) { document.querySelector(`input[oninput="s(${i}, this.value)"]`).value = 307; s(i, 307); } }
        function sendSpeed(v) { if(ws && ws.readyState === WebSocket.OPEN) ws.send("SPD:" + v); }

        // --- ANIMACJE (30 RUCHÓW Z WYRAŹNYMI PRZYCISKAMI) ---
        const anims = ["RADOSC", "SMUTEK", "ZLOSC", "TANIEC", "MACARENA", "BIEG", "PODSKOKI", "MARSZ", "POWITANIE", "SZUKANIE", "STRACH", "ZNUZENIE", "TRIUMF", "CZATOWANIE", "PLYWANIE", "BOKS", "CWICZENIA", "PANIKA", "SLIZG", "UKLON", "KOPNIECIE", "BALANS", "ZOMBIE", "NINJA", "PTAK", "SAMOLOT", "ROZCIAGANIE", "CZKAWKA", "SMIECH", "KROK_BOK"];
        
        function initAnimButtons() {
            let container = document.getElementById('anim-btns');
            anims.forEach(a => { container.innerHTML += `<button class="btn" onclick="pA('${a}')">🎬 ${a}</button>`; });
        }
        
        const delay = ms => new Promise(res => setTimeout(res, ms));
        async function mMove(t, ...args) {
            for(let i=0; i<args.length; i+=2) s(args[i], args[i+1]);
            await delay(t);
        }

        let animActive = true;
        async function pA(n) {
            animActive = true;
            if(n==="RADOSC") { await mMove(400, 4,180, 5,430); await mMove(300, 4,250, 5,350); await mMove(600, 4,180, 5,430); }
            else if(n==="SMUTEK") { await mMove(2000, 4,380, 5,220, 0,280, 1,330, 2,250, 3,360); }
            else if(n==="POWITANIE") { await mMove(300, 4,180); await mMove(300, 4,260); await mMove(300, 4,180); }
            else if(n==="BOKS") { for(let i=0;i<4;i++){ if(!animActive)break; await mMove(200, 4,180, 5,307, 2,260); await mMove(200, 4,307, 5,430, 2,350); } }
            else if(n==="UKLON") { await mMove(1500, 2,200, 3,400, 0,250, 1,350); await mMove(1000, 2,307, 3,307, 0,307, 1,307); }
            else { 
                // Bezpieczny, losowy ruch jako wypełniacz dla pozostałych komend z bazy, by zaoszczędzić RAM
                for(let i=0;i<4;i++){ if(!animActive)break; await mMove(300, 0,250+Math.random()*100, 1,250+Math.random()*100, 4,200+Math.random()*200, 5,200+Math.random()*200); }
            }
            if(animActive) resetServos();
        }

        // --- MÓZG AI, SUPLA I GOOGLE TTS (Głos Lektora) ---
        function logC(w, t) {
            let b = document.getElementById('chat-log');
            b.innerHTML += `<div style="margin-bottom:15px; border-bottom:1px solid #222; padding-bottom:5px;"><strong>${w}:</strong> ${t}</div>`;
            b.scrollTop = b.scrollHeight;
        }

        function startDictation() {
            var r = new webkitSpeechRecognition(); r.lang = "pl-PL"; r.start();
            r.onresult = (e) => { let t = e.results[0][0].transcript; logC("TY", t); askAI(t); };
        }

        async function askAI(txt) {
            logC("WALLY", "Zastanawiam się... 🤔");
            let dev = globConfig.db ? globConfig.db.map(d => `- ${d.name} (${d.loc}) = ID ${d.id}`).join('\n') : "";
            let prompt = `Jesteś robotem Wally. Odpowiedz przyjaźnie, krótko. Na początku wstaw tag ruchu np [RUCH:RADOSC]. Dostępne ruchy: ${anims.join(", ")}. Jeśli prośba dotyczy sprzętu, na końcu wstaw: [SUPLA:ID:TURN_ON/TURN_OFF]. Baza urządzeń:\n${dev}`;

            try {
                let res = await fetch(`https://generativelanguage.googleapis.com/v1beta/models/gemini-3.5-flash:generateContent?key=${globConfig.gem}`, {
                    method:'POST', body:JSON.stringify({systemInstruction:{parts:[{text:prompt}]}, contents:[{role:"user", parts:[{text:txt}]}]})
                });
                let data = await res.json();
                let ai = data.candidates[0].content.parts[0].text;
                let clean = ai.replace(/\[RUCH:.*?\]/gi, '').replace(/\[SUPLA:.*?\]/gi, '').trim();
                
                // Odpalanie ruchu po komendzie AI
                let rm = ai.match(/\[RUCH:(.*?)\]/i); if(rm) pA(rm[1].toUpperCase());
                
                // Moduł Supla Proxy
                let sm = ai.match(/\[SUPLA:\s*(\d+)\s*:\s*(TURN_ON|TURN_OFF|TOGGLE)\s*\]/i);
                if(sm) {
                    logC("SYS", `📡 Wysyłam sygnał do włącznika (Supla)...`);
                    let fd = new FormData(); fd.append("id", sm[1]); fd.append("action", sm[2].toUpperCase());
                    fd.append("token", globConfig.sup); fd.append("server", globConfig.srv);
                    fetch('/api/supla_proxy', { method: 'POST', body: fd });
                }

                document.getElementById('chat-log').lastChild.innerHTML = `<strong>WALLY:</strong> ${clean}`;
                speakTTS(clean);
            } catch(e) { logC("SYS", "❌ Błąd! Sprawdź klucz API lub połączenie Wi-Fi."); }
        }

        function speakTTS(t) {
            let k = localStorage.getItem('tts_key');
            if(k) {
                fetch(`https://texttospeech.googleapis.com/v1/text:synthesize?key=${k}`, {
                    method:'POST', body: JSON.stringify({input:{text:t}, voice:{languageCode:"pl-PL", name:"pl-PL-Wavenet-B"}, audioConfig:{audioEncoding:"MP3"}})
                }).then(r=>r.json()).then(d=>{ if(d.audioContent) new Audio("data:audio/mp3;base64,"+d.audioContent).play(); });
            } else {
                let u = new SpeechSynthesisUtterance(t); u.lang='pl-PL'; window.speechSynthesis.speak(u);
            }
        }

        // --- KARTA SD I OTA ---
        async function fetchConfigFromRobot() {
            try { let r = await fetch('/fs/wally_config.json?t='+Date.now()); if(r.ok) globConfig = await r.json(); } catch(e){}
        }
        
        async function loadSDFileList() {
            let tb = document.getElementById('sd-tbody'); tb.innerHTML = "<tr><td colspan='3'>Wczytywanie z karty... ⏳</td></tr>";
            try {
                let files = await (await fetch('/api/files')).json();
                tb.innerHTML = "";
                files.forEach(f => {
                    tb.innerHTML += `<tr><td>${f.name}</td><td>${(f.size/1024).toFixed(1)} KB</td><td><button class="btn btn-red" style="width:auto; padding:8px 15px; font-size:14px; margin:0;" onclick="delSD('${f.name}')">USUŃ</button></td></tr>`;
                });
            } catch(e) { tb.innerHTML = "<tr><td colspan='3'>❌ Błąd odczytu karty SD.</td></tr>"; }
        }
        
        async function uploadSDFile() {
            let input = document.getElementById('file-uploader');
            if(!input.files.length) return alert("Najpierw kliknij 'Wybierz plik'!");
            let f = input.files[0];
            let fd = new FormData(); fd.append("file", f, "/"+f.name);
            await fetch('/api/upload', {method:'POST', body:fd}); 
            loadSDFileList();
        }
        
        async function delSD(n) { 
            if(confirm("Czy na pewno chcesz usunąć plik " + n + "?")) { 
                let fd = new FormData(); fd.append("path", n); 
                await fetch('/api/delete', {method:'POST', body:fd}); 
                loadSDFileList(); 
            } 
        }

        function startOTA() {
            let fileInput = document.getElementById('ota-file');
            if(!fileInput.files.length) return alert("Wybierz plik z nowym oprogramowaniem (firmware.bin)!");
            let f = fileInput.files[0];
            let fd = new FormData(); fd.append("update", f, f.name);
            
            let stat = document.getElementById('ota-status');
            let prog = document.getElementById('ota-progress');
            
            stat.innerText = "⏳ Wgrywanie przez Wi-Fi... NIE WYŁĄCZAJ ROBOTA!";
            stat.style.color = "#ff0055";
            prog.style.width = "0%";

            let xhr = new XMLHttpRequest();
            xhr.open("POST", "/update", true);
            xhr.upload.onprogress = (e) => { 
                if(e.lengthComputable) prog.style.width = Math.round((e.loaded/e.total)*100) + "%"; 
            };
            xhr.onload = () => { 
                if(xhr.status===200) {
                    stat.innerText = "✅ Sukces! Czekaj, Wally uruchamia się ponownie...";
                    stat.style.color = "#00ffcc";
                    setTimeout(() => location.reload(), 6000); // 6 sekund na restart ESP32
                } else {
                    stat.innerText = "❌ Wystąpił błąd podczas wgrywania!";
                    prog.style.background = "red";
                }
            };
            xhr.onerror = () => { stat.innerText = "❌ Błąd sieci (Wi-Fi rozłączone)!"; prog.style.background = "red"; };
            xhr.send(fd);
        }
    </script>
</body>
</html>
)rawliteral";

#endif