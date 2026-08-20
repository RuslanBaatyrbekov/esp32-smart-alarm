#pragma once

const char* index_html = R"rawliteral(
<!DOCTYPE html><html><head><meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1">
<title>Smart Alarm Admin</title><style>
:root{--bg:#121212;--card:#1e1e1e;--text:#fff;--accent:#007bff;--danger:#dc3545;--warn:#ffc107;--success:#28a745;}
body{font-family:system-ui,-apple-system,sans-serif;background:var(--bg);color:var(--text);margin:0;padding:20px;display:flex;justify-content:center;}
.container{max-width:500px;width:100%;}
.card{background:var(--card);padding:20px;border-radius:12px;margin-bottom:20px;box-shadow:0 4px 15px rgba(0,0,0,0.3);}
h2,h3{margin-top:0;}
.flex-between{display:flex;justify-content:space-between;align-items:center;}
button{background:var(--accent);color:#fff;border:none;padding:10px 15px;border-radius:8px;font-size:1rem;cursor:pointer;transition:0.2s;}
button:active{opacity:0.8;}
.btn-danger{background:var(--danger);}
.btn-warn{background:var(--warn);color:#000;}
.btn-success{background:var(--success);}
.alarm-item{background:#2a2a2a;padding:15px;border-radius:8px;margin-bottom:10px;display:flex;justify-content:space-between;align-items:center;flex-wrap:wrap;gap:10px;}
.alarm-time{font-size:1.8rem;font-weight:bold;}
.toggle{cursor:pointer;accent-color:var(--accent);width:22px;height:22px;flex-shrink:0;}
.sunrise-label{display:flex;align-items:center;gap:4px;font-size:0.9rem;color:#aaa;cursor:pointer;}
.days-picker{display:flex;gap:4px;margin-top:8px;flex-wrap:wrap;}
.day-btn{background:#444;color:#aaa;border-radius:50%;width:26px;height:26px;display:flex;align-items:center;justify-content:center;font-size:0.75rem;cursor:pointer;user-select:none;transition:0.2s;flex-shrink:0;}
.day-btn.active{background:var(--accent);color:#fff;font-weight:bold;}
input[type="range"]{accent-color:var(--accent);}
textarea{width:100%; height:80px; background:#2a2a2a; color:#fff; border:1px solid #444; border-radius:8px; padding:10px; box-sizing:border-box; font-family:inherit; resize:vertical;}
</style></head><body>
<div class="container">
  
  <div class="card">
    <div class="flex-between">
      <h2>Время устройства</h2>
      <button onclick="syncTime()">Синхронизировать</button>
    </div>
    <p id="devTime" style="color:#aaa;margin-bottom:0;">Загрузка...</p>
  </div>

  <div class="card">
    <div class="flex-between" style="margin-bottom:15px;">
      <h3>Будильники</h3>
      <button onclick="addAlarm()">+ Добавить</button>
    </div>
    <div id="alarmsList"></div>
  </div>

  <div class="card">
    <div class="flex-between">
      <div style="display:flex;align-items:center;gap:10px;">
        <span style="font-size:1.8rem;">📱</span>
        <h2 style="margin:0;">Экран</h2>
      </div>
      <label class="sunrise-label" style="font-size:1rem;">
        <input type="checkbox" id="autoBrightToggle" onchange="toggleAutoBright(this.checked)"> Авто-режим
      </label>
    </div>
    <input type="range" id="brightSlider" min="1" max="255" style="width:100%; margin-top:20px;" onchange="setBrightness(this.value)">
  </div>

  <div class="card">
    <div class="flex-between">
      <div style="display:flex;align-items:center;gap:10px;">
        <span style="font-size:1.8rem;">💡</span>
        <h2 style="margin:0;">Светильник</h2>
      </div>
      <button id="lightBtn" onclick="toggleLight()" style="padding:12px 20px; font-weight:bold;">Загрузка...</button>
    </div>
  </div>

  <div class="card">
    <div class="flex-between" style="margin-bottom:12px;">
      <div style="display:flex;align-items:center;gap:10px;">
        <span style="font-size:1.8rem;">🚆</span>
        <h2 style="margin:0;">Поезда</h2>
      </div>
      <input type="checkbox" class="toggle" id="trainsToggle" onchange="toggleTrains(this.checked)">
    </div>
    <div style="display:flex; gap:10px; margin-bottom:10px; align-items:center;">
      <input type="time" id="queryTime" value="07:15" style="background:#2a2a2a; color:#fff; border:1px solid #444; border-radius:8px; padding:8px; font-size:1rem; flex:1;">
      <button onclick="queryTrainsByTime()" style="flex:1;">Смотреть</button>
    </div>
    <div style="display:flex; gap:10px;">
      <button onclick="showTrainsNow()" class="btn-success" style="flex:1;">Показать сейч.</button>
      <button onclick="hideTrainsNow()" class="btn-danger" style="flex:1;">Спрятать</button>
    </div>
  </div>

  <div class="card">
    <div class="flex-between" style="margin-bottom:10px;">
      <div style="display:flex;align-items:center;gap:10px;">
        <span style="font-size:1.8rem;">💬</span>
        <h2 style="margin:0;">Сообщение на экран</h2>
      </div>
    </div>
    <textarea id="customText" placeholder="Введите текст для отображения на экране..."></textarea>
    <div style="display:flex; gap:10px; margin-top:10px;">
      <button onclick="sendCustomText()" style="flex:1;">Отобразить</button>
      <button onclick="clearCustomText()" class="btn-danger" style="flex:1;">Очистить</button>
    </div>
  </div>

</div>
<script>
let alarmsData = [];
let lightState = false;
const dNames = ['Пн','Вт','Ср','Чт','Пт','Сб','Вс'];

function updateLightBtn() {
  const btn = document.getElementById('lightBtn');
  if(lightState) {
    btn.innerText = 'ВЫКЛЮЧИТЬ';
    btn.style.background = 'var(--danger)';
  } else {
    btn.innerText = 'ВКЛЮЧИТЬ';
    btn.style.background = 'var(--success)';
  }
}

function toggleLight() {
  fetch('/api/light', {method:'POST'}).then(r=>r.json()).then(d=>{
    lightState = d.state;
    updateLightBtn();
  });
}

function toggleTrains(state) {
  fetch('/api/trains-config', {
    method:'POST',
    headers:{'Content-Type':'application/json'},
    body:JSON.stringify({enabled: state})
  });
}

function queryTrainsByTime() {
  const t = document.getElementById('queryTime').value;
  if(!t) return alert('Выберите время!');
  fetch('/api/trains-query', {
    method:'POST',
    headers:{'Content-Type':'application/json'},
    body:JSON.stringify({time: t})
  }).then(()=>alert('Расписание на ' + t + ' выведено на экран!'));
}

function showTrainsNow() {
  fetch('/api/trains-show', {
    method:'POST',
    headers:{'Content-Type':'application/json'},
    body:JSON.stringify({show: true})
  }).then(()=>alert('Расписание поездов выведено на экран!'));
}

function hideTrainsNow() {
  fetch('/api/trains-show', {
    method:'POST',
    headers:{'Content-Type':'application/json'},
    body:JSON.stringify({show: false})
  });
}

function sendCustomText() {
  const txt = document.getElementById('customText').value;
  fetch('/api/message', {
    method:'POST',
    headers:{'Content-Type':'application/json'},
    body:JSON.stringify({text: txt, active: true})
  }).then(()=>alert('Сообщение отправлено на экран!'));
}

function clearCustomText() {
  document.getElementById('customText').value = '';
  fetch('/api/message', {
    method:'POST',
    headers:{'Content-Type':'application/json'},
    body:JSON.stringify({text: '', active: false})
  });
}

function toggleAutoBright(isAuto) {
  document.getElementById('brightSlider').disabled = isAuto;
  fetch('/api/brightness', {
    method:'POST',
    headers:{'Content-Type':'application/json'},
    body:JSON.stringify({auto: isAuto, val: parseInt(document.getElementById('brightSlider').value)})
  });
}

function setBrightness(val) {
  fetch('/api/brightness', {
    method:'POST',
    headers:{'Content-Type':'application/json'},
    body:JSON.stringify({auto: document.getElementById('autoBrightToggle').checked, val: parseInt(val)})
  });
}

function load(){
  fetch('/api/status').then(r=>r.json()).then(d=>{
    document.getElementById('devTime').innerText = `Часы: ${d.time}`;
    lightState = d.light;
    updateLightBtn();
    document.getElementById('trainsToggle').checked = d.trainsEnabled;
    document.getElementById('autoBrightToggle').checked = !d.isManualBright;
    document.getElementById('brightSlider').value = d.bright;
    document.getElementById('brightSlider').disabled = !d.isManualBright;
    alarmsData = d.alarms; renderAlarms();
  });
}

function renderAlarms(){
  const html = alarmsData.map((a, i) => `
    <div class="alarm-item">
      <div style="flex-grow:1; min-width:180px;">
        <div style="display:flex;align-items:center;gap:15px;">
          <div class="alarm-time">${String(a.h).padStart(2,'0')}:${String(a.m).padStart(2,'0')}</div>
          <div style="color:#aaa;font-size:0.9rem;">Мелодия ${a.melody}</div>
        </div>
        <div class="days-picker">
          ${a.days.map((d, j) => `<div class="day-btn ${d?'active':''}" onclick="tDay(${i},${j})">${dNames[j]}</div>`).join('')}
        </div>
      </div>
      <div style="display:flex;flex-direction:column;gap:12px;align-items:flex-end;">
        <div style="display:flex;gap:15px;align-items:center;">
          <label class="sunrise-label" title="Рассвет">☀️ <input type="checkbox" ${a.sunrise?'checked':''} onchange="toggleSunrise(${i}, this.checked)"></label>
          <input type="checkbox" class="toggle" ${a.active?'checked':''} onchange="toggle(${i}, this.checked)">
        </div>
        <div style="display:flex;gap:5px;">
          <button class="btn-warn" style="padding:6px 12px;font-size:0.85rem;" onclick="edit(${i})">Изменить</button>
          <button class="btn-danger" style="padding:6px 12px;font-size:0.85rem;" onclick="del(${i})">X</button>
        </div>
      </div>
    </div>
  `).join('');
  document.getElementById('alarmsList').innerHTML = html || "<p style='color:#777'>Нет будильников</p>";
}
function addAlarm(){
  if(alarmsData.length >= 5) return alert('Максимум 5 будильников');
  let t = prompt("Введите время (ЧЧ:ММ):", "07:00");
  if(!t) return;
  let m = prompt("Номер мелодии (1-5):", "1");
  let parts = t.split(':');
  alarmsData.push({active:true, h:parseInt(parts[0]), m:parseInt(parts[1]), melody:parseInt(m)||1, sunrise:true, days:[true,true,true,true,true,false,false]});
  save();
}
function edit(i){
  let t = prompt("Изменить время (ЧЧ:ММ):", String(alarmsData[i].h).padStart(2,'0') + ":" + String(alarmsData[i].m).padStart(2,'0'));
  if(!t) return;
  let m = prompt("Изменить номер мелодии (1-5):", alarmsData[i].melody);
  let parts = t.split(':');
  alarmsData[i].h = parseInt(parts[0]);
  alarmsData[i].m = parseInt(parts[1]);
  alarmsData[i].melody = parseInt(m)||1;
  save();
}
function del(i){ alarmsData.splice(i, 1); save(); }
function toggle(i, state){ alarmsData[i].active = state; save(); }
function toggleSunrise(i, state){ alarmsData[i].sunrise = state; save(); }
function tDay(i, dIndex){ alarmsData[i].days[dIndex] = !alarmsData[i].days[dIndex]; save(); }
function save(){
  fetch('/api/alarms',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(alarmsData)}).then(load);
}
function syncTime(){
  const d = new Date();
  const payload = {y:d.getFullYear(), mo:d.getMonth()+1, d:d.getDate(), h:d.getHours(), m:d.getMinutes(), s:d.getSeconds()};
  fetch('/api/time',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(payload)})
  .then(()=>alert('Время синхронизировано!')).then(load);
}
setInterval(load, 5000); load();
</script></body></html>
)rawliteral";