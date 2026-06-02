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
        
        .nav { display: flex; flex-wrap: wrap; justify-content: center; gap: 10px; margin-bottom: 20px; background: #0a0a0a; padding: 10px; border-radius: 12px; border: 1px solid #333; }
        .tab-btn { background: #222; color: #888; border: 1px solid #444; padding: 10px 15px; border-radius: 8px; cursor: pointer; font-weight: bold; transition: 0.3s; }
        .tab-btn.active { background: var(--card); color: var(--c-main); border-color: var(--c-main); box-shadow: 0 0 10px rgba(0,255,204,0.3); }
        .tab-content { display: none; animation: fadeIn 0.3s; }
        .tab-content.active { display: block; }
        @keyframes fadeIn { from { opacity: 0; } to { opacity: 1; } }

        .card { background: var(--card); border: 1px solid #333; border-radius: 12px; padding: 20px; margin-bottom: 20px; position: relative; }
        h3 { margin-top: 0; border-bottom: 1px solid #333; padding-bottom: 10px; color: var(--c-main); }
        input[type="text"], input[type="password"], input[type="number"] { width: 100%; padding: 10px; margin: 5px 0 15px; border-radius: 5px; border: 1px solid #444; background: #111; color: #fff; box-sizing: border-box; }
        .btn { background: var(--c-main); color: #000; border: none; padding: 10px 20px; border-radius: 5px; font-weight: bold; cursor: pointer; width: 100%; transition: 0.2s; margin-bottom: 5px;}
        .btn:hover { background: #00ccaa; }
        .btn-red { background: #ff3366; color: #fff; }
        .btn-blue { background: #00bfff; color: #fff; }
        
        .chat-box { background: #000; height: 250px; overflow-y: auto; padding: 10px; border: 1px solid #333; border-radius: 8px; margin-bottom: 15px; font-family: monospace; }
        .btn-mic { background: #ff0055; color: #fff; padding: 15px; font-size: 18px; border-radius: 30px; border:none; width: 100%; cursor: pointer; font-weight: bold; display: flex; justify-content: center; align-items: center; gap:10px; }
        .btn-mic.listening { background: #00aa88; animation: pulse 1s infinite; }
        @keyframes pulse { 0% { transform: scale(1); } 50% { transform: scale(1.02); } 100% { transform: scale(1); } }

        /* BAZA SUPLA & PLIKI */
        .table-wrapper { overflow-x: auto; margin-top: 15px; }
        table { width: 100%; border-collapse: collapse; font-size: 14px; min-width: 500px; }
        th, td { border: 1px solid #444; padding: 8px; text-align: left; }
        th { background: #333; color: var(--c-main); }
        textarea { width: 100%; height: 80px; background: #111; color: #fff; border: 1px solid #444; padding: 10px; border-radius: 5px; font-family: monospace; box-sizing: border-box; }

        /* KINEMATYKA & KALIBRACJA */
        .calib-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 15px; }
        .calib-box { background: #111; padding: 15px; border: 1px solid #333; border-radius: 8px; text-align: center;}
        .calib-inputs { display: flex; gap: 5px; margin-top: 10px; }
        .calib-inputs input { margin: 0; text-align: center; font-weight: bold;}
        
        input[type="range"] { width: 100%; accent-color: var(--c-main); margin-top: 10px;}
        .anim-grid { display: grid; grid-template-columns: repeat(auto-fill, minmax(140px, 1fr)); gap: 10px; }
    </style>
</head>
<body>

    <div class="nav">
        <button class="tab-btn active" onclick="switchTab('ai')">🧠 Asystent AI</button>
        <button class="tab-btn" onclick="switchTab('kine')">🦾 Sterowanie & Kalibracja</button>
        <button class="tab-btn" onclick="switchTab('anim')">🎬 Baza Animacji</button>
        <button class="tab-btn" onclick="switchTab('sd')">💾 Menedżer SD</button>
    </div>

    <div id="tab-ai" class="tab-content active">
        <div class="card">
            <h3>💬 Komunikator AI</h3>
            <button id="btn-speak" class="btn-mic" onclick="startDictation()">🎤 Mów do Wally'ego</button>
            <div id="chat-log" class="chat-box" style="margin-top:15px;"></div>
        </div>
        <div class="card">
            <h3>🔑 Globalna Konfiguracja (Zapis na MicroSD)</h3>
            <input type="password" id="api-gemini" placeholder="Gemini API Key (AQ...)">
            <input type="password" id="api-supla" placeholder="Supla Personal Access Token">
            <input type="text" id="supla-server" placeholder="Supla Server (np. svr12.supla.org)">
            <button class="btn" onclick="saveGlobalConfig()">💾 ZAPISZ W ROBOCIE</button>
        </div>
    </div>

    <div id="tab-kine" class="tab-content">
        <div class="card" style="border-color: #ffaa00;">
            <h3 style="color: #ffaa00;">⚡ Globalna Prędkość Ruchu</h3>
            <p style="font-size:12px; color:#aaa;">Zmień, jeśli robot rusza się za wolno. (Im mniej ms, tym szybciej). Pamiętaj: zbyt szybko = mniejsza precyzja i większy prąd!</p>
            <input type="range" min="2" max="50" value="15" oninput="sendSpeed(this.value)">
            <div style="text-align:center; font-weight:bold; color:#ffaa00; font-size:20px;" id="spd-val">15 ms / krok</div>
        </div>

        <div class="card">
            <h3>🎛️ Suwaki Live i Twarde Limity (Flash)</h3>
            <button class="btn btn-red" onclick="resetServos()" style="margin-bottom:15px;">PION (HOME)</button>
            <div class="calib-grid">
                <div class="calib-box">
                    <label style="color:var(--c-main); font-weight:bold;">L. RĘKA (S4)</label>
                    <input type="range" id="sl4" min="100" max="500" value="307" oninput="s(4, this.value)">
                    <div style="font-family:monospace; margin-bottom:5px;">VAL: <span id="v4">307</span></div>
                    <div class="calib-inputs">
                        <input type="number" id="min4" placeholder="MIN" title="MIN">
                        <input type="number" id="hom4" placeholder="HOME" title="HOME">
                        <input type="number" id="max4" placeholder="MAX" title="MAX">
                    </div>
                    <button class="btn btn-blue" style="margin-top:5px; font-size:12px;" onclick="saveLim(4)">Wgraj Limit</button>
                </div>
                <div class="calib-box">
                    <label style="color:var(--c-main); font-weight:bold;">P. RĘKA (S5)</label>
                    <input type="range" id="sl5" min="100" max="500" value="307" oninput="s(5, this.value)">
                    <div style="font-family:monospace; margin-bottom:5px;">VAL: <span id="v5">307</span></div>
                    <div class="calib-inputs"><input type="number" id="min5"><input type="number" id="hom5"><input type="number" id="max5"></div>
                    <button class="btn btn-blue" style="margin-top:5px; font-size:12px;" onclick="saveLim(5)">Wgraj Limit</button>
                </div>
                <div class="calib-box">
                    <label style="color:var(--c-main); font-weight:bold;">L. NOGA (S2)</label>
                    <input type="range" id="sl2" min="100" max="500" value="307" oninput="s(2, this.value)">
                    <div style="font-family:monospace; margin-bottom:5px;">VAL: <span id="v2">307</span></div>
                    <div class="calib-inputs"><input type="number" id="min2"><input type="number" id="hom2"><input type="number" id="max2"></div>
                    <button class="btn btn-blue" style="margin-top:5px; font-size:12px;" onclick="saveLim(2)">Wgraj Limit</button>
                </div>
                <div class="calib-box">
                    <label style="color:var(--c-main); font-weight:bold;">P. NOGA (S3)</label>
                    <input type="range" id="sl3" min="100" max="500" value="307" oninput="s(3, this.value)">
                    <div style="font-family:monospace; margin-bottom:5px;">VAL: <span id="v3">307</span></div>
                    <div class="calib-inputs"><input type="number" id="min3"><input type="number" id="hom3"><input type="number" id="max3"></div>
                    <button class="btn btn-blue" style="margin-top:5px; font-size:12px;" onclick="saveLim(3)">Wgraj Limit</button>
                </div>
                <div class="calib-box">
                    <label style="color:var(--c-main); font-weight:bold;">L. STOPA (S0)</label>
                    <input type="range" id="sl0" min="100" max="500" value="307" oninput="s(0, this.value)">
                    <div style="font-family:monospace; margin-bottom:5px;">VAL: <span id="v0">307</span></div>
                    <div class="calib-inputs"><input type="number" id="min0"><input type="number" id="hom0"><input type="number" id="max0"></div>
                    <button class="btn btn-blue" style="margin-top:5px; font-size:12px;" onclick="saveLim(0)">Wgraj Limit</button>
                </div>
                <div class="calib-box">
                    <label style="color:var(--c-main); font-weight:bold;">P. STOPA (S1)</label>
                    <input type="range" id="sl1" min="100" max="500" value="307" oninput="s(1, this.value)">
                    <div style="font-family:monospace; margin-bottom:5px;">VAL: <span id="v1">307</span></div>
                    <div class="calib-inputs"><input type="number" id="min1"><input type="number" id="hom1"><input type="number" id="max1"></div>
                    <button class="btn btn-blue" style="margin-top:5px; font-size:12px;" onclick="saveLim(1)">Wgraj Limit</button>
                </div>
            </div>
        </div>
    </div>

    <div id="tab-anim" class="tab-content">
        <div class="card">
            <h3>🎬 Biblioteka Sekwencji (Długie odtwarzanie)</h3>
            <p style="font-size:12px; color:#aaa;">Poniższe sekwencje mogą trwać od kilku do kilkunastu sekund. Robot wykona je w pełni asynchronicznie.</p>
            <div class="anim-grid">
                <button class="btn" onclick="pA('RADOSC')">Radość</button>
                <button class="btn" onclick="pA('SMUTEK')">Smutek</button>
                <button class="btn" onclick="pA('ZLOSC')">Złość</button>
                <button class="btn" onclick="pA('ZASKOCZENIE')">Zaskoczenie</button>
                <button class="btn" onclick="pA('SPANIE')">Spanie</button>
                <button class="btn" onclick="pA('TANIEC')">Taniec 1</button>
                <button class="btn" onclick="pA('TANIEC2')">Taniec 2 (Macarena)</button>
                <button class="btn" onclick="pA('BIEGANIE')">Bieganie</button>
                <button class="btn" onclick="pA('PODSKOKI')">Podskoki</button>
                <button class="btn" onclick="pA('MARSZ')">Marsz Żołnierza</button>
                <button class="btn" onclick="pA('POWITANIE')">Powitanie</button>
                <button class="btn" onclick="pA('SZUKANIE')">Szukanie (Rozglądanie)</button>
                <button class="btn" onclick="pA('STRACH')">Strach (Trzęsienie)</button>
                <button class="btn" onclick="pA('ZNUDZENIE')">Znudzenie</button>
                <button class="btn" onclick="pA('TRIUMF')">Triumf (Ręce w górę)</button>
                <button class="btn" onclick="pA('CZATOWANIE')">Czatowanie (Przysiad)</button>
                <button class="btn" onclick="pA('PLYWANIE')">Pływanie</button>
                <button class="btn" onclick="pA('BOKS')">Boksowanie</button>
                <button class="btn" onclick="pA('CWICZENIA')">Ćwiczenia (Przysiady)</button>
                <button class="btn" onclick="pA('PANIKA')">Panika!</button>
            </div>
            <button class="btn btn-red" onclick="resetServos()" style="margin-top:15px;">PRZERWIJ ANIMACJĘ (HOME)</button>
        </div>
    </div>

    <div id="tab-sd" class="tab-content">
        <div class="card">
            <h3>💾 Menedżer Plików KARTY MicroSD</h3>
            <p style="font-size:12px; color:#aaa;">Dodawaj, przeglądaj i usuwaj pliki muzyczne (.mp3, .wav) oraz konfiguracyjne bezpośrednio na karcie robota.</p>
            
            <div style="display:flex; gap:10px; margin-bottom:15px; align-items:center;">
                <input type="file" id="file-uploader" style="margin:0;">
                <button class="btn btn-blue" style="margin:0; width:150px;" onclick="uploadSDFile()">Wgraj Plik</button>
                <button class="btn" style="margin:0; width:150px;" onclick="loadSDFileList()">Odśwież Listę</button>
            </div>

            <div class="table-wrapper">
                <table>
                    <thead><tr><th>Nazwa Pliku</th><th>Rozmiar</th><th>Akcje</th></tr></thead>
                    <tbody id="sd-tbody">
                        <tr><td colspan="3" style="text-align:center;">Kliknij "Odśwież Listę", aby załadować pliki.</td></tr>
                    </tbody>
                </table>
            </div>
            
            <h3 style="margin-top:20px;">📥 Szybki Import Bazy Supla (CSV) do wally_config.json</h3>
            <textarea id="csv-input" placeholder="Wklej zawartość CSV (Nazwa;ID;Typ;Urzadzenie;Lokalizacja)..."></textarea>
            <button class="btn" onclick="importCSV()">Przetwórz i wyślij bazę do robota</button>
        </div>
    </div>

    <script>
        let globConfig = { gem: "", sup: "", srv: "", db: [] };
        let ws;

        // --- NAWIGACJA ---
        function switchTab(t) {
            document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
            document.querySelectorAll('.tab-content').forEach(c => c.classList.remove('active'));
            event.currentTarget.classList.add('active');
            document.getElementById('tab-' + t).classList.add('active');
        }

        window.onload = () => {
            fetchConfigFromRobot();
            initWS();
        };

        // --- WEBSOCKETS I KALIBRACJA ---
        function initWS() { ws = new WebSocket(`ws://${window.location.hostname}/ws`); }

        function s(id, val) {
            document.getElementById('sl'+id).value = val;
            document.getElementById('v'+id).innerText = val;
            if(ws && ws.readyState === WebSocket.OPEN) ws.send("S"+id+":"+val);
        }

        function sendSpeed(val) {
            document.getElementById('spd-val').innerText = val + " ms / krok";
            if(ws && ws.readyState === WebSocket.OPEN) ws.send("SPD:" + val);
        }

        function saveLim(id) {
            let min = document.getElementById('min'+id).value;
            let hom = document.getElementById('hom'+id).value;
            let max = document.getElementById('max'+id).value;
            if(!min || !max || !hom) return alert("Wypełnij MIN, HOME i MAX!");
            if(ws && ws.readyState === WebSocket.OPEN) {
                ws.send(`CFG:${id}:${min}:${max}:${hom}`);
                alert(`✅ Wysłano nowe limity dla Serwa ${id} (Zapisano we Flash)`);
            }
        }

        function resetServos() { for(let i=0; i<6; i++) s(i, 307); }

        // --- SILNIK 20 DŁUGICH ANIMACJI ---
        const delay = ms => new Promise(res => setTimeout(res, ms));
        let animActive = true;

        async function pA(name) {
            animActive = true;
            if(name === "RADOSC") {
                s(4,180); s(5,430); await delay(500); s(4,250); s(5,350); await delay(300); s(4,180); s(5,430); await delay(1000);
            } else if(name === "SMUTEK") {
                s(4,380); s(5,220); s(0,280); s(1,330); s(2,250); s(3,360); await delay(3000);
            } else if(name === "ZLOSC") {
                for(let i=0; i<8; i++){ if(!animActive) break; s(0,350); s(4,250); s(5,350); await delay(200); s(0,307); s(4,350); s(5,250); await delay(200); }
            } else if(name === "ZASKOCZENIE") {
                s(0,250); s(1,350); s(4,150); s(5,450); await delay(2000);
            } else if(name === "SPANIE") {
                s(4,350); s(5,260); s(0,260); s(1,350); s(2,260); s(3,350); await delay(4000); s(4,330); s(5,280); await delay(4000);
            } else if(name === "TANIEC") {
                for(let i=0; i<4; i++){ if(!animActive) break; s(0,350); s(1,350); s(4,200); await delay(500); s(0,250); s(1,250); s(4,307); s(5,400); await delay(500); }
            } else if(name === "TANIEC2") { // Macarena
                for(let i=0; i<2; i++){ if(!animActive) break; s(4,250); await delay(600); s(5,350); await delay(600); s(4,380); await delay(600); s(5,220); await delay(600); s(4,180); s(5,430); await delay(1000); s(2,250); s(3,350); await delay(500); s(2,307); s(3,307); await delay(500); }
            } else if(name === "BIEGANIE") {
                for(let i=0; i<8; i++){ if(!animActive) break; s(2,200); s(3,200); s(4,200); s(5,400); await delay(300); s(2,400); s(3,400); s(4,400); s(5,200); await delay(300); }
            } else if(name === "PODSKOKI") {
                for(let i=0; i<6; i++){ if(!animActive) break; s(0,400); s(1,200); s(4,200); s(5,400); await delay(300); s(0,307); s(1,307); s(4,307); s(5,307); await delay(300); }
            } else if(name === "MARSZ") {
                for(let i=0; i<6; i++){ if(!animActive) break; s(2,260); s(3,307); s(4,400); s(5,400); await delay(500); s(2,307); s(3,350); s(4,200); s(5,200); await delay(500); }
            } else if(name === "POWITANIE") {
                s(4,180); await delay(400); s(4,260); await delay(400); s(4,180); await delay(400); s(4,260); await delay(1000);
            } else if(name === "SZUKANIE") { // Rozglądanie
                s(2,200); s(3,200); s(4,250); await delay(1500); s(2,400); s(3,400); s(5,350); await delay(1500);
            } else if(name === "STRACH") {
                for(let i=0; i<15; i++){ if(!animActive) break; s(4,280); s(5,320); s(2,280); s(3,320); await delay(100); s(4,320); s(5,280); s(2,320); s(3,280); await delay(100); }
            } else if(name === "ZNUDZENIE") {
                s(4,330); s(5,280); s(0,250); await delay(3000); s(0,350); await delay(3000);
            } else if(name === "TRIUMF") {
                s(4,150); s(5,450); s(2,260); s(3,350); await delay(3000);
            } else if(name === "CZATOWANIE") {
                s(0,220); s(1,390); s(4,250); s(5,350); await delay(4000);
            } else if(name === "PLYWANIE") {
                for(let i=0; i<5; i++){ if(!animActive) break; s(4,180); s(5,307); await delay(500); s(4,307); s(5,430); await delay(500); s(4,400); s(5,307); await delay(500); s(4,307); s(5,200); await delay(500); }
            } else if(name === "BOKS") {
                for(let i=0; i<6; i++){ if(!animActive) break; s(4,180); s(5,307); s(2,260); await delay(250); s(4,307); s(5,430); s(2,350); await delay(250); }
            } else if(name === "CWICZENIA") {
                for(let i=0; i<5; i++){ if(!animActive) break; s(0,240); s(1,370); s(4,180); s(5,430); await delay(800); s(0,307); s(1,307); s(4,307); s(5,307); await delay(800); }
            } else if(name === "PANIKA") {
                for(let i=0; i<10; i++){ if(!animActive) break; s(0,180); s(1,430); s(4,150); s(5,450); await delay(150); s(0,430); s(1,180); s(4,450); s(5,150); await delay(150); }
            }
            if(animActive) resetServos();
        }

        // --- MENEDŻER PLIKÓW KARTY SD ---
        async function loadSDFileList() {
            let tb = document.getElementById('sd-tbody');
            tb.innerHTML = "<tr><td colspan='3' style='text-align:center;'>Ładowanie plików... ⏳</td></tr>";
            try {
                let res = await fetch('/api/files');
                if(!res.ok) throw new Error("Błąd API");
                let files = await res.json();
                tb.innerHTML = "";
                if(files.length === 0) { tb.innerHTML = "<tr><td colspan='3' style='text-align:center;'>Brak plików na karcie SD.</td></tr>"; return; }
                files.forEach(f => {
                    tb.innerHTML += `<tr>
                        <td>${f.name}</td>
                        <td>${(f.size/1024).toFixed(2)} KB</td>
                        <td>
                            <button class="btn" style="padding:5px; margin:0;" onclick="playSD('${f.name}')">▶️ Graj</button>
                            <button class="btn btn-red" style="padding:5px; margin:0;" onclick="delSD('${f.name}')">🗑️ Usuń</button>
                        </td>
                    </tr>`;
                });
            } catch(e) { tb.innerHTML = "<tr><td colspan='3' style='text-align:center; color:red;'>❌ Błąd odczytu karty SD. Upewnij się, że wgrałeś kod C++!</td></tr>"; }
        }

        async function uploadSDFile() {
            var input = document.getElementById('file-uploader').files;
            if(input.length === 0) return alert("Wybierz plik z dysku!");
            var fd = new FormData();
            fd.append("file", input[0], "/" + input[0].name); 
            
            try {
                let res = await fetch('/api/upload', { method: 'POST', body: fd });
                if(res.ok) { alert("✅ Wgrano pomyślnie!"); document.getElementById('file-uploader').value = ""; loadSDFileList(); }
                else alert("❌ Błąd wgrywania.");
            } catch(e) { alert("❌ Błąd sieci."); }
        }

        async function delSD(filename) {
            if(!confirm("Na pewno usunąć " + filename + "?")) return;
            var fd = new FormData(); fd.append("path", filename);
            await fetch('/api/delete', { method: 'POST', body: fd });
            loadSDFileList();
        }

        function playSD(filename) {
            // Tymczasowo możemy wysłać żądanie do API aby zagrał plik z SD (wymaga implementacji w C++)
            // lub po prostu odtworzyć w przeglądarce jeśli to serwer plików:
            let a = new Audio('/fs/' + filename); a.play();
        }

        // --- GLOBALNA KONFIGURACJA W ROBOCIE (JSON) ---
        async function fetchConfigFromRobot() {
            try {
                let res = await fetch('/fs/wally_config.json?t=' + Date.now());
                if(res.ok) {
                    let data = await res.json();
                    if(data.gem) document.getElementById('api-gemini').value = data.gem;
                    if(data.sup) document.getElementById('api-supla').value = data.sup;
                    if(data.srv) document.getElementById('supla-server').value = data.srv;
                    if(data.db) globConfig.db = data.db;
                    globConfig.gem = data.gem; globConfig.sup = data.sup; globConfig.srv = data.srv;
                }
            } catch (e) {}
        }

        async function saveGlobalConfig() {
            globConfig.gem = document.getElementById('api-gemini').value.trim();
            globConfig.sup = document.getElementById('api-supla').value.trim();
            globConfig.srv = document.getElementById('supla-server').value.trim();
            
            let blob = new Blob([JSON.stringify(globConfig)], { type: "application/json" });
            let fd = new FormData(); fd.append("file", blob, "wally_config.json");
            await fetch('/api/upload', { method: 'POST', body: fd });
            alert("✅ Konfiguracja i klucze zapisane na karcie robota!");
        }

        function importCSV() {
            let csv = document.getElementById('csv-input').value;
            let lines = csv.split('\n');
            let added = 0;
            lines.forEach(line => {
                let cols = line.split(';');
                if(cols.length >= 5 && cols[1].trim() !== "ID_Kanału") {
                    globConfig.db.push({ name: cols[0].trim(), loc: cols[4].trim(), id: cols[1].trim() });
                    added++;
                }
            });
            document.getElementById('csv-input').value = "";
            saveGlobalConfig(); // Wyślij zaktualizowany JSON na kartę SD
            alert(`✅ Przeanalizowano i dodano do bazy robota ${added} urządzeń!`);
        }

        // --- AI (MÓZG WALLY'EGO) ---
        function startDictation() {
            var r = new webkitSpeechRecognition(); r.lang = "pl-PL"; document.getElementById('btn-speak').classList.add('listening');
            r.start();
            r.onresult = (e) => {
                document.getElementById('btn-speak').classList.remove('listening');
                let txt = e.results[0][0].transcript; 
                document.getElementById('chat-log').innerHTML += `<div><strong>TY:</strong> ${txt}</div><br>`;
                askAI(txt);
            };
        }

        async function askAI(txt) {
            if(!globConfig.gem) return alert("Zapisz klucz Gemini w zakładce AI!");
            let suplaContext = globConfig.db.map(d => `- ${d.name} (${d.loc}) = ID ${d.id}`).join('\n');
            let sysInst = `Jesteś robotem Wally. Odpowiedz krótko. Zawsze podaj tag ruchu np [RUCH:RADOSC], [RUCH:PODSKAKIWANIE], [RUCH:TANIEC], [RUCH:BIEGANIE], [RUCH:ZASKOCZENIE], [RUCH:SMUTEK], [RUCH:MARSZ], [RUCH:POWITANIE], [RUCH:ZLOSC], [RUCH:SPANIE], [RUCH:SZUKANIE], [RUCH:STRACH], [RUCH:ZNUDZENIE], [RUCH:TRIUMF], [RUCH:CZATOWANIE], [RUCH:PLYWANIE], [RUCH:BOKS], [RUCH:CWICZENIA], [RUCH:PANIKA].
Jeśli użytkownik prosi o sprzęt domowy, wygeneruj [SUPLA:ID_KANALU:TURN_ON/TURN_OFF/TOGGLE]. 
BAZA URZĄDZEŃ W DOMU (Wstawiaj tylko cyfrowe ID z listy!):
${suplaContext}`;

            const url = `https://generativelanguage.googleapis.com/v1beta/models/gemini-3.5-flash:generateContent?key=${globConfig.gem}`;
            const p = { systemInstruction: { parts: [{text: sysInst}] }, contents: [{role:"user", parts:[{text:txt}]}] };

            let res = await fetch(url, { method:'POST', body:JSON.stringify(p) });
            let data = await res.json();
            
            let aiText = data.candidates[0].content.parts[0].text;
            let cTxt = aiText.replace(/\[RUCH:.*?\]/g, '').replace(/\[SUPLA:.*?\]/g, '').trim(); 
            
            let rMatch = aiText.match(/\[RUCH:(.*?)\]/); if(rMatch) pA(rMatch[1]);
            let sMatch = aiText.match(/\[SUPLA:(\d+):(TURN_ON|TURN_OFF|TOGGLE)\]/);
            if(sMatch) {
                fetch(`https://${globConfig.srv}/api/v2.3.0/channels/${sMatch[1]}`, {
                    method:'PATCH', headers:{'Authorization':`Bearer ${globConfig.sup}`, 'Content-Type':'application/json'}, body:JSON.stringify({action:sMatch[2]})
                });
            }

            document.getElementById('chat-log').innerHTML += `<div><strong>WALLY:</strong> ${cTxt}</div><br>`;
            let u = new SpeechSynthesisUtterance(cTxt); u.lang='pl-PL'; u.pitch=1.3; window.speechSynthesis.speak(u);
        }
    </script>
</body>
</html>
)rawliteral";

#endif