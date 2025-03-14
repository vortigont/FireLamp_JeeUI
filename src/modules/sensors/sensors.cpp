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
#include <format>
#include <string_view>
#include "components.hpp"
#include "modules/mod_textq.hpp"
#include "Wire.h"
#include "sensors.h"
#include "EmbUI.h"
#include "log.h"

#define SENSOR_UPD_PERIOD   10  // Default Update rate in seconds
#define SENSOR_DATA_BUFSIZE 25  // chars for sensors formatted data

// configuration file name
static constexpr const char T_sensors_cfg[] = "sensors.json";

// List of sensor types
static constexpr const char T_Bosch_BMx[] = "Bosch_BMx";
//static constexpr const char T_Si702x[] = "Si702x";
static constexpr const char T_SGP30[] = "SGP30";

/*
// array with all available module names (labels) we can run
static constexpr std::array<const char*, 3> sensor_types_list = {
  T_Bosch_BMx,
  T_Si702x,
  T_SGP30
};
*/

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
SensorManager::SensorManager() : GenericModule(T_sensors, false){
  set( 1000, TASK_FOREVER, [this](){ _poll_sensors(); } );
  ts.addTask(*this);
}

SensorManager::~SensorManager(){
  ts.deleteTask(*this);
}

void SensorManager::load_cfg(JsonVariantConst cfg){
  // read i2c gpio's
  _i2c_scl = cfg[T_i2c][T_scl] | -1;
  _i2c_sda = cfg[T_i2c][T_sda] | -1;

  if (_i2c_sda == -1 || _i2c_scl == -1){
    LOGE(T_sensors, printf, "i2c bus pins are not configured, sda:%d, scl:%d\n", _i2c_scl, _i2c_scl);
    return;
  }
  // destory all sensors
  _sensors.clear();
  // init i2c bus
  Wire.setPins(_i2c_sda, _i2c_scl);
  if (!Wire.begin()){
    LOGE(T_sensors, println, "i2c init err");
    return;
  }

  JsonArrayConst arr = cfg[T_sensors];

  for(JsonVariantConst v : arr){
    LOGD(T_sensors, println, "creating sensors pool");
    if (!v[T_enabled])
      continue;

    sensor_pt s;
    std::string_view lbl;
    if (v[T_type].is<const char*>())
      lbl = v[T_type].as<const char*>();
    else{
      LOGV(T_sensors, println, "wrong sensor type!");
      continue;
    }

    // go through all known sensors

    // bosch BMx
    if (std::string_view(lbl).compare(T_Bosch_BMx) == 0){
      LOGI(T_sensors, printf, "Load: %s\n", T_Bosch_BMx);
      s = std::make_unique<Sensor_Bosch>(v[P_id] | random());
      // load sensor's configuration
      s->load_cfg(v);
      // try to init sensor, if fails - then discard the object
      if (s->init())
        _sensors.emplace_back(std::move(s));
      continue;
    }

    // SGP30
    if (std::string_view(lbl).compare(T_SGP30) == 0){
      LOGI(T_sensors, printf, "Load: %s\n", T_SGP30);
      s = std::make_unique<Sensor_SGP>(v[P_id] | random());
      // load sensor's configuration
      s->load_cfg(v);
      // try to init sensor, if fails - then discard the object
      if (s->init())
        _sensors.emplace_back(std::move(s));
      continue;
    }
  }

}

void SensorManager::start(){
  // enable periodic poll
  enable();
};

void SensorManager::stop(){
  disable();
};

void SensorManager::_poll_sensors(){
  //LOGV(T_sensors, println, "SM poll");
  for (auto &s : _sensors){
    if (s->getState())
      s->run();
  }
}



