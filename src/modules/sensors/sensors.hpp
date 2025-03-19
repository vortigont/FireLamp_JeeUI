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
// SGP30 gas sensor
#include <SparkFun_SGP30_Arduino_Library.h>
//Si7021 sensors
#include "HTU2xD_SHT2x_Si70xx.h"


// sensors types enum
enum class sensor_t{
  na,           // not available
  bosch_bme,
  bosch_bmp,
  si7021,
  sgp30
};

class GenericSensor {

protected:
  sensor_t stype{sensor_t::na};

  // poll rate in seconds
  uint32_t poll_rate;
  // last poll tstamp
  uint32_t last_poll_tstamp;
  // shows if sensor's health is OK
  bool online{false};

  // sensor's mnemonic description, i.e. "room", "outdoor" etc...
  std::string descr;

  // textq receiver
  uint8_t scroller_id;
  uint32_t message_id;

public:
  // sensor unique id
  const int32_t id;

  GenericSensor(int32_t id) : message_id(random()), id(id) {}

	/**
	 * @brief load configuration from a json object
	 * method should be implemented in derived class to process
	 * class specific json object.
   * This is the first method that should called on derived class instantiation
	 * @param cfg 
	 */
	virtual void load_cfg(JsonVariantConst cfg) = 0;

  /**
   * @brief initialize sensor
   * this method should be called right after load_cfg()
   * 
   * @return true if sensor found/operational
   * @return false if failed to init sensor
   */
  virtual bool init() = 0;

  /**
   * @brief poll sensor and publish it's data to text sink if needed
   * executed once a second by sensor manager, will check sensor's
   * poll rate value and call poll() method when needed
   */
  void run();

  /**
   * @brief returns 'true' if sensor is initialized and operational
   * offline sensors are not polled by sensor manager
   * 
   * @return true 
   * @return false 
   */
  bool getState(){ return online; }

  // temporary enable/disable sensor
  void setState(bool state){ online = state; }

protected:

  /**
   * @brief poll sensor and publish it's data to sink if needed
   * 
   */
  virtual void poll() = 0;

};



/**
 * @brief Maintains a pool of known sensors
 * loads/unloads sensor configurations
 * 
 */
class SensorManager : public GenericModule, public Task {
private:
  // i2c bus
  int _i2c_scl{-1}, _i2c_sda{-1};

  /**
   * @brief poll each sensor in a pool
   * 
   */
  void _poll_sensors();

public:
  SensorManager();
  ~SensorManager();
  

  // pack class configuration into JsonObject
  void generate_cfg(JsonVariant cfg) const override {};

  // load class configuration from JsonObject
  void load_cfg(JsonVariantConst cfg) override;

  void start() override;
  void stop() override;


  //bool getFormattedValues( char* str);
  //bool getFormattedValues( String &str);

  //void getSensorModel(char* str);
  //void getSensorModel(String &str);

  // temp sensor offset get/set
  //float tempoffset();
  //float tempoffset(float t);

private:
  using sensor_pt = std::unique_ptr<GenericSensor>;
  // sensors modules container    TODO: this container need a locking when controled over asyncWS!
  std::list<sensor_pt> _sensors;

};


/**
 * @brief Bosch BMP/BME sensor
 * 
 */
class Sensor_Bosch : public GenericSensor {
  BME280I2C _bosch;
  // readings
 	float temp, humidity, pressure;

public:
  Sensor_Bosch(int32_t id) : GenericSensor(id) {}


	void load_cfg(JsonVariantConst cfg) override;

  bool init() override;

  void poll() override;


  static float RHtoAbsolute (float relHumidity, float tempC);
  static uint16_t doubleToFixedPoint( double number);

};

/**
 * @brief Bosch BMP/BME sensor
 * 
 */
class Sensor_SGP : public GenericSensor {
  uint32_t _pub_rate, _ctr{0};
  SGP30 _sensor;
  // readings

public:
  Sensor_SGP(int32_t id) : GenericSensor(id) {}
  ~Sensor_SGP();

	void load_cfg(JsonVariantConst cfg) override;

  bool init() override;

  void poll() override;
};

/**
 * @brief Si70xx sensor
 * 
 */
class Sensor_SiSHT : public GenericSensor {
  uint8_t _sensor_model{0};
  HTU2xD_SHT2x_SI70xx _sensor;
  // readings
 	float temp, humidity;

public:
  Sensor_SiSHT(int32_t id) : GenericSensor(id) {}

	void load_cfg(JsonVariantConst cfg) override;

  bool init() override;

  void poll() override;
};
