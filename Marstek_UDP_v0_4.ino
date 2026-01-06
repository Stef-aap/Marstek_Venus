//****************************************************************************
// ESP-32 Communication with the Marstek Venus E v3 through wifi-UDP
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
//
// Important
//   - HomeAssistant part is not tested
//   - Belangrijk: webpage/api/status moet exact deze velden bevatten, 
//       anders gaat Javascript in de fout en wordt er nooit geupdate
//   - ESP32C3,  https://www.studiopieters.nl/esp32-c3-super-mini-pinout/
//       vergeet in Arduio niet "USB CDC on boot" te enablen
//****************************************************************************
// Marstek Venus Communicatie
//  - 17-12-2025, firmware geupdate
//       - Venus E v3:  versie v135 --> v144
//       - CT003     :  versie v116 --> v122
//  
//  - OPEN-API: https://static-eu.marstekenergy.com/ems/resource/agreement/MarstekDeviceOpenApi.pdf
// ****************************************************************************
// Version 0.4, 06-01-2026, SM
//  - zoveel identeik aan BLE versie gemaakt
//
// Version 0.4, 05-01-2026, SM
//  - Data, MQTT, WebPage, Alarms in aparte file, shared with Marstek_BLE
//  - WiFi.setHostname toegevoegd, maar de DHCP server detecteert dat niet (direct?)
//
// Version 0.2, 01-01-2026, SM
//  - getest op ESP32, ESP32C3
//  - Webserver in aparte file
//  - UDP in aparte file
//
// Version 0.1, 15-12-2025, SM
//    - initial version, met behulp van Claude.ai
//    - ?? NEEDS hardware reset to get correct on the home network ?
// ****************************************************************************
#define _Main_Name     "Marstek_Monitor"
#define _Main_Version   0.3
//#define Debug

// ****************************************************************************
// WiFi_Settings.h should contain your secrets
//  __SECRET_Wifi_Name, __SECRET_Wifi_PWD
//  __SECRET_Broker_IP (if you need MQTT)
// ****************************************************************************
#include <Wifi_Settings.h>
const char* ssid     = __SECRET_Wifi_Name ;
const char* password = __SECRET_Wifi_PWD ;

// Marstek Venus E
const char* marstekIP   = "192.168.0.21" ;  // you can find it with the BLE Test Tool | Network Info
const int   marstekPort = 30000 ;

// MQTT
//#define HomeAssistant   // als HomeAssistant Discovery gewenst, NIET GETEST
const char* mqtt_server       = __SECRET_Broker_IP ;   // empty string if no MQTT needed
const char* mqtt_user         = "" ;                   // Leeg laten als geen auth
const char* mqtt_password     = "" ;
const int   mqtt_port         = 1883;
const char* MQTT_Topic_Prefix = "marstek/venus";
const char* mqtt_HA_prefix    = "marstek/venus";

// ****************************************************************************
// ****************************************************************************
#include <WiFi.h>
WiFiClient   espClient ;
// ****************************************************************************

// ****************************************************************************
// Globals
// ****************************************************************************
String   MAC_Address = "" ;


// ****************************************************************************
// ****************************************************************************
#include <PubSubClient.h>
PubSubClient mqttClient ( espClient ) ;
// ****************************************************************************

// ****************************************************************************
// ****************************************************************************
#include <WebServer.h>
WebServer WEBServer ( 80 ) ;
// ****************************************************************************

#include "Marstek_Data.h"
//#include "Marstek_Alarms.h"
#include "Marstek_WebServer_2.h"
#include "Marstek_MQTT.h"

// ****************************************************************************
// ****************************************************************************
WiFiUDP      udp ;
#include "Marstek_UDP.h"
// ****************************************************************************


// ****************************************************************************
void setup() {
// ****************************************************************************
  Serial.begin ( 115200 ) ;
  delay ( 1000 ) ;
  
  Serial.println ( "\n========================================" ) ;
  Serial.println ( "  Marstek Venus E v3 UDP Monitor"           ) ;
  Serial.println ( "========================================\n" ) ;
  
  WiFi.begin ( ssid, password ) ;
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 ) ;
    Serial.print ( "." ) ;
  }
  WiFi.setHostname ( _Main_Name ) ;
  Serial.print   ( "\nWifi Connected, IP address = " ) ;
  Serial.println ( WiFi.localIP() ) ;
  
  udp.begin ( marstekPort ) ;
  Serial.printf("✓ UDP gestart op poort %d\n", marstekPort);
  
  MQTT_Setup () ;

  My_WebServer_Setup () ;
  
  configTime ( 3600, 3600, "pool.ntp.org" ) ;
  // Configure NTP
  //configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  //Serial.println("NTP time configured.");
}

// ****************************************************************************
unsigned long Last         = millis() - 100000 ;  // offset to get started
uint16_t      Loop_Counter = 0 ;
// ****************************************************************************
void loop() {
  if ( WiFi.status() != WL_CONNECTED ) {
    ESP.restart () ;
  }
  
  MQTT_Loop () ;
 
  My_WebServer_Loop () ;

  if ( millis() - Last > 30000 ) {
    Serial.println ( "Update Data" ) ;
    Last = millis () ;
    
    updateAllData ( Loop_Counter ) ;
    
    Publish_2_MQTT_Actual () ;

    #ifdef HomeAssistant
      if ( mqttClient.connected() ) {
        HA_publishToMQTT () ;
      }
    #endif
    
    #ifdef Marstek_Alarms_h
      checkAlarms () ;
    #endif

    // Finally, update Loop Counter
    Loop_Counter += 1 ;
  }
}