// ****************************************************************************
// ESP-32 Communication with the Marstek Venus E v3 through BLE
// * - WiFi verbinding met Marstek via UDP
// * - All parameters send within one MQTT message
// * - MQTT publicatie voor Home Assistant
// * - Single parameter MQTT messages for Home Assistant
// * - Web interface (http://ESP32-IP-ADRES)
// * - Alarm notificaties
//
// ToDo
//  - add watchdog
//  - favicon, wel aangemaakt, maar lijkt niet te werken
//  - WebPage in een gewone file stoppen
//  - werkt om onduidelijke redenen niet op een ESP32C3 ????
//
// Important
//   - HomeAssistant part is not tested
//   - Belangrijk: webpage/api/status moet exact deze velden bevatten, 
//       anders gaat Javascript in de fout en wordt er nooit geupdate
//****************************************************************************
// Marstek Venus Communicatie
//  - 17-12-2025, firmware geupdate
//       - Venus E v3:  versie v135 --> v144
//       - CT003     :  versie v116 --> v122
//  
//  - OPEN-API: https://static-eu.marstekenergy.com/ems/resource/agreement/MarstekDeviceOpenApi.pdf

// ****************************************************************************
// Version 0.4, 04-02-2026, SM
//   - individual cell voltages read
//  - Data, MQTT, WebPage, Alarms in aparte file, shared with Marstek_BLE
//  - WiFi.setHostname toegevoegd, maar de DHCP server detecteert dat niet (direct?)
//
// Version 0.3, 31-12-2025, SM
//   - poging udp
//
// Version 0.2, 31-12-2025, SM
//   - WebServer toegevoegd (in aparte file), werkt goed
//   - TODO vras naar webiste overzetten eb verder uitdecoderen
//
// Version 0.1, 21-12-2025, SM
//   - orginal version based on Marstek_Claude2_BLE_v15
// ****************************************************************************
// ESP32
// ESP32C3 doet het soms helemaal niet goed
// compile as huge app 
// Sketch uses 1583724 bytes (50%) of program storage space. Maximum is 3145728 bytes.
// Global variables use 58080 bytes (17%) of dynamic memory, leaving 269600 bytes for local variables. Maximum is 327680 bytes.
// ****************************************************************************
#define _Main_Name          "Marstek_Monitor_BLE"
#define _Main_Version       0.4
//??#define WDT_TIMEOUT_SECONDS 30  // PAS OP, als te kort, werkt OTA niet !!!
//#define Debug     // print extra informatie op de seriele poort

#include <BLEDevice.h>
#include <WiFi.h>
#include <PubSubClient.h>

// WiFi (alleen voor webserver en MQTT)
#include <Wifi_Settings.h> 
const char* ssid     = __SECRET_Wifi_Name ;
const char* password = __SECRET_Wifi_PWD  ;

// MQTT (optioneel - laat leeg als niet gebruikt)
const char* mqtt_server       = __SECRET_Broker_IP ;   // empty string if no MQTT needed
const char* mqtt_user         = "" ;                   // Leeg laten als geen auth
const char* mqtt_password     = "" ;
const int   mqtt_port         = 1883;
const char* MQTT_Topic_Prefix = "marstek/venus";
const char* mqtt_HA_prefix    = "marstek/venus";

// BLE Device informatie
//const char* MARSTEK_BLE_NAME = "MST_VNSE3_63f9";
//const char* MARSTEK_BLE_NAME = "MST_VNSE3_e4e1";
//String MAC_Address = "00:9B:08:A5:E4:E1" ;  // MST_VNSE3_e4e1
String MAC_Address = "18:CE:DF:F5:63:F9" ;  // MST_VNSE3_63f9
static BLEAddress bleAddress  ( MAC_Address ) ;

WiFiClient espClient;
PubSubClient mqttClient ( espClient );

#include <WebServer.h>
WebServer WEBServer ( 80 ) ;

#include "Marstek_Data.h"
//#include "Marstek_Alarms.h"
#include "Marstek_WebServer_2.h"
#include "Marstek_MQTT.h"

#include "Marstek_BLE.h"

// ****************************************************************************
// ****************************************************************************
void setup() {
  Serial.begin ( 115200 ) ;
  Serial.println ( "\nMarstek Venus BLE->MQTT" ) ;
  
  WiFi.begin ( ssid, password ) ;
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 ) ;
    Serial.print ( "." ) ;
  }
  WiFi.setHostname ( _Main_Name ) ;
  Serial.print   ( "\nWifi Connected, IP address = " ) ;
  Serial.println ( WiFi.localIP() ) ;

  MQTT_Setup () ;

  BLEDevice::init ( "" ) ;

  My_WebServer_Setup ();
}


uint8_t       My_CMD       = 0 ;
unsigned long Last         = millis() ;
uint16_t      Loop_Counter = 0 ;
// ****************************************************************************
void loop() {
// ****************************************************************************
  if ( WiFi.status() != WL_CONNECTED ) {
    ESP.restart () ;
  }
  
  MQTT_Loop () ;

  My_WebServer_Loop ();

  if ( millis() - Last > 10000 ) {
    Serial.println ( "Update Data" ) ;
    Last = millis () ;
    
    //updateAllData ( Loop_Counter ) ;
    if ( ! Send_BLE_Command ( My_CMD ) ) {
      Serial.println ( "BLE Command Failed" ) ;
    }
    // loop through BLE_Device_Info, BLE_Runtime_Info, BLE_BMS_Data
    My_CMD += 1 ;
    if ( My_CMD == 3 ) My_CMD = 0 ;
    
    Print_Battery () ;

    Publish_2_MQTT_Actual () ;
    Do_Long_Line = ! Do_Long_Line ;

    //#ifdef HomeAssistant
    //  HA_publishToMQTT () ;
    //#endif
    
    #ifdef Marstek_Alarms_h
      checkAlarms () ;
    #endif

    // Finally, update Loop Counter
    Loop_Counter += 1 ;
  }
  //?delay(1000);
}
