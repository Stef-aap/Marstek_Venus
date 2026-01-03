#ifndef Marstek_WebServer_2_h
#define Marstek_WebServer_2_h

#include <ArduinoJson.h>     // v7.x


//****************************************************************************
void handleFavicon() {
//****************************************************************************
  Serial.println ( "WebServer Favicon" ) ;
// 'logo', 16x16px, gemaakt vanuit een groter plaatje met https://javl.github.io/image2cpp/
const unsigned char favicon_ico [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x20, 0x04, 0x31, 0x8c, 0x19, 0x98, 0x1b, 0xd8, 0x01, 0x80, 0x00, 0x00, 
	0x06, 0x60, 0x06, 0x60, 0x00, 0x00, 0x0c, 0x30, 0x06, 0x60, 0x03, 0xc0, 0x00, 0x00, 0x00, 0x00
};
  WEBServer.send_P(200, "image/x-icon", (const char*)favicon_ico, sizeof(favicon_ico));
}


//****************************************************************************
void handleRoot() {
  Serial.println ( "WebServer RootPage" ) ;
//****************************************************************************
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="nl">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <link rel="icon" type="image/x-icon" href="favicon.ico">
  <!-- <link rel='icon' href=\"data:image/svg+xml,<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 100 100'> <text y='75' font-size='80'>⚡</text></svg>> -->
  <title>Marstek Venus Monitor</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      min-height: 100vh;
      padding: 20px;
      color: #333;
    }
    .container {
      max-width: 1200px;
      margin: 0 auto;
    }
    h1 {
      color: white;
      text-align: center;
      margin-bottom: 30px;
      font-size: 2.5em;
      text-shadow: 2px 2px 4px rgba(0,0,0,0.2);
    }
    .alerts {
      margin-bottom: 20px;
    }
    .alert {
      background: #ff5252;
      color: white;
      padding: 15px;
      border-radius: 10px;
      margin: 10px 0;
      font-weight: bold;
      text-align: center;
      animation: pulse 2s infinite;
      box-shadow: 0 5px 15px rgba(255,82,82,0.4);
    }
    @keyframes pulse {
      0%, 100% { opacity: 1; }
      50% { opacity: 0.8; }
    }
    .grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
      gap: 20px;
      margin-bottom: 20px;
    }
    .card {
      background: white;
      border-radius: 15px;
      padding: 25px;
      box-shadow: 0 10px 30px rgba(0,0,0,0.2);
      transition: all 0.3s ease;
    }
    .card:hover {
      transform: translateY(-5px);
      box-shadow: 0 15px 40px rgba(0,0,0,0.3);
    }
    .card h2 {
      color: #667eea;
      margin-bottom: 20px;
      font-size: 1.5em;
      display: flex;
      align-items: center;
      gap: 10px;
    }
    .metric {
      display: flex;
      justify-content: space-between;
      padding: 12px 0;
      border-bottom: 1px solid #f0f0f0;
    }
    .metric:last-child {
      border-bottom: none;
    }
    .metric-label {
      color: #666;
      font-weight: 500;
    }
    .metric-value {
      color: #333;
      font-weight: bold;
      font-size: 1.1em;
    }
    .soc-display {
      text-align: center;
      margin: 20px 0;
    }
    .soc-circle {
      width: 150px;
      height: 150px;
      border-radius: 50%;
      background: conic-gradient(#667eea var(--soc-angle), #e0e0e0 var(--soc-angle));
      display: flex;
      align-items: center;
      justify-content: center;
      margin: 0 auto 15px;
      box-shadow: 0 5px 15px rgba(0,0,0,0.1);
      transition: all 0.3s ease;
    }
    .soc-inner {
      width: 120px;
      height: 120px;
      border-radius: 50%;
      background: white;
      display: flex;
      align-items: center;
      justify-content: center;
      flex-direction: column;
    }
    .soc-value {
      font-size: 2.5em;
      font-weight: bold;
      color: #667eea;
    }
    .soc-label {
      font-size: 0.9em;
      color: #999;
    }
    .status-badge {
      display: inline-block;
      padding: 5px 15px;
      border-radius: 20px;
      font-size: 0.9em;
      font-weight: bold;
    }
    .status-charging {
      background: #4caf50;
      color: white;
    }
    .status-discharging {
      background: #ff9800;
      color: white;
    }
    .status-idle {
      background: #9e9e9e;
      color: white;
    }
    .power-indicator {
      display: flex;
      align-items: center;
      gap: 5px;
    }
    .power-arrow {
      font-size: 1.2em;
    }
    .refresh-btn {
      background: #667eea;
      color: white;
      border: none;
      padding: 15px 30px;
      border-radius: 10px;
      font-size: 1em;
      font-weight: bold;
      cursor: pointer;
      width: 100%;
      transition: all 0.3s ease;
      box-shadow: 0 5px 15px rgba(102, 126, 234, 0.3);
    }
    .refresh-btn:hover {
      background: #5568d3;
      transform: translateY(-2px);
      box-shadow: 0 7px 20px rgba(102, 126, 234, 0.4);
    }
    .refresh-btn:active {
      transform: translateY(0);
    }
    .last-update {
      text-align: center;
      color: white;
      margin-top: 20px;
      font-size: 0.9em;
      text-shadow: 1px 1px 2px rgba(0,0,0,0.2);
    }
    .loading {
      opacity: 0.6;
      pointer-events: none;
    }
    @media (max-width: 768px) {
      h1 {
        font-size: 1.8em;
      }
      .grid {
        grid-template-columns: 1fr;
      }
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>⚡MiRa  Marstek Venus E v3.0 Monitor</h1>
    
    <div class="alerts" id="alerts"></div>
    
    <div class="grid">
      <div class="card">
        <h2><span>🔋</span> Batterij Status</h2>
        <div class="soc-display">
          <div class="soc-circle" id="socCircle" style="--soc-angle: 0deg">
            <div class="soc-inner">
              <div class="soc-value" id="socValue">--</div>
              <div class="soc-label">%</div>
            </div>
          </div>
        </div>

        <div class="metric">
          <span class="metric-label">Capaciteit</span>
          <span class="metric-value power-indicator">
            <span class="metric-value" id="capacity">-- Wh</span>
            <span class="power-arrow" id="capacityArrow"></span>
          </span>
        </div>

        <div class="metric">
          <span class="metric-label">Mode</span>
          <span class="metric-value" id="mode">-- V</span>
        </div>

        <div class="metric">
          <span class="metric-label">Status</span>
          <span class="metric-value" id="batteryStatus">--</span>
        </div>
        
        <div class="metric">
          <span class="metric-label">Temperatuur</span>
          <span class="metric-value" id="temperature">-- °C</span>
        </div>

      </div>
      
      <div class="card">
        <h2><span>⚡</span> Energie Systeem</h2>
        
        <div class="metric">
          <span class="metric-label">Fase A</span>
          <span class="metric-value power-indicator">
            <span id="Power_A">-- W</span>
            <span class="power-arrow" id="gridArrow_A"></span>
          </span>
        </div>
        <div class="metric">
          <span class="metric-label">Fase B</span>
          <span class="metric-value power-indicator">
            <span id="Power_B">-- W</span>
            <span class="power-arrow" id="gridArrow_B"></span>
          </span>
        </div>
        <div class="metric">
          <span class="metric-label">Fase C</span>
          <span class="metric-value power-indicator">
            <span id="Power_C">-- W</span>
            <span class="power-arrow" id="gridArrow_C"></span>
          </span>
        </div>
        <div class="metric">
          <span class="metric-label">Totaal Afname</span>
          <span class="metric-value power-indicator">
            <span id="Power_Total">-- W</span>
            <span class="power-arrow" id="gridArrow_Total"></span>
          </span>
        </div>
        <div class="metric">
          <span class="metric-label">Batterij Vermogen</span>
          <span class="metric-value power-indicator">
            <span id="Power_OnGrid">-- W</span>
            <span class="power-arrow" id="gridArrow_OnGrid"></span>
          </span>
        </div>


      </div>
      
      <div class="card">
        <h2><span>📊</span> Statistiek</h2>
        <div class="metric">
          <span class="metric-label">Laden</span>
          <span class="metric-value" id="Energy_Total_In">-- kWh</span>
        </div>
        <div class="metric">
          <span class="metric-label">Ontladen</span>
          <span class="metric-value" id="Energy_Total_Out">-- kWh</span>
        </div>
        <div class="metric">
          <span class="metric-label">Rendement Last Week</span>
          <span class="metric-value" id="Rendement_LastWeek">-- %</span>
        </div>
        <div class="metric">
          <span class="metric-label">Rendement Totaal</span>
          <span class="metric-value" id="Rendement_Total">-- %</span>
        </div>
      </div>
    </div>
    
    <div class="card">
      <button class="refresh-btn" onclick="fetchData()">🔄 Ververs Data</button>
    </div>
    
    <div class="last-update" id="lastUpdate">Laatste update: --</div>
  </div>
  
  <script>
    function fetchData() {
      fetch('/api/status')
        .then(response => response.json())
        .then(data => {
          // Update SoC cirkel
          const soc = data.Accu.SOC;
          document.getElementById('socValue').textContent = soc;
          const socAngle = (soc / 100) * 360;
          document.getElementById('socCircle').style.setProperty('--soc-angle', socAngle + 'deg');
          
          // Update batterij data
          document.getElementById('capacity').textContent = data.Accu.Capacity.toFixed(1) + ' kWh';
          document.getElementById('temperature').textContent = data.Accu.Temperature.toFixed(1) + ' °C';
          document.getElementById('mode').textContent = data.Accu.Mode ;
          
          if (data.Accu.Status === 'Laden' ) {
            document.getElementById('capacityArrow').textContent = '⬆️';
          } else if (data.Accu.Status === 'Ontladen' ) {
            document.getElementById('capacityArrow').textContent = '⬇️';
          } else {
            document.getElementById('capacityArrow').textContent = '';
          }


          // Update batterij status met badge
          const statusEl = document.getElementById('batteryStatus');
          let statusClass = 'status-idle';
          if      (data.Accu.Status === 'Laden'   ) statusClass = 'status-charging';
          else if (data.Accu.Status === 'Ontladen') statusClass = 'status-discharging';
          statusEl.innerHTML = '<span class="status-badge ' + statusClass + '">' + data.Accu.Status + '</span>';



          // Update energie data met pijlen
          document.getElementById('Power_A').textContent = data.Accu.Power_A + ' W';
          document.getElementById('gridArrow_A').textContent = data.Accu.Power_A > 0 ? '⬅️' : '➡️';
          document.getElementById('Power_B').textContent = data.Accu.Power_B + ' W';
          document.getElementById('gridArrow_B').textContent = data.Accu.Power_B > 0 ? '⬅️' : '➡️';
          document.getElementById('Power_C').textContent = data.Accu.Power_C + ' W';
          document.getElementById('gridArrow_C').textContent = data.Accu.Power_C > 0 ? '⬅️' : '➡️';
          document.getElementById('Power_Total').textContent = data.Accu.Power_Total + ' W';
          document.getElementById('gridArrow_Total').textContent = data.Accu.Power_Total > 0 ? '⬅️' : '➡️';
          
          document.getElementById('Power_OnGrid').textContent = data.Accu.Power_OnGrid + ' W';
          <!-- document.getElementById('gridArrow_OnGrid').textContent = data.Accu.Power_OnGrid > 0 ? '⬅️' : '➡️'; -->
          document.getElementById('gridArrow_OnGrid').textContent = ''; 
          if      (data.Accu.Status === 'Laden'   ) document.getElementById('gridArrow_OnGrid').textContent = '⬅️';
          else if (data.Accu.Status === 'Ontladen') document.getElementById('gridArrow_OnGrid').textContent = '➡️'; 
          


          // Update historische data
          document.getElementById('Energy_Total_In'   ).textContent = data.Accu.Energy_Total_In  + ' kWh';
          document.getElementById('Energy_Total_Out'  ).textContent = data.Accu.Energy_Total_Out + ' kWh';
          document.getElementById('Rendement_LastWeek').textContent = data.Accu.Rendement_LastWeek + ' %';
          document.getElementById('Rendement_Total'   ).textContent = data.Accu.Rendement_Total    + ' %';
          
          // Update alarmen
          let alertsHTML = '';
          if (data.alarms.critical_battery) {
            alertsHTML += '<div class="alert">⚠️ KRITIEK: Batterij niveau zeer laag! (' + soc + '%)</div>';
          } else if (data.alarms.low_battery) {
            alertsHTML += '<div class="alert">⚠️ Waarschuwing: Batterij niveau laag (' + soc + '%)</div>';
          }
          if (data.alarms.high_temperature) {
            alertsHTML += '<div class="alert">🌡️ Waarschuwing: Batterij temperatuur hoog (' + data.Accu.Temperature.toFixed(1) + '°C)</div>';
          }
          document.getElementById('alerts').innerHTML = alertsHTML;
          
          // Update timestamp
          const now = new Date();
          document.getElementById('lastUpdate').textContent = 'Laatste update: ' + now.toLocaleTimeString('nl-NL');
        })
        .catch(error => {
          console.error('Fout bij ophalen data:', error);
          document.getElementById('lastUpdate').textContent = 'Fout bij ophalen data';
        });
    }
    
    // Haal data op bij laden
    fetchData();
    
    // Auto-refresh elke 5 seconden
    setInterval(fetchData, 5000);
  </script>
</body>
</html>
)rawliteral";
  
  WEBServer.send(200, "text/html", html);
}
/*
          <!--
          if (data.Accu.Power_OnGrid > 2 ) {
            document.getElementById('capacityArrow').textContent = '⬆️';
          } else if (data.Accu.Power_OnGrid < -2 ) {
            document.getElementById('capacityArrow').textContent = '⬇️';
          } else {
            document.getElementById('capacityArrow').textContent = '';
          } -->

*/
//****************************************************************************
void handleAPIStatus() {
// PAS OP: Velden moet allemaal aanwezig zijn, anders gaat Javascript in de fout en wordt er nooit geupdate
//****************************************************************************
  #ifdef Debug
    Serial.println ( "WebServer api/status" ) ;
  #endif
  JsonDocument doc;

  doc [ "Accu" ] [ "SOC"         ] = AccuData.Bat_SOC ;
  doc [ "Accu" ] [ "Temperature" ] = AccuData.Bat_Temperature ;
  if ( AccuData.Bat_Capacity != 0 ) {
       doc [ "Accu" ] [ "Capacity"    ] = AccuData.Bat_Capacity / 1000.0 ; }
  else doc [ "Accu" ] [ "Capacity"    ] = AccuData.Bat_SOC * 5.12 / 100 ;
  
  if      ( AccuData.Mode == 1 ) doc ["Accu"] ["Mode"] = "AI"        ;
  else if ( AccuData.Mode == 2 ) doc ["Accu"] ["Mode"] = "Handmatig" ;
  else if ( AccuData.Mode == 3 ) doc ["Accu"] ["Mode"] = "UPS"       ;
  else                           doc ["Accu"] ["Mode"] = "Unknown"   ;

  if      ( AccuData.OnGrid_Power == 0 ) doc ["Accu"] ["Status"] = "Stand By" ;
  else if ( AccuData.OnGrid_Power  > 0 ) doc ["Accu"] ["Status"] = "Ontladen" ;
  else                                   doc ["Accu"] ["Status"] = "Laden"    ;

  doc [ "Accu" ] [ "Power_A"      ] = AccuData.Power_A ;
  doc [ "Accu" ] [ "Power_B"      ] = AccuData.Power_B ;
  doc [ "Accu" ] [ "Power_C"      ] = AccuData.Power_C ;
  doc [ "Accu" ] [ "Power_Total"  ] = AccuData.Power_A + AccuData.Power_B + AccuData.Power_C ;
  doc [ "Accu" ] [ "Power_OnGrid" ] = AccuData.OnGrid_Power ;

  int Rendement       = -1 ;
  if ( AccuData.Energy_Out > 0 ) {
    Rendement       = round ( 100.0 * AccuData.Energy_Out / AccuData.Energy_In ) ;
  }
  int Rendement_Total = -1 ;
  if ( AccuData.Total_Energy_Out > 0 ) {
    Rendement_Total = round ( 100.0 * AccuData.Total_Energy_Out / AccuData.Total_Energy_In ) ;
  }

  doc [ "Accu" ] [ "Rendement_LastWeek" ] = Rendement       ;
  doc [ "Accu" ] [ "Rendement_Total"    ] = Rendement_Total ;
  doc [ "Accu" ] [ "Energy_Total_In"    ] = round ( AccuData.Total_Energy_In  / 1000.0 ) ;
  doc [ "Accu" ] [ "Energy_Total_Out"   ] = round ( AccuData.Total_Energy_Out / 1000.0 );


  #ifdef Marstek_Alarms_h
    doc [ "alarms" ] [ "low_battery"     ] = alarmLowBattery ;
    doc [ "alarms" ] [ "critical_battery"] = alarmCriticalBattery ;
    doc [ "alarms" ] [ "high_temperature"] = alarmHighTemp ;
  #else 
    doc [ "alarms" ] [ "low_battery"     ] = false ;
    doc [ "alarms" ] [ "critical_battery"] = false ;
    doc [ "alarms" ] [ "high_temperature"] = false ;
  #endif


  String json ;
  serializeJson ( doc, json ) ;
  WEBServer.send ( 200, "application/json", json ) ;
}


//****************************************************************************
void handleAPIReset() {
//****************************************************************************
  Serial.println ( "WebServer api/reset" ) ;
  // voor iets anders gaan gebruiken
  //resetDailyStats();
  WEBServer.send ( 200, "text/plain", "OK - Dagelijkse statistieken gereset" ) ;
}

//****************************************************************************
void handleNotFound() {
//****************************************************************************
  Serial.println ( "WebServer NOT Found" ) ;
  WEBServer.send(404, "text/plain", "404: Pagina niet gevonden");
}

// ****************************************************************************
void My_WebServer_Setup () {
// ****************************************************************************
  WEBServer.on ( "/"           , handleRoot       ) ;
  WEBServer.on ( "/api/status" , handleAPIStatus  ) ;
  WEBServer.on ( "/api/reset"  , handleAPIReset   ) ;
  WEBServer.on ( "/favicon.ico", handleFavicon    ) ;
  WEBServer.onNotFound ( handleNotFound ) ;
  WEBServer.begin () ;
  Serial.printf ( "✓ Webserver: http://%s\n", WiFi.localIP().toString().c_str() ) ;
}

// ****************************************************************************
void  My_WebServer_Loop () {
// ****************************************************************************
  WEBServer.handleClient();
}

#endif