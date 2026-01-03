#ifndef Marstek_UDP_h
#define Marstek_UDP_h

#include <WiFiUdp.h>
#include <ArduinoJson.h>     // v7.x

// ****************************************************************************
String sendUDPCommand ( const char* method, const char* params ) {
// ****************************************************************************
  JsonDocument doc ;
  doc [ "id"     ] = messageId++ ;
  doc [ "method" ] = method ;
  
  if ( params != nullptr && strlen ( params ) > 0 ) {
    JsonDocument paramsDoc ;
    deserializeJson ( paramsDoc, params ) ;
    doc [ "params" ] = paramsDoc ;
  }
  
  String jsonString ;
  serializeJson ( doc, jsonString ) ;
  #ifdef Debug
    Serial.printf ( "SEND %s\n", jsonString.c_str() ) ;
  #endif

  udp.beginPacket ( marstekIP, marstekPort ) ;
  udp.print ( jsonString ) ;
  udp.endPacket () ;
  
  unsigned long startTime = millis() ;
  //while ( millis() - startTime < 20000 ) {
  while ( millis() - startTime < 1000 ) {
    int packetSize = udp.parsePacket () ;
    if ( packetSize ) {
      char incomingPacket [ 1024 ] ;
      int len = udp.read ( incomingPacket, sizeof ( incomingPacket ) - 1 ) ;
      if ( len > 0 ) {
        incomingPacket [ len ] = 0 ;
        String Line ;
        Line = String (  incomingPacket ) ;
        #ifdef Debug
          Serial.printf ( "XXResponse %s: %s\n", method, Line.c_str() ) ;
        #endif
        return Line ;
        //return String ( incomingPacket ) ;
      }
    }
    delay ( 10 ) ;
  }
  #ifdef Debug
    Serial.printf ( "--- NO Response %s\n", method ) ;
  #endif  
  return "";
}


// ****************************************************************************
void Print_Battery () {
// ****************************************************************************
  int Rendement ;

  if      ( AccuData.OnGrid_Power == 0 ) Serial.println ( "====  Stand By  ====" ) ;
  else if ( AccuData.OnGrid_Power  > 0 ) Serial.println ( "====  Ontladen  ====" ) ;
  else                                     Serial.println ( "====  Laden  ====" ) ;

  Serial.printf   ( "  Mode             = %i (1=AI, 2=Manual, 3=UPS)\n", AccuData.Mode     ) ;
  Serial.printf   ( "  P1_Meter         = %i\n", AccuData.P1_Meter ) ;
  
  Serial.printf   ( "  Power_A          = %4i W\n", AccuData.Power_A ) ;
  Serial.printf   ( "  Power_B          = %4i W\n", AccuData.Power_B ) ;
  Serial.printf   ( "  Power_C          = %4i W\n", AccuData.Power_C ) ;

  Serial.printf   ( "  OnGrid_Power     = %4i W\n" , AccuData.OnGrid_Power    ) ;
  Serial.printf   ( "  OffGrid_Power    = %4i W\n" , AccuData.OffGrid_Power   ) ;
  Serial.printf   ( "  Bat_SOC          = %4i %%\n", AccuData.Bat_SOC         ) ;
  Serial.printf   ( "  Bat_Capacity     = %4i Wh\n", AccuData.Bat_Capacity    ) ;
  Serial.printf   ( "  Bat_Temperature  =%5.1f °C\n", AccuData.Bat_Temperature ) ;

  Serial.printf   ( "  Energy_In        =%7.2f kWh\n", AccuData.Energy_In /1000000.0 ) ;
  Serial.printf   ( "  Energy_Out       =%7.2f kWh\n", AccuData.Energy_Out/1000000.0 ) ;
  Serial.printf   ( "  Total_Energy_In  =%7.2f kWh\n", AccuData.Total_Energy_In /1000.0 ) ;
  Serial.printf   ( "  Total_Energy_Out =%7.2f kWh\n", AccuData.Total_Energy_Out/1000.0 ) ;

  if ( AccuData.Energy_Out > 0 ) {
    Rendement = int ( 100.0 * AccuData.Energy_Out / AccuData.Energy_In ) ;
    Serial.printf   ( "  Rendement        = %i %%\n", Rendement ) ;
  }
  if ( AccuData.Total_Energy_Out > 0 ) {
    Rendement = int ( 100.0 * AccuData.Total_Energy_Out / AccuData.Total_Energy_In ) ;
    Serial.printf   ( "  Total Rendement  = %i %%\n", Rendement ) ;
  }
}  


