/*
 *  InfoClock - ESP8266 based informeter/wall-clock
 *  ESP8266 contoller uses Max72xx modules as a display
 *  Internal/external sensors/message brokers could be used as an information source
 *
 *  Author      : Emil Muratov
 *
 *  This file        : sensors.h
 *  This file Author : Emil Muratow
 *
 *  Description      : sensors poller/parser
 *
*/

#pragma once
#include "modules/mod_manager.hpp"

//Baro sensor
//#include <EnvironmentCalculations.h>
#include <BME280I2C.h>       //https://github.com/finitespace/BME280

#include "HTU2xD_SHT2x_Si70xx.h"

// SGP30 gas sensor
#include <SparkFun_SGP30_Arduino_Library.h>

// BME Sensor setup
#define PRESSURE_UNITS 5        // unit: B001 = hPa, B010 = inHg, 5 = mmHg
#define METRIC_UNITS true       // measurement units
#define SENSOR_UPD_PERIOD 5     // Update rate in seconds
#define SENSOR_DATA_BUFSIZE 25  // chars for sensors formatted data

// List of sensor types
//static constexpr char* S_na = "N/A";
static constexpr const char* T_BME280 = "BME280";
static constexpr const char* T_BMP280 = "BMP280";
static constexpr const char* T_Si702x = "Si702x";
static constexpr const char* T_SGP30 = "SGP30";

//Table of sensor names
//const char* const sensor_types[] PROGMEM = { sname_0, sname_1, sname_2, sname_3, sname_4 };

// sensors type enum
enum class sensor_t{
  bosch_bmx,
  si7021,
  sgp30
};

class GenericSensor {

protected:
  // textq receiver id to publish to
  uint8_t sink_id;
  // poll rate in seconds
  uint32_t poll_rate;
  // last poll tstamp
  uint32_t last_poll_tstamp;

  /**
   * @brief poll sensor and publish it's date to sink if needed
   * 
   */
  virtual void poll() = 0;

public:
  // sensor unique id
  const int32_t id;

  const sensor_t stype;

  GenericSensor(int32_t id, sensor_t stype) : id(id), stype(stype) {}

  // sensor's mnemonic description, i.e. "room", "outdoor" etc...
  std::string descr;

	/**
	 * @brief load configuration from a json object
	 * method should be implemented in derived class to process
	 * class specific json object
	 * @param cfg 
	 */
	virtual void load_cfg(JsonVariantConst cfg) = 0;

  /**
   * @brief initialize sensor
   * 
   * @return true if sensor found/operational
   * @return false if failed to init sensor
   */
  virtual bool init() = 0;


  /**
   * @brief poll sensor and publish it's date to sink if needed
   * executed once a second by sensor manager, will check sensor's
   * poll rate value and call poll() method when needed
   */
  void run();
};




class SensorManager : public GenericModule, public Task {
private:
  // initial id counter
  int32_t _snsr_id{0};

  // i2c bus
  int _i2c_scl{-1}, _i2c_sda{-1};

  BME280I2C _bosch;
  HTU2xD_SHT2x_SI70xx _si702x{HTU2xD_SHT2x_SI70xx(SI702x_SENSOR, HUMD_12BIT_TEMP_14BIT)}; //sensor type, resolution
  SGP30 sgp30;

  // detect flags
  bool _sgp_present{false}, _bosch_present{false}, _si702_present{false};
  sensor_t _bosch_model;

  // readings
 	float temp, pressure, humidity, dew = NAN;  // toffset = 0.0
  uint16_t co2, tvoc;


  void readbme280(float& t, float& h, float& p, float& dew);
  void readsi7021(float& t, float& h);

  void sgp30poll();
  void readsgp30(uint16_t &co2, uint16_t &tvoc, float rh, float t);

  void _spawn(sensor_t sensor);

  /**
   * @brief poll each sensor in a pool
   * 
   */
  void _poll_sensors();

public:
  SensorManager();
  //virtual ~Sensors();
  

  // pack class configuration into JsonObject
  //void generate_cfg(JsonVariant cfg) const override;

  // load class configuration from JsonObject
  void load_cfg(JsonVariantConst cfg) override;

  void start() override;
  void stop() override;


  bool getFormattedValues( char* str);
  bool getFormattedValues( String &str);

  //void getSensorModel(char* str);
  //void getSensorModel(String &str);

  static float RHtoAbsolute (float relHumidity, float tempC);
  static uint16_t doubleToFixedPoint( double number);

  // temp sensor offset get/set
  float tempoffset();
  float tempoffset(float t);

private:
  using sensor_pt = std::unique_ptr<GenericSensor>;
  // sensors modules container
  std::list<sensor_pt> _sensors;

};
