//----------------------------------------------------
// мини-класс таймера, версия 1.0
#include <Arduino.h>

class timerMinim
{
  public:
	timerMinim() {_interval = 0;} // по дефолту - отключен

    timerMinim(uint32_t interval)				                  // объявление таймера с указанием интервала
    {
      _interval = interval;
      _timer = millis();
    }

    uint32_t getInterval()	                  						 // получение интервала работы таймера
    {
    	return _interval;
    }

    void setInterval(uint32_t interval)	                   // установка интервала работы таймера
    {
      _interval = interval;
    }

    bool isReady()						                             // возвращает true, когда пришло время. Сбрасывается в false сам (AUTO) или вручную (MANUAL)
    {
      if ((uint32_t)millis() - _timer >= _interval && _interval!=0){
        _timer = millis();
        return true;
      }
      else {
        return false;
      }
    }

    bool isReadyManual()                                   // возвращает true, когда пришло время. Без сбороса
    {
      if ((uint32_t)millis() - _timer >= _interval && _interval!=0){
        return true;
      }
      else {
        return false;
      }
    }

    void reset()							                              // ручной сброс таймера на установленный интервал
    {
      _timer = millis();
    }

  private:
    uint32_t _timer = 0;
    uint32_t _interval = 0;
};