// ****************************************************************************
float calculateLiFePO4Voltage ( int soc ) {
// ****************************************************************************
  // Realistische LiFePO4 spanning curve (16 cellen in serie)
  // Gebaseerd op typische discharge curve
  if ( soc >= 95 ) {
    return 54.4 + ( soc - 95 ) * 0.32 ;    // 54.4V - 56.0V
  } else if ( soc >= 90 ) {
    return 53.6 + ( soc - 90 ) * 0.16 ;    // 53.6V - 54.4V
  } else if ( soc >= 20 ) {
    return 51.2 + ( soc - 20 ) * 0.034 ;   // 51.2V - 53.6V (vlak plateau)
  } else if ( soc >= 10 ) {
    return 50.0 + ( soc - 10 ) * 0.12 ;    // 50.0V - 51.2V
  } else {
    return 48.0 + soc * 0.20 ;             // 48.0V - 50.0V
  }
}



// ****************************************************************************
//  device, version, SSID, IP, MAC-IP, MAC-BLE
// ****************************************************************************
void Get_Marstek_Device () {
  String Response = sendUDPCommand ( "Marstek.GetDevice", "{\"ble_mac\":\"0\"}" ) ;
/* Response Marstek.GetDevice: {
	"id":	6,
	"src":	"VenusE 3.0-18cedff563f9",
	"result":	{
		"device":	"VenusE 3.0",
		"ver":	144,
		"ble_mac":	"18cedff563f9",
		"wifi_mac":	"00167822c71c",
		"wifi_name":	"Home_Repeater",
		"ip":	"192.168.0.20"
	}
}
*/
}

// ****************************************************************************
void Get_BlueTooth_Status () {
// ****************************************************************************
  String Response = sendUDPCommand ( "BLE.GetStatus", "{\"id\":0}" ) ;
/* Response BLE.GetStatus: {
	"id":	3,
	"src":	"VenusE 3.0-18cedff563f9",
	"result":	{
		"id":	0,
		"state":	"disconnect",
		"ble_mac":	"18cedff563f9"
	}
*/
}

// ****************************************************************************
//  sinds firmware geupdate is van v135 naar v144 doet ie het weer
// ****************************************************************************
//void Get_EnergySystem_Status () {
bool ES_Get_Status () {
  String Response = sendUDPCommand ( "ES.GetStatus", "{\"id\":0}" ) ;
/* esponse ES.GetStatus: {
	"id":	9,
	"src":	"VenusE 3.0-18cedff563f9",
	"result":	{
		"id":	0,
		"bat_soc":	12,
		"bat_cap":	5120,
		"pv_power":	0,
		"ongrid_power":	0,
		"offgrid_power":	0,
		"total_pv_energy":	0,
		"total_grid_output_energy":	44583,
		"total_grid_input_energy":	56003,
		"total_load_energy":	0
	}
}
*/
  if ( Response.length () > 0 ) {
    JsonDocument Doc ;
    if ( !deserializeJson ( Doc, Response ) && Doc.containsKey ( "result" ) ) {
      JsonObject Result = Doc [ "result" ] ;

      AccuData.Total_Energy_In   = Result [ "total_grid_input_energy"  ] ;
      AccuData.Total_Energy_Out  = Result [ "total_grid_output_energy" ] ;

      Print_Battery () ;
      return true;
    }
  } 
  Serial.println ( "+++++++++++++++++++++++++++++++++++ Geen respons EM.GetStatus" ) ;
  return false ;
}

