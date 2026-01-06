#ifndef Marstek_Data_h
#define Marstek_Data_h

struct AccuData {
  int   Mode             = 0 ;  // 1=AI , 2=Manual, 3=UPS
  int   P1_Meter         = 0 ;
  
  int   Power_A          = 0 ;
  int   Power_B          = 0 ;
  int   Power_C          = 0 ;
  int   OnGrid_Power     = 0 ;   // positief = ontladen
  int   OffGrid_Power    = 0 ;
  
  int   Bat_SOC          = 0 ;
  int   Bat_Lading       = 0 ;
  int   Bat_Capacity     = 0 ;
  float Bat_Temperature  = 0.0 ;


  int   Energy_In        = 0 ;
  int   Energy_Out       = 0 ;
  int   Total_Energy_In  = 0 ;  
  int   Total_Energy_Out = 0 ;  

// Uitbreidingen vanuit BLE
  int   BMS_version        = 0 ;     // BMS_ver
  int   EMS_version        = 0 ;     // dev_ver
  int   VMS_version        = 0 ;     // inv_ver
  float Voltage            = 0.0 ;
  float Current            = 0.0 ;
  float MosFET_Temperature = 0.0 ;
  float Temperature_1      = 0.0 ;
  float Temperature_2      = 0.0 ;
  float Temperature_3      = 0.0 ;
  float Temperature_4      = 0.0 ;

  uint16_t  Cell_Min       = 0 ;
  uint16_t  Cell_Max       = 0 ;
  uint16_t  Cell_Delta     = 0 ;
  
  uint16_t Cell [ 16 ]     = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

} AccuData;


// ****************************************************************************
void Print_Battery () {
// ****************************************************************************
  int Rendement ;

  if      ( AccuData.OnGrid_Power == 0 ) Serial.println ( "====  Stand By  ====" ) ;
  else if ( AccuData.OnGrid_Power  > 0 ) Serial.println ( "====  Ontladen  ====" ) ;
  else                                     Serial.println ( "====  Laden  ====" ) ;

  Serial.printf   ( "  Mode             = %i (1=AI, 2=Manual, 3=UPS)\n", AccuData.Mode     ) ;
  Serial.printf   ( "  P1_Meter         = %i\n", AccuData.P1_Meter ) ;
  
  Serial.printf   ( "  BMS version      = %i\n", AccuData.BMS_version ) ;
  Serial.printf   ( "  EMS version      = %i\n", AccuData.EMS_version ) ;
  Serial.printf   ( "  VMS version      = %i\n", AccuData.VMS_version ) ;
  Serial.printf   ( "  Voltage          = %i\n", AccuData.Voltage     ) ;
  Serial.printf   ( "  Current          = %i\n", AccuData.Current     ) ;
  
  Serial.printf   ( "  Power_A          = %4i W\n", AccuData.Power_A ) ;
  Serial.printf   ( "  Power_B          = %4i W\n", AccuData.Power_B ) ;
  Serial.printf   ( "  Power_C          = %4i W\n", AccuData.Power_C ) ;

  Serial.printf   ( "  OnGrid_Power     = %4i W\n" , AccuData.OnGrid_Power    ) ;
  Serial.printf   ( "  OffGrid_Power    = %4i W\n" , AccuData.OffGrid_Power   ) ;
  Serial.printf   ( "  Bat_SOC          = %4i %%\n", AccuData.Bat_SOC         ) ;
  Serial.printf   ( "  Bat_Lading       = %4i %%\n", AccuData.Bat_Lading      ) ;
  Serial.printf   ( "  Bat_Capacity     = %4i Wh\n", AccuData.Bat_Capacity    ) ;

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

  Serial.printf   ( "  Bat_Temperature  =%5.1f °C\n", AccuData.Bat_Temperature ) ;
  Serial.printf   ( "  MosFET_Temp      =%5.1f °C\n", AccuData.Bat_Temperature ) ;
  Serial.printf   ( "  Temperature-1    =%5.1f °C\n", AccuData.Temperature_1   ) ;
  Serial.printf   ( "  Temperature-2    =%5.1f °C\n", AccuData.Temperature_2   ) ;
  Serial.printf   ( "  Temperature-3    =%5.1f °C\n", AccuData.Temperature_3   ) ;
  Serial.printf   ( "  Temperature-4    =%5.1f °C\n", AccuData.Temperature_4   ) ;
  
  Serial.printf   ( "  Cell Voltage Min   =%5.3f °C\n", AccuData.Cell_Min   / 1000.0 ) ;
  Serial.printf   ( "  Cell Voltage Max   =%5.3f °C\n", AccuData.Cell_Max   / 1000.0 ) ;
  Serial.printf   ( "  Cell Voltage Delta =%5.3f °C\n", AccuData.Cell_Delta / 1000.0 ) ;
  
  Serial.print ( "Cell Voltages  [1-8] : " ) ;
  for ( int i=0; i<8; i++ ) {
    Serial.print ( AccuData.Cell [i] / 1000.0, 3 ) ;
    Serial.print ( "  " ) ;
  }
  Serial.println () ; 
  Serial.print ( "Cell Voltages [9-16] : " ) ;
  for ( int i=8; i<16; i++ ) {
    Serial.print ( AccuData.Cell [i] / 1000.0, 3 ) ;
    Serial.print ( "  " ) ;
  }
  Serial.println () ; 
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
// BEREKENING VAN SPANNING EN STROOM
// Voor LiFePO4 batterijen (zoals de Venus E):
// - Nominale spanning per cel: 3.2V
// - 16 cellen in serie = 51.2V nominaal
// - Bereik: ~48V (leeg) tot ~58V (vol)
// ****************************************************************************
/*
float calculateLiFePO4Voltage ( int SOC ) {
  // Bereken spanning op basis van SoC (geschatte curve voor LiFePO4)
  float Voltage = 48.0 + ( SOC / 100.0 ) * 10.0;  // 48V @ 0% tot 58V @ 100%
  return Voltage ;
}
//*/

#endif