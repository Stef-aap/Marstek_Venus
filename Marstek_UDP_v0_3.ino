//****************************************************************************
//
// HomeAssistant part is not tested
// Belangrijk: webpage/api/status moet exact deze velden bevatten, 
// anders gaat Javascript in de fout en wordt er nooit geupdate
//
// * Features:
// * - WiFi verbinding met Marstek via UDP
// * - MQTT publicatie voor Home Assistant
// * - Web interface (http://ESP32-IP-ADRES)
// * - Alarm notificaties
//****************************************************************************
// ESP32C3
//  - https://www.studiopieters.nl/esp32-c3-super-mini-pinout/
//  - vergeet in Arduio niet "USB CDC on boot" te enablen
//
// Marstek Venus Communicatie
//  - 17-12-2025, firmware geupdate
//       - Venus E v3:  v135 --> v144,
//       - CT003     :  versie 116 --> 122
//  
//  - OPEN-API: https://static-eu.marstekenergy.com/ems/resource/agreement/MarstekDeviceOpenApi.pdf
// ****************************************************************************
// ToDo
//  - add watchdog
//  - favicon, wel aangemaakt, maar lijkt niet te werken
//  - WebPage in een gewone file stoppen
//
// Version 0.3, 01-01-2026, SM
//  - MQTT in aparte file
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
String      Mac_Address = "" ;

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
// ==================== GLOBALS ====================
// ****************************************************************************
int messageId = 0;
unsigned long lastEnergyUpdate = 0;

struct BatteryData {
  int           soc           = 0;
  float         batCapacity   = 0;
  float         ratedCapacity = 0;
  float         batTemp       = 0;
  int           batCurrent    = 0;
//  int           batVoltage    = 0;
  float         batVoltage    = 0;
  unsigned long lastUpdate    = 0;
} batteryData;

struct AccuData {
  int   Mode             = 0 ;  // 1=AI , 2=Manual, 3=UPS
  int   P1_Meter         = 0 ;
  
  int   Power_A          = 0 ;
  int   Power_B          = 0 ;
  int   Power_C          = 0 ;
  int   OnGrid_Power     = 0 ;
  int   OffGrid_Power    = 0 ;
  
  int   Bat_SOC          = 0 ;
  int   Bat_Capacity     = 0 ;
  float Bat_Temperature  = 0.0 ;

  int   Energy_In        = 0 ;
  int   Energy_Out       = 0 ;
  int   Total_Energy_In  = 0 ;  
  int   Total_Energy_Out = 0 ;  

  int gridPower = 0;
  int backupPower = 0;
  int batPower = 0;
  int status = 0;
  unsigned long lastUpdate = 0;   // MAG DEZE WEG ??
} AccuData;

// ****************************************************************************
//#include "Marstek_Alarms.h"
// ****************************************************************************

// ****************************************************************************
// ****************************************************************************
#include <PubSubClient.h>
PubSubClient mqttClient ( espClient ) ;
#include "Marstek_MQTT.h"
// ****************************************************************************


// ****************************************************************************
// ****************************************************************************
WiFiUDP      udp ;
#include "Marstek_UDP.h"
// ****************************************************************************


// ****************************************************************************
// ****************************************************************************
#include <WebServer.h>
WebServer WEBServer ( 80 ) ;
#include "Marstek_WebServer_2.h"
// ****************************************************************************


// ****************************************************************************
void setup() {
// ****************************************************************************
  Serial.begin ( 115200 ) ;
  delay ( 1000 ) ;
  
  Serial.println ( "\n========================================" ) ;
  Serial.println ( "  Marstek Venus E v3 Monitor Pro");
  Serial.println ( "========================================\n" ) ;
  
  //connectWiFi();
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
  
  if ( strlen ( mqtt_server ) > 0 ) {
    mqttClient.setServer ( mqtt_server, mqtt_port ) ;
    connectMQTT () ;
  }

  mqttClient.setBufferSize ( 1000 ) ;
 
  My_WebServer_Setup () ;
  
  configTime ( 3600, 3600, "pool.ntp.org" ) ;
  // Configure NTP
  //configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  //Serial.println("NTP time configured.");

  updateAllData ( 0 ) ;
}

// ****************************************************************************
unsigned long Last         = millis() ;
uint16_t      Loop_Counter = 0 ;
void loop() {
// ****************************************************************************
  if ( WiFi.status() != WL_CONNECTED ) {
    ESP.restart () ;
  }
  
  if ( strlen(mqtt_server) > 0 && !mqttClient.connected() ) {
    connectMQTT () ;
  }
  mqttClient.loop () ;
  
  My_WebServer_Loop () ;

  if ( millis() - Last > 30000 ) {
    Serial.println ( "Update Data" ) ;
    Last = millis () ;
    
    updateAllData ( Loop_Counter ) ;
    
    Publish_2_MQTT () ;


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

  /*
  static int lastDay = -1 ;
  struct tm timeinfo;
  if ( getLocalTime ( &timeinfo ) ) {
    if ( timeinfo.tm_mday != lastDay ) {
      lastDay = timeinfo.tm_mday ;
      if ( timeinfo.tm_hour == 0 && timeinfo.tm_min < 5 ) {
        //resetDailyStats () ;
      }
    }
  }
 */
  //delay ( 100 ) ;
}

