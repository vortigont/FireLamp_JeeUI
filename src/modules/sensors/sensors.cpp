/*
 *  This is a sensors class code derived from Infoclock project - https://github.com/vortigont/infoclock
 *
 *  Author      : Emil Muratov
 *
 *  This file        : sensors.h
 *  This file Author : Emil Muratow
 *
 *  Description      : sensors poller/parser
 *
 */
#include "sensors.h"


void GenericSensor::run(){
  std::time_t now;
  std::time(&now);
  if (now - last_poll_tstamp >= poll_rate){
    poll();
    last_poll_tstamp = now;
  }
}


/**
 * default constructor
**/
SensorManager::SensorManager() : GenericModule(T_sensors, true){
  set( 1000, TASK_FOREVER, [this](){ _poll_sensors(); } );
  ts.addTask(*this);
}

/**
 * default destructor
**/
//SensorManager::~SensorManager(){}
/*
void SensorManager::begin(){
  Wire.begin();
  if (_bosch.begin()) {
    _bosch_model = _bosch.chipModel() == _bosch.ChipModel::ChipModel_BME280 ? sensor_t::bme280: sensor_t::bmp280;
    _bosch_present = true;
  }
  
  if (_si702x.begin())  {
    _si702_present = true;
    //readsi7021(temp, humidity);
  }

  if (sgp30.begin()){
    _sgp_present = true;
    sgp30.initAirQuality();
  }

}
*/

void SensorManager::load_cfg(JsonVariantConst cfg){
  // read i2c gpio's
  _i2c_scl = cfg[T_i2c][T_scl] | -1;
  _i2c_scl = cfg[T_i2c][T_sda] | -1;

}

void SensorManager::start(){
  Wire.setPins(_i2c_sda, _i2c_scl);
  Wire.begin();
  enable();
};

void SensorManager::stop(){
  disable();
  Wire.end();
};


void SensorManager::_poll_sensors(){
  //std:rand()
}



// **************************************

void SensorManager::readbme280(float& t, float& h, float& p, float& dew) {
    //BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
    //BME280::PresUnit presUnit(BME280::PresUnit_torr);
    _bosch.read(p, t, h, BME280::TempUnit_Celsius, BME280::PresUnit_torr);
     //EnvironmentCalculations::TempUnit     envTempUnit =  EnvironmentCalculations::TempUnit_Celsius;
     //dew = EnvironmentCalculations::DewPoint(t, h, envTempUnit);
 }

void SensorManager::readsi7021(float& t, float& h) {
  h = _si702x.readHumidity();
  // try to reset sensor on read error
  if ( h == HTU2XD_SHT2X_SI70XX_ERROR ){
    _si702x.softReset();
    _si702x.setResolution(HUMD_12BIT_TEMP_14BIT);
  }

  t = _si702x.readTemperature(READ_TEMP_AFTER_RH);
}

// Update string with sensor's data
bool SensorManager::getFormattedValues(String &str){
  char sensorstr[SENSOR_DATA_BUFSIZE];
  getFormattedValues(sensorstr);

  str = sensorstr;

  if (!issgp)
    return true;

  // add air quality
  readsgp30(co2, tvoc, temp, humidity);
  str += " CO2:";
  str += co2;
  str += " ppm, tvoc:";
  str += tvoc;
  str += " ppb";

  return true;
}

bool SensorManager::getFormattedValues(char* str) {
    switch(_bosch_model) {
      case sensor_t::bme280 :
      case sensor_t::bmp280 :
            readbme280(temp, humidity, pressure, dew);

            switch (s_bosch.chipModel()){
              case BME280::ChipModel_BME280:
                snprintf_P(str, SENSOR_DATA_BUFSIZE, PSTR("T:%.1f Rh:%.f%% P:%.fmmHg"), temp + toffset, humidity, pressure);
                return true;
              default:
              //case BME280::ChipModel_BMP280:
                snprintf_P(str, SENSOR_DATA_BUFSIZE, PSTR("T:%.1f P:%.fmmHg"), temp + toffset, pressure);
                return true;
            }
      case sensor_t::si7021 :
            readsi7021(temp, humidity);
            snprintf_P(str, SENSOR_DATA_BUFSIZE, PSTR("T:%.1f Rh:%.f%%"), temp + toffset, humidity);
            return true;
      default:
            snprintf_P(str, SENSOR_DATA_BUFSIZE, PSTR("Temp sensor err!"));
            return false;
    }
}
/*
void SensorManager::getSensorModel(String &str){
  str = F("SensorManager: ");
  str += sensor_types[(uint8_t)_bosch_model];
  if (issgp)
    str += F(", SGP30");
}
*/

void SensorManager::sgp30poll(){
  if (!issgp)
    return;
  sgp30.measureAirQuality(); 
}

void SensorManager::readsgp30(uint16_t &co2, uint16_t &tvoc, float rh, float t){
  if (!issgp)
    return;
  // read current values
  co2 = sgp30.CO2;
  tvoc = sgp30.TVOC;

  // Update sensor with humi data
  //Convert relative humidity to absolute humidity
  double absHumidity = RHtoAbsolute(rh, t);
  //Convert the double type humidity to a fixed point 8.8bit number
  uint16_t sensHumidity = doubleToFixedPoint(absHumidity);
  sgp30.setHumidity(sensHumidity);
}


float SensorManager::RHtoAbsolute (float relHumidity, float tempC) {
  float eSat = 6.11 * pow(10.0, (7.5 * tempC / (237.7 + tempC)));
  float vaporPressure = (relHumidity * eSat) / 100; //millibars
  float absHumidity = 1000 * vaporPressure * 100 / ((tempC + 273) * 461.5); //Ideal gas law with unit conversions
  return absHumidity;
}

uint16_t SensorManager::doubleToFixedPoint( double number) {
  int power = 1 << 8;
  double number2 = number * power;
  uint16_t value = floor(number2 + 0.5);
  return value;
}

/*
float SensorManager::tempoffset(float t){
  if (t != NAN)
    toffset = t;
  
  return toffset;
};
*/