#ifndef Marstek_MQTT_h
#define Marstek_MQTT_h

#include <ArduinoJson.h>     // v7.x

//compile error
//#include <PubSubClient.h>
//PubSubClient mqttClient ( espClient ) ;

int     MQTT_Fail_Count     = 0 ;
#define MQTT_Fail_Count_Max   3  

// ****************************************************************************
void Publish_2_MQTT_Actual () {
// ****************************************************************************
    if ( ! mqttClient.connected() ) return ;
    JsonDocument Doc ;
    String       Topic ;
    String       Payload ;

    // Add struct fields to JSON
    Doc [ "Mode"             ] = AccuData.Mode             ;
    Doc [ "Bat_SOC"          ] = AccuData.Bat_SOC          ;
    Doc [ "Power_A"          ] = AccuData.Power_A          ;
    Doc [ "Power_B"          ] = AccuData.Power_B          ;
    Doc [ "Power_C"          ] = AccuData.Power_C          ;
    Doc [ "OnGrid_Power"     ] = AccuData.OnGrid_Power     ;
    Doc [ "OffGrid_Power"    ] = AccuData.OffGrid_Power    ;
    Doc [ "Bat_Temperature"  ] = AccuData.Bat_Temperature  ;
    Doc [ "Bat_Lading"       ] = AccuData.Bat_Lading     ;
    Doc [ "Energy_In"        ] = AccuData.Energy_In        ;
    Doc [ "Energy_Out"       ] = AccuData.Energy_Out       ;
    Doc [ "Total_Energy_In"  ] = AccuData.Total_Energy_In  ;
    Doc [ "Total_Energy_Out" ] = AccuData.Total_Energy_Out ;
    Doc [ "P1_Meter"         ] = AccuData.P1_Meter         ;

    serializeJson ( Doc, Payload ) ;

    String Temp = MAC_Address ;
    Temp.replace ( ":", "" ) ;
    Temp.toUpperCase();
    Topic = String ( MQTT_Topic_Prefix ) + "/" + Temp.substring(6) + "_Actual" ;
    Serial.print   ( "Send MQTT: " ) ;
    Serial.println ( Topic   ) ;
    //Serial.println ( Payload ) ;

    mqttClient.publish ( Topic.c_str(), Payload.c_str() ) ; //, true ) ; 3e parameter ??
}

// ****************************************************************************
#ifdef HomeAssistant
  void HA_publishToMQTT() {
// ****************************************************************************
    if (!mqttClient.connected()) return;
    
    char topic[100];
    char payload[50];
    
    snprintf(topic, sizeof(topic), "%s/battery/soc", mqtt_HA_prefix);
    snprintf(payload, sizeof(payload), "%d", batteryData.soc);
    mqttClient.publish(topic, payload, true);
    
    snprintf(topic, sizeof(topic), "%s/battery/capacity", mqtt_HA_prefix);
    snprintf(payload, sizeof(payload), "%.1f", batteryData.batCapacity);
    mqttClient.publish(topic, payload, true);
    
    snprintf(topic, sizeof(topic), "%s/battery/temperature", mqtt_HA_prefix);
    snprintf(payload, sizeof(payload), "%.1f", batteryData.batTemp);
    mqttClient.publish(topic, payload, true);
    
    snprintf(topic, sizeof(topic), "%s/battery/voltage", mqtt_HA_prefix);
    //snprintf(payload, sizeof(payload), "%.2f", batteryData.batVoltage / 1000.0);
    snprintf(payload, sizeof(payload), "%.2f", batteryData.batVoltage ) ;
    mqttClient.publish(topic, payload, true);
    
    snprintf(topic, sizeof(topic), "%s/battery/current", mqtt_HA_prefix);
    snprintf(payload, sizeof(payload), "%.2f", batteryData.batCurrent / 1000.0);
    mqttClient.publish(topic, payload, true);
    
    snprintf(topic, sizeof(topic), "%s/energy/grid_power", mqtt_HA_prefix);
    snprintf(payload, sizeof(payload), "%d", AccuData.gridPower);
    mqttClient.publish(topic, payload, true);
    
    snprintf(topic, sizeof(topic), "%s/energy/backup_power", mqtt_HA_prefix);
    snprintf(payload, sizeof(payload), "%d", AccuData.backupPower);
    mqttClient.publish(topic, payload, true);
    
    snprintf(topic, sizeof(topic), "%s/energy/battery_power", mqtt_HA_prefix);
    snprintf(payload, sizeof(payload), "%d", AccuData.batPower);
    mqttClient.publish(topic, payload, true);
    
    const char* batteryStatus;
    if (AccuData.batPower > 50) batteryStatus = "discharging";
    else if (AccuData.batPower < -50) batteryStatus = "charging";
    else batteryStatus = "idle";
    
    snprintf(topic, sizeof(topic), "%s/battery/status", mqtt_HA_prefix);
    mqttClient.publish(topic, batteryStatus, true);
    
   
    /*
    snprintf(topic, sizeof(topic), "%s/history/energy_charged", mqtt_HA_prefix);
    snprintf(payload, sizeof(payload), "%.2f", historicalData.energyChargedToday / 1000.0);
    mqttClient.publish(topic, payload, true);
    
    snprintf(topic, sizeof(topic), "%s/history/energy_discharged", mqtt_HA_prefix);
    snprintf(payload, sizeof(payload), "%.2f", historicalData.energyDischargedToday / 1000.0);
    mqttClient.publish(topic, payload, true);
    */
  }
