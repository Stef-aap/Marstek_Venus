#ifndef Marstek_UDP_h
#define Marstek_UDP_h

#include <WiFiUdp.h>
#include <ArduinoJson.h>     // v7.x

// ****************************************************************************
// ES_Set_Mode nog te implementeren
// zie: https://static-eu.marstekenergy.com/ems/resource/agreement/MarstekDeviceOpenApi.pdf
// ****************************************************************************
/*
{  // ES.SetMode
  "id": 1,
  "method": "ES.SetMode",
  "params": {
    "id": 0,
    "config": {
      "mode": "Manual",
      "manual_cfg": {
        "time_num": 1,
        "start_time": "08:30",
        "end_time": "20:30",
        "week_set": 127,
        "power": 100,
        "enable": 1
      }
    }
  }
}
*/
// ****************************************************************************


// ****************************************************************************
// UDP Error messages, nog te implementeren
// ****************************************************************************
/*
-32700, Parse error, Invalid JSON was received by the server. An error occurred on the server while parsing the JSON text.
-32600, Invalid Request, The JSON sent is not a valid Request object.
-32601, Method not found The method does not exist / is not available.
-32602, Invalid params Invalid method parameter(s).
-32603, Internal error Internal JSON-RPC error.
-32000 to -32099, Server error Reserved for implementation-defined server-errors.
*/
// ****************************************************************************

// ****************************************************************************
// globals
// ****************************************************************************
int messageId = 0;


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
//  device, version, SSID, IP, MAC-IP, MAC-BLE
// ****************************************************************************
void Get_Marstek_Devices () {
  String Response = sendUDPCommand ( "Marstek.GetDevice", "{\"ble_mac\":\"0\"}" ) ;
/* Response Marstek.GetDevice: {
	"id":	6,
	"src":	"VenusE 3.0-18cedfxxxxxx",
	"result":	{
		"device":	"VenusE 3.0",
		"ver":	144,
		"ble_mac":	"18cedfxxxxxx",
		"wifi_mac":	"001678xxxxxx",
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
	"src":	"VenusE 3.0-18cedfxxxxxx",
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
bool ES_Get_Status () {
  String Response = sendUDPCommand ( "ES.GetStatus", "{\"id\":0}" ) ;
/* esponse ES.GetStatus: {
	"id":	9,
	"src":	"VenusE 3.0-18cedfxxxxxx",
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

      AccuData.Bat_Capacity      = Result [ "bat_cap"       ] ;
      AccuData.Bat_SOC           = Result [ "bat_soc"       ] ;
      AccuData.OnGrid_Power      = Result [ "ongrid_power"  ] ;
      AccuData.OffGrid_Power     = Result [ "offgrid_power" ] ;

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
	"src":	"VenusE 3.0-18cedfxxxxxx",
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
	"src":	"VenusE 3.0-18cedfxxxxxx",
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
	"src":	"VenusE 3.0-18cedfxxxxxx",
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
      if ( MAC_Address == "" ) {
        String Temp = Doc ["src" ] ;  // VenusE 3.0-18cedff563f9
        int Len = Temp.length() ;
        MAC_Address = Temp.substring ( Len-12 ) ;
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
	"src":	"VenusE 3.0-18cedfxxxxxx",
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

      AccuData.Bat_SOC          =       Result [ "soc"            ] ;
      AccuData.Bat_Lading       = int ( Result [ "bat_capacity"   ] ) ;
      AccuData.Bat_Capacity     = int ( Result [ "rated_capacity" ] ) ;
      AccuData.Bat_Temperature  =       Result [ "bat_temp"       ] ;
      
      Print_Battery () ;
      return true ;
    }
  } 
  Serial.println ( "+++++++++++++++++++++++++++++++++++ Geen respons Bat.GetStatus" ) ;
  return false ;
}


// ****************************************************************************
// WERKT NIET
// ****************************************************************************
/*void ES_Get_Status () {
  String response = sendUDPCommand ( "ES.GetStatus", "{\"id\":0}" ) ;
}*/


// ****************************************************************************
void updateAllData ( uint16_t Loop_Counter ) {
// ****************************************************************************
  #define Delay  5000
  unsigned long Delta   ;
  bool          Success ; 

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