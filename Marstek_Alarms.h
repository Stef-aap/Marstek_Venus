#ifndef Marstek_Alarms_h
#define Marstek_Alarms_h

// ****************************************************************************
// ****************************************************************************
// Alarm drempelwaarden
const int BATTERY_LOW_THRESHOLD      = 20;
const int BATTERY_CRITICAL_THRESHOLD = 10;
const int TEMP_HIGH_THRESHOLD        = 45;

bool alarmLowBattery = false;
bool alarmCriticalBattery = false;
bool alarmHighTemp = false;


// ****************************************************************************
// ==================== ALARMEN ====================
// ****************************************************************************
void checkAlarms() {
  if (batteryData.soc <= BATTERY_CRITICAL_THRESHOLD && !alarmCriticalBattery) {
    alarmCriticalBattery = true;
    Serial.println("⚠️  ALARM: Kritiek batterij niveau!");
  } else if (batteryData.soc > BATTERY_CRITICAL_THRESHOLD) {
    alarmCriticalBattery = false;
  }
  
  if (batteryData.soc <= BATTERY_LOW_THRESHOLD && !alarmLowBattery) {
    alarmLowBattery = true;
    Serial.println("⚠️  ALARM: Laag batterij niveau");
  } else if (batteryData.soc > BATTERY_LOW_THRESHOLD) {
    alarmLowBattery = false;
  }
  
  if (batteryData.batTemp >= TEMP_HIGH_THRESHOLD && !alarmHighTemp) {
    alarmHighTemp = true;
    Serial.println("⚠️  ALARM: Hoge temperatuur!");
  } else if (batteryData.batTemp < TEMP_HIGH_THRESHOLD) {
    alarmHighTemp = false;
  }
}


#endif