#endif


// ****************************************************************************
#ifdef HomeAssistant
  void publishHomeAssistantDiscovery () {
// ****************************************************************************
    if ( ! mqttClient.connected() ) return ;
    JsonDocument doc ;
    char topic   [ 150 ] ;
    char payload [ 800 ] ;
    
    // Helper functie voor device config
    auto addDevice = [ &doc ] () {
      doc [ "device" ] ["identifiers"][0] = "marstek_venus_e";
      doc [ "device" ] ["name"] = "Marstek Venus E v3";
      doc [ "device" ] ["manufacturer"] = "Marstek";
      doc [ "device" ] ["model"] = "Venus E v3";
    };
    
    // Battery SoC
    doc.clear();
    snprintf(topic, sizeof(topic), "homeassistant/sensor/marstek/soc/config");
    doc["name"] = "Marstek Battery Level";
    doc["state_topic"] = String(mqtt_HA_prefix) + "/battery/soc";
    doc["unit_of_measurement"] = "%";
    doc["device_class"] = "battery";
    doc["unique_id"] = "marstek_soc";
    addDevice();
    serializeJson(doc, payload);
    mqttClient.publish(topic, payload, true);
    
    // Temperature
    doc.clear();
    snprintf(topic, sizeof(topic), "homeassistant/sensor/marstek/temperature/config");
    doc["name"] = "Marstek Temperature";
    doc["state_topic"] = String(mqtt_HA_prefix) + "/battery/temperature";
    doc["unit_of_measurement"] = "°C";
    doc["device_class"] = "temperature";
    doc["unique_id"] = "marstek_temp";
    addDevice();
    serializeJson(doc, payload);
    mqttClient.publish(topic, payload, true);
    
    // Battery Power
    doc.clear();
    snprintf(topic, sizeof(topic), "homeassistant/sensor/marstek/bat_power/config");
    doc["name"] = "Marstek Battery Power";
    doc["state_topic"] = String(mqtt_HA_prefix) + "/energy/battery_power";
    doc["unit_of_measurement"] = "W";
    doc["device_class"] = "power";
    doc["unique_id"] = "marstek_bat_power";
    addDevice();
    serializeJson(doc, payload);
    mqttClient.publish(topic, payload, true);
    
    // Grid Power
    doc.clear();
    snprintf(topic, sizeof(topic), "homeassistant/sensor/marstek/grid_power/config");
    doc["name"] = "Marstek Grid Power";
    doc["state_topic"] = String(mqtt_HA_prefix) + "/energy/grid_power";
    doc["unit_of_measurement"] = "W";
    doc["device_class"] = "power";
    doc["unique_id"] = "marstek_grid_power";
    addDevice();
    serializeJson(doc, payload);
    mqttClient.publish(topic, payload, true);
    
    Serial.println("✓ Home Assistant discovery configs verzonden");
  }
#endif

// ****************************************************************************
void MQTT_Connect () {
// ****************************************************************************
  String Client_ID = "ESP32-" + WiFi.macAddress();
  bool Connected;
  
  int Attempts = 0;
  Serial.print ( "MQTT" ) ;
  while ( ! mqttClient.connected() && ( Attempts < 3 ) ) {
    Serial.print ( "." ) ;
    Connected = mqttClient.connect ( Client_ID.c_str(), mqtt_user, mqtt_password ) ;
    if ( Connected ) {
      Serial.println ( "OK" );
      MQTT_Fail_Count = 0; // Reset teller bij succes
    } 
    else {
      Attempts++;
      if (Attempts < 3) delay ( 2000 ) ;
    }
  }
  
  if ( Connected ) {
    Serial.printf ( "✓ MQTT verbonden!  %s\n", mqtt_server ) ;

    #ifdef HomeAssistant
      publishHomeAssistantDiscovery () ;
    #endif
  } 
  else {
    MQTT_Fail_Count++ ;
    Serial.printf ( "✗ MQTT naar %s mislukt (code: %d)   (Count: %i)\n", mqtt_server, mqttClient.state(), MQTT_Fail_Count ) ;
    
    if ( MQTT_Fail_Count >= MQTT_Fail_Count_Max ) {
      Serial.println ( "MQTT failures >= maxRetries, RESTARTING ESP..." ) ;
      delay(1000);
      ESP.restart();
    }
  }
}

// ****************************************************************************
void MQTT_Setup () {
// ****************************************************************************
  if ( mqtt_server == "" ) return ;
  mqttClient.setServer ( mqtt_server, mqtt_port ) ;
  mqttClient.setBufferSize ( 1000 ) ;
} 

// ****************************************************************************
void MQTT_Loop () {
// ****************************************************************************
  if ( mqtt_server == "" ) return ;
  if ( ! mqttClient.connected () ) {
    MQTT_Connect () ;
  }
  mqttClient.loop () ;
}

#endif