#ifndef Marstek_BLE_h
#define Marstek_BLE_h

// ****************************************************************************
//
// https://github.com/jaapp/ha-marstek-ble/blob/main/docs/ENTITIES.md
//
// Hex Conversie: https://www.rapidtables.com/convert/number/hex-to-decimal.html?x=FEC3
// ****************************************************************************

static BLEUUID    serviceUUID ( "0000ff00-0000-1000-8000-00805f9b34fb" ) ;
static BLEUUID    charUUID    ( "0000ff02-0000-1000-8000-00805f9b34fb" ) ;


static BLEClient* pClient = nullptr;
static BLERemoteCharacteristic* pChar = nullptr;
static bool BLE_Connected = false;
static bool dataReceived = false;

int bleFailCount      = 0 ;
const int maxRetries = 5; // Reset na 5 mislukkingen

#define BLE_Device_Info   0
#define BLE_Runtime_Info  1
#define BLE_BMS_Data      2

// ****************************************************************************
// ****************************************************************************
uint16_t getU16 ( uint8_t* d, int o ) { return d[o] | (d[o + 1] << 8); }
int16_t  getI16 ( uint8_t* d, int o ) { return (int16_t)getU16(d, o);  }

// ****************************************************************************
// ****************************************************************************
float pub ( const char* Text, float Value ) {
  char msg[16] ;
  snprintf ( msg, 16, "%.3f", Value );
  Serial.print   ( Text ) ;
  Serial.print ( ":" ) ;
  Serial.println ( msg ) ;
  return Value ;
}

// ****************************************************************************
// ****************************************************************************
int pub ( const char* Text, int Value ) {
  char msg[16];
  snprintf ( msg, 16, "%d", Value ) ;
  Serial.print   ( Text ) ;
  Serial.print ( ":" ) ;
  Serial.println ( msg   ) ;
  return Value ;
}

// ****************************************************************************
// BLE Connect / Disconnect completion routines
// ****************************************************************************
class ConnCallback : public BLEClientCallbacks {
  void onConnect    ( BLEClient* c ) { BLE_Connected = true  ; }
  void onDisconnect ( BLEClient* c ) { BLE_Connected = false ; }
};