// **************************************
/*
void SensorManager::readsi7021(float& t, float& h) {
  h = _si702x.readHumidity();
  // try to reset sensor on read error
  if ( h == HTU2XD_SHT2X_SI70XX_ERROR ){
    _si702x.softReset();
    _si702x.setResolution(HUMD_12BIT_TEMP_14BIT);
  }

  t = _si702x.readTemperature(READ_TEMP_AFTER_RH);
}
*/
/*
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

*/
/*
void SensorManager::getSensorModel(String &str){
  str = F("SensorManager: ");
  str += sensor_types[(uint8_t)_bosch_model];
  if (issgp)
    str += F(", SGP30");
}

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
*/



// ======= Bosch sensor
float Sensor_Bosch::RHtoAbsolute (float relHumidity, float tempC) {
  float eSat = 6.11 * pow(10.0, (7.5 * tempC / (237.7 + tempC)));
  float vaporPressure = (relHumidity * eSat) / 100; //millibars
  float absHumidity = 1000 * vaporPressure * 100 / ((tempC + 273) * 461.5); //Ideal gas law with unit conversions
  return absHumidity;
}

uint16_t Sensor_Bosch::doubleToFixedPoint( double number) {
  int power = 1 << 8;
  double number2 = number * power;
  uint16_t value = floor(number2 + 0.5);
  return value;
}

void Sensor_Bosch::load_cfg(JsonVariantConst cfg){
  descr = cfg[T_descr].as<const char*>();
  poll_rate = cfg[T_publish_rate] | SENSOR_UPD_PERIOD;
  scroller_id = cfg[T_destination];

}

bool Sensor_Bosch::init(){
  online = false;
  if (!_bosch.begin()){
    LOGE(T_sensors, println, "bosch BMx init err!");
    return false;
  }

  switch(_bosch.chipModel()){
     case BME280::ChipModel_BME280:
       stype = sensor_t::bosch_bme;
       LOGI(T_sensors, println, "Found BME280");
       break;

     case BME280::ChipModel_BMP280:
       LOGI(T_sensors, println, "Found BMP280");
       stype = sensor_t::bosch_bmp;
       break;

     default:
       LOGW(T_sensors, println, "No Bosch BMx found!");
       return false;
  }
  online = true;
  return online;
}

void Sensor_Bosch::poll(){
  //LOGV(T_sensors, println, "BMx poll");
  _bosch.read(pressure, temp, humidity, BME280::TempUnit_Celsius, BME280::PresUnit_torr);
  auto scroller = zookeeper.getModulePtr(T_txtscroll);

  // no text destination available
  if (!scroller)
    return;

  std::string buffer = descr;
  buffer += std::format(" температура: {:.1f}°С, атм. давление: {:.1f} мм.рт.ст.", temp, pressure);
  if (stype == sensor_t::bosch_bme)
    buffer += std::format(", влажность: {:.1f}%", humidity);

  TextMessage m(std::move(buffer), 1, 0, message_id);
  static_cast<ModTextScroller*>(scroller)->updateMSG(std::move(m), scroller_id);
}


// ======= SGP30 gas sensor

void Sensor_SGP::load_cfg(JsonVariantConst cfg){
  descr = cfg[T_descr].as<const char*>();
  poll_rate = 1;
  _pub_rate = cfg[T_publish_rate] | SENSOR_UPD_PERIOD;
  scroller_id = cfg[T_destination];
}

bool Sensor_SGP::init(){
  online = false;
  if (!_sensor.begin()){
    LOGE(T_sensors, println, "SGP30 init err!");
    return false;
  }
  _sensor.initAirQuality();

  online = true;
  return online;
}

void Sensor_SGP::poll(){
  _sensor.measureAirQuality();
  if (++_ctr % _pub_rate != 0)
    return;

  auto scroller = zookeeper.getModulePtr(T_txtscroll);
  // no text destination available
  if (!scroller)
    return;

  // todo: need to feed sensor with data from temp/humi sensor if that one is available
  std::string buffer = descr;
  buffer += std::format(" CO2: {}ppm, tvoc: {}", _sensor.CO2, _sensor.TVOC);

  TextMessage m(std::move(buffer), 1, 0, message_id);
  static_cast<ModTextScroller*>(scroller)->updateMSG(std::move(m), scroller_id);
}