// ****************************************************************************
bool ES_Get_Mode () {
// ****************************************************************************
  String Response = sendUDPCommand ( "ES.GetMode", "{\"id\":0}" ) ;
/* Response ES.GetMode: {
	"id":	4,
	"src":	"VenusE 3.0-18cedff563f9",
	"result":	{
		"id":	0,
		"mode":	"AI",
		"ongrid_power":	517,
		"offgrid_power":	0,
		"bat_soc":	27,
		"ct_state":	1,
		"a_power":	22,
		"b_power":	480,
		"c_power":	-511,
		"total_power":	-8,
		"input_energy":	25092170,
		"output_energy":	21344430
	} 
}
MANUAL MODE: Response ES.GetMode: {
	"id":	4,
	"src":	"VenusE 3.0-18cedff563f9",
	"result":	{
		"id":	0,
		"mode":	"Manual",
		"ongrid_power":	0,
		"offgrid_power":	0,
		"bat_soc":	14
	}
}
*/
  if ( Response.length () > 0 ) {
    JsonDocument Doc ;
    if ( !deserializeJson ( Doc, Response ) && Doc.containsKey ( "result" ) ) {
      JsonObject Result = Doc [ "result" ] ;
      if ( Result [ "mode" ] == "AI" ) {

        //AccuData.Mode        = Result [ "a_power" ] | AccuData.Mode ;
        //AccuData.Power_A     = Result [ "a_power" ] | 0 ;

        AccuData.Power_A       = Result [ "a_power"       ] ;
        AccuData.Power_B       = Result [ "b_power"       ] ;
        AccuData.Power_C       = Result [ "c_power"       ] ;
        AccuData.Energy_In     = Result [ "input_energy"  ] ;
        AccuData.Energy_Out    = Result [ "output_energy" ] ;

        AccuData.Mode          = 1                          ; // 1= AI
        AccuData.OnGrid_Power  = Result [ "ongrid_power"  ] ;
        AccuData.OffGrid_Power = Result [ "offgrid_power" ] ;
        AccuData.Bat_SOC       = Result [ "bat_soc"       ] ;
      } 
      else {
        AccuData.Mode          = 2                          ; // 1= AI, 2= Manual
        AccuData.OnGrid_Power  = Result [ "ongrid_power"  ] ;
        AccuData.OffGrid_Power = Result [ "offgrid_power" ] ;
        AccuData.Bat_SOC       = Result [ "bat_soc"       ] ;
      }
      return true ;
    }
  } 
  Serial.println ( "+++++++++++++++++++++++++++++++++++ Geen respons ES.GetMode" ) ;
  return false ;
}

// ****************************************************************************
bool EM_Get_Status () {
//  MAAR dit is een subset van ES.GetMode
//  IN MANUAL MODE NIET
// ****************************************************************************
  String Response = sendUDPCommand ( "EM.GetStatus", "{\"id\":0}" ) ;
/* Response EM.GetStatus: {
	"id":	114,
	"src":	"VenusE 3.0-18cedff563f9",
	"result":	{
		"id":	0,
		"ct_state":	1,
		"a_power":	21,
		"b_power":	492,
		"c_power":	-508,
		"total_power":	6,
		"input_energy":	25089840,
		"output_energy":	21344430
	}
}
*/
  if ( Response.length () > 0 ) {
    JsonDocument Doc ;
    if ( !deserializeJson ( Doc, Response ) && Doc.containsKey ( "result" ) ) {
      if ( Mac_Address == "" ) {
        String Temp = Doc ["src" ] ;  // VenusE 3.0-18cedff563f9
        int Len = Temp.length() ;
        Mac_Address = Temp.substring ( Len-12 ) ;
      }

      JsonObject Result = Doc [ "result" ] ;

      AccuData.Power_A    = Result [ "a_power"       ] ;
      AccuData.Power_B    = Result [ "b_power"       ] ;
      AccuData.Power_C    = Result [ "c_power"       ] ;
      AccuData.Energy_In  = Result [ "input_energy"  ] ;
      AccuData.Energy_Out = Result [ "output_energy" ] ;

      AccuData.P1_Meter   = Result [ "ct_state"      ] ;
      
      Print_Battery () ;
      return true;
    }
  } 
  Serial.println ( "+++++++++++++++++++++++++++++++++++ Geen respons EM.GetStatus" ) ;
  return false ;
}