// ****************************************************************************
static void Process_BLE_Answer ( BLERemoteCharacteristic* c, uint8_t* d, size_t len, bool notify ) {
// ****************************************************************************
  for ( int i=0; i<len; i++ ) {
    //Serial.print ( d[i], HEX ) ;
    Serial.printf("%02X ", d[i]);
    Serial.print ( " " ) ;
  }
  Serial.println ();

  // ************************************
  // Device Info
  // ************************************
  if      ( d[3] == 0x04 ) {
    String Line = String ( (char*) d ) ;
    //Serial.println ( Line ) ;
    Line = Line.substring ( 4, 117 ) ;
    #ifdef Debug
      Serial.println ( "Device Info" ) ;
      Serial.println ( Line ) ;
    #endif
    //type=VNSE3-0,id=291a12323732595650110136,mac=18cedff563f9,dev_ver=144,bms_ver=110,fc_ver=202409090159,inv_ver=116
    //Success BLE Answer Received

    int i1 = 0;
    int i2 = 0;
    i1 = Line.indexOf ( "dev_ver=" ) + 8 ;
    i2 = Line.indexOf ( ",", i1 ) ;
    AccuData.EMS_version = Line.substring (i1, i2 ).toInt() ;

    i1 = Line.indexOf ( "bms_ver=" ) + 8 ;
    i2 = Line.indexOf ( ",", i1 ) ;
    AccuData.BMS_version = Line.substring (i1, i2 ).toInt() ;

    i1 = Line.indexOf ( "inv_ver=" ) + 8 ;
    i2 = Line.indexOf ( ",", i1 ) ;
    AccuData.VMS_version = Line.substring (i1, i2 ).toInt() ;
  }
  // ************************************
  // Runtime Info
  // ************************************
  else if ( d[3] == 0x03 ) {
    uint8_t* p = &d[4]  ;
    String Line ;

    Serial.print   ( "Firmware Version =" ) ;
    Serial.println ( d[16] ) ;
    
    Serial.print ( "Firmware Build = " ) ;
    Line = "" ;
    Line += String ( (char)d[85] ) ;
    Line += String ( (char)d[86] ) ;
    Line += String ( (char)d[87] ) ;
    Line += String ( (char)d[88] ) ;
    Line += '-' ;
    Line += String ( (char)d[89] ) ;
    Line += String ( (char)d[90] ) ;
    Line += '-' ;
    Line += String ( (char)d[91] ) ;
    Line += String ( (char)d[92] ) ;
    Line += ' ' ;
    Line += String ( (char)d[93] ) ;
    Line += String ( (char)d[94] ) ;
    Line += ':' ;
    Line += String ( (char)d[95] ) ;
    Line += String ( (char)d[96] ) ;
    Serial.println ( Line ) ;
    String Firmware_Build = Line ;

    pub ( "Status (1=Standby,  2=Laden,  3=Ontladen)" , d [8] ) ;

    AccuData.OnGrid_Power = pub ( "Power OnGrid = " , getI16 ( p, 116 ) ) ;  // 118 lijkt een kopie te zijn
    //AccuData.OnGrid_Power = pub ( "Power OnGrid = " , getI16 ( p, 118 ) ) ;  // 118 lijkt een kopie te zijn
  }

  // ************************************
  // BMS DATA
  // ************************************
  else if ( d[3] == 0x14 ) {
    if (len >= 85 && d[0] == 0x73 && d[1] == 0x55 && d[2] == 0x23 ) {
      uint8_t* p = &d[4]  ;
      float Temp;

      AccuData.BMS_version = pub ( "BMS_Version", (int)getU16 ( p, 0 ));
      pub ( "Max_Voltage"          , getU16(p, 2) / 10.0f );
      Temp = pub ( "Max_Charge_Current"   , getI16(p, 4) / 10.0f );
      Temp = pub ( "Max_Discharge_Current", getI16(p, 6) / 10.0f );
      AccuData.Bat_SOC     = pub ( "Bat_SOC", (int)getU16 ( p, 8 ));
      Temp = pub ( "soh", (int)getU16 ( p, 10 ));
      AccuData.Bat_Capacity = pub ( "Bat_Capaciteit", (int)getU16(p, 12));
      
      //Temp = calculateLiFePO4Voltage ( AccuData.Bat_SOC ) ;
      //Serial.print ( "Voltage Calculated = " ) ;
      //Serial.println ( Temp ) ;

      float Voltage         = getU16 ( p, 14) / 100.0f;
      float Current         = getI16 ( p, 16) / 10.0f;
      AccuData.Voltage      = pub ( "voltage", Voltage ) ;
      AccuData.Current      = pub ( "current", Current ) ;
      AccuData.OnGrid_Power = -Voltage * Current ;

      AccuData.Bat_Temperature    = pub ( "batt_temp" , (int)getU16 ( p, 18 ));
      AccuData.Bat_Lading         = pub ( "Bat_Lading", (int)getU16 ( p, 22 ));

      //Temp = pub ( "???", (int)getU16 ( p, 36 ));  // zwabbert tussen de 145 en 147 ??
      // Rest lijkt allemaal 0

      Temp = pub("error", (int)getU16(p, 26));
      AccuData.MosFET_Temperature = pub ( "mosfet_temp", getU16 ( p, 38 ) / 10.0f ) ;
      AccuData.Temperature_1      = pub ( "temp1", getU16 ( p, 40 ) / 10.0f );
      AccuData.Temperature_2      = pub ( "temp2", getU16 ( p, 42 ) / 10.0f );
      AccuData.Temperature_3      = pub ( "temp3", getU16 ( p, 44 ) / 10.0f );
      AccuData.Temperature_4      = pub ( "temp4", getU16 ( p, 46 ) / 10.0f );
      
      int minV=6000, maxV=0;
      for (int i = 0; i < 16; i++) {
        int cv = getU16 ( p, 48 + i * 2 ) ;
        AccuData.Cell [i] = cv ;           // STORE CELL VOLTAGE
        char ct[8];
        snprintf ( ct, 8, "cell-%d", i + 1 ) ;
        Temp = pub ( ct, cv ) ;
        if ( cv < minV ) minV = cv;
        if ( cv > maxV ) maxV = cv;
      }
      
      AccuData.Cell_Min   = pub ( "cell_min"  , minV        );
      AccuData.Cell_Max   = pub ( "cell_max"  , maxV        );
      AccuData.Cell_Delta = pub ( "cell_delta", maxV - minV );
      Serial.print ( "Min,Max,Delta ");
      Serial.println ( minV ) ;
      Serial.print ( "Min,Max,Delta ");
      Serial.println ( maxV ) ;
      Serial.print ( "Min,Max,Delta ");
      Serial.println ( AccuData.Cell_Delta ) ;
    }
  } 
  else {
    Serial.println ( "BLE received incorrect Data" ) ;
  }
  dataReceived = true;
}