// ****************************************************************************
bool Bat_Get_Status () {
// ****************************************************************************
  String Response = sendUDPCommand ( "Bat.GetStatus", "{\"id\":0}" ) ;
/*Response Bat.GetStatus: {
	"id":	108,
	"src":	"VenusE 3.0-18cedff563f9",
	"result":	{
		"id":	0,
		"soc":	32,
		"charg_flag":	true,
		"dischrg_flag":	true,
		"bat_temp":	14.0,
		"bat_capacity":	1643.0,
		"rated_capacity":	5120.0
	}
}
*/  
  if ( Response.length () > 0 ) {
    JsonDocument Doc ;
    if ( !deserializeJson ( Doc, Response ) && Doc.containsKey ( "result" ) ) {
      JsonObject Result = Doc [ "result" ] ;

      AccuData.Bat_SOC          = Result [ "soc"          ] ;
      AccuData.Bat_Capacity     = int ( Result [ "bat_capacity" ] ) ;
      AccuData.Bat_Temperature  = Result [ "bat_temp"     ] ;
      
      Print_Battery () ;
/*
      batteryData.soc           = result [ "soc" ];

      batteryData.batCapacity   = result [ "bat_capacity" ] ;
      batteryData.batCapacity  *= 10 ;

      batteryData.ratedCapacity = result [ "rated_capacity" ] ;

      batteryData.batTemp       = result [ "bat_temp" ] ;
      batteryData.batTemp      /= 10 ;

      // BEREKENING VAN SPANNING EN STROOM
      // Voor LiFePO4 batterijen (zoals de Venus E):
      // - Nominale spanning per cel: 3.2V
      // - 16 cellen in serie = 51.2V nominaal
      // - Bereik: ~48V (leeg) tot ~58V (vol)
      
      // Bereken spanning op basis van SoC (geschatte curve voor LiFePO4)
      //float voltage = 48.0 + ( batteryData.soc / 100.0 ) * 10.0;  // 48V @ 0% tot 58V @ 100%
      //batteryData.batVoltage = (int) ( voltage * 1000 ) ;         // Converteer naar mV

      //float voltage_Better = calculateLiFePO4Voltage ( batteryData.soc ) ;
      //Serial.printf  ( "  Spanning (Better): %.2f V\n", voltage_Better ) ;
      batteryData.batVoltage = calculateLiFePO4Voltage ( batteryData.soc ) ;
      // Bereken stroom uit vermogen en spanning
      // P = U × I  -->  I = P / U
      if ( batteryData.batVoltage > 0 && AccuData.batPower != 0 ) {
      //if ( voltage > 0 && AccuData.batPower != 0 ) {
        float current = AccuData.batPower / batteryData.batVoltage ;       // Stroom in A
        //float current = AccuData.batPower / voltage ;       // Stroom in A
        batteryData.batCurrent = (int)( current * 1000 ) ;    // Converteer naar mA
      } else {
        batteryData.batCurrent = 0 ;
      }
      batteryData.lastUpdate    = millis () ;
      Serial.println ( "✓ Batterij data bijgewerkt" ) ;
      Serial.printf  ( "    SoC              : %d%%\n"   , batteryData.soc           ) ;
      Serial.printf  ( "    Capaciteit       : %.1f Wh\n", batteryData.batCapacity   ) ;
      Serial.printf  ( "    Rated capaciteit : %.1f Wh\n", batteryData.ratedCapacity ) ;
      Serial.printf  ( "    Temperatuur      : %.1f°C\n" , batteryData.batTemp       ) ;
      Serial.printf  ( "    Spanning         : %.2f V\n" , batteryData.batVoltage    ) ;
      Serial.printf  ( "    Stroom           : %.2f A\n" , batteryData.batCurrent / 1000.0 ) ;

      //getBatteryDataViaModbus () ; werkt niet
      //Serial.printf  ( "  Spanning (berekend): %.2f V\n", voltage ) ;
      //Serial.printf  ( "  Stroom (berekend): %.2f A\n", batteryData.batCurrent / 1000.0 ) ;
      //*/

      return true ;
    }
  } 
  Serial.println ( "+++++++++++++++++++++++++++++++++++ Geen respons Bat.GetStatus" ) ;
  return false ;
}

// ****************************************************************************
// WERKT NIET
// ****************************************************************************
void getEnergySystemStatus () {
  String response = sendUDPCommand ( "ES.GetStatus", "{\"id\":0}" ) ;
}

// ****************************************************************************
void updateAllData ( uint16_t Loop_Counter ) {
// ****************************************************************************
  #define Delay  5000
  unsigned long Delta ;

  bool Success ; 

  // **********************************
  // EM Get_Status altijd als niet in AI-mode
  // EM_Get_Status opvragen voor ES_Get_Mode verlgaad het aantal fouten aanzienlijk
  // **********************************
  if ( AccuData.Mode != 1 ) {
    Success = EM_Get_Status () ;
    if ( ! Success ) EM_Get_Status () ;
  }

  // **********************************
  // ES_Get_Mode doen we altijd
  // **********************************
  delay ( Delay ) ;
  Success = ES_Get_Mode () ;
  if ( ! Success ) ES_Get_Mode () ;
  int   Mode             = 0 ;  // 1=AI , 2=Manual

  // **********************************
  // Batterij Status doen we om de 10 keer
  // **********************************
  if ( ( Loop_Counter % 10 ) == 0 ) {
    //delay ( Delay ) ;
    Success = Bat_Get_Status () ;
    if ( ! Success ) Bat_Get_Status () ;
  }

  // **********************************
  // ES_Get_Status doen we om de 31 keer
  // uit de pas lopen dus niet precies 30
  // **********************************
  if ( ( Loop_Counter % 31 ) == 0 ) {
    //delay ( Delay ) ;
    Success = ES_Get_Status () ;
    if ( ! Success ) ES_Get_Status () ;
  }




  
}

#endif