// ****************************************************************************
// Probeer met een BLE-Client een connection op te zetten met de Marstek Venus
//   zet completion routine voor de verwerking van het inkomende antwoord
// ****************************************************************************
bool connectBLE () {
  Serial.print("BLE connect...");
  
  // Als er al een client is, die hergebruiken
  if (pClient == nullptr) {
    pClient = BLEDevice::createClient();
    pClient->setClientCallbacks(new ConnCallback());
  }
  
  if (!pClient->connect(bleAddress)) {
    Serial.println("FAILED");
    return false;
  }
  pClient->setMTU(517);
  
  BLERemoteService* svc = pClient->getService(serviceUUID);
  if (!svc) {
    Serial.println("NO SERVICE");
    pClient->disconnect();
    return false;
  }
  
  pChar = svc->getCharacteristic(charUUID);
  if (!pChar) {
    Serial.println("NO CHAR");
    pClient->disconnect();
    return false;
  }
  
  // completion routine voor de verwerking van het inkomende antwoord
  if ( pChar->canNotify() ) {
    pChar->registerForNotify ( Process_BLE_Answer ) ; //notifyCallback ) ;
  }

  BLE_Connected    = true  ;
  dataReceived = false ;
  Serial.println ( "OK" ) ;
  return true ;
}


// ****************************************************************************
// ****************************************************************************
void disconnectBLE() {
  if ( pClient && BLE_Connected ) {
    pChar = nullptr; // Eerst characteristic resetten
    pClient->disconnect();
    BLE_Connected = false;
    delay(1000); // Langere delay voor cleanup
  }
  // Niet deleten - dit veroorzaakt de crash!
}

// ****************************************************************************
bool Send_BLE_Command ( uint8_t Command = 0 ) {
// ****************************************************************************
    Serial.print   ( "=== BLE Request Send ===  command=" ) ;
    Serial.println ( Command ) ;
    dataReceived = false;
    if ( connectBLE() ) {
      bleFailCount = 0; // Reset teller bij succes
      uint8_t cmd_Device_Info  [ 5 ] = { 0x73, 0x05, 0x23, 0x04, 0x51 } ;
      uint8_t cmd_Runtime_Info [ 5 ] = { 0x73, 0x05, 0x23, 0x03, 0x56 } ;
      uint8_t cmd_BMS_Data     [ 5 ] = { 0x73, 0x05, 0x23, 0x14, 0x41 } ;
      if      ( Command == BLE_Device_Info  ) pChar->writeValue ( cmd_Device_Info , 5, false ) ;
      else if ( Command == BLE_Runtime_Info ) pChar->writeValue ( cmd_Runtime_Info, 5, false ) ;
      else                                    pChar->writeValue ( cmd_BMS_Data    , 5, false ) ;
      
      // wacht (maximaal 5 seconden) totdat Data ontvangen
      unsigned long start = millis();
      while (!dataReceived && (millis() - start < 5000)) {
        delay(100);
      }
      
      if (dataReceived) {
        Serial.println("Success BLE Answer Received");
      } else {
        Serial.println("Timeout");
      }
      
      disconnectBLE();
      Serial.println();
      return true ;
    } 
    else {
      bleFailCount++;
      Serial.print("BLE Failed (");
      Serial.print(bleFailCount);
      Serial.println(")");
      
      // Probeer BLE stack te resetten bij herhaalde fouten
      if (bleFailCount >= 3) {
        Serial.println("Resetting BLE stack...");
        if (pClient) {
          pClient->disconnect();
        }
        pClient = nullptr;
        pChar = nullptr;
        BLEDevice::deinit(false);
        delay(2000);
        BLEDevice::init("");
        bleFailCount = 0;
      }
      
      // Als ook dat niet helpt, restart ESP
      if ( bleFailCount >= maxRetries ) {
        Serial.println ( "BLE failures >= maxRetries, RESTARTING ESP..." ) ;
        delay ( 1000 ) ;
        ESP.restart() ;
      }
      Serial.println();
      return false;
    }
}
#endif