/*
Copyright © 2020 Dmytro Korniienko (kDn)
JeeUI2 lib used under MIT License Copyright (c) 2019 Marsel Akhkamov

    This file is part of FireLamp_JeeUI.

    FireLamp_JeeUI is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FireLamp_JeeUI is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FireLamp_JeeUI.  If not, see <https://www.gnu.org/licenses/>.

  (Этот файл — часть FireLamp_JeeUI.

   FireLamp_JeeUI - свободная программа: вы можете перераспространять ее и/или
   изменять ее на условиях Стандартной общественной лицензии GNU в том виде,
   в каком она была опубликована Фондом свободного программного обеспечения;
   либо версии 3 лицензии, либо (по вашему выбору) любой более поздней
   версии.

   FireLamp_JeeUI распространяется в надежде, что она будет полезной,
   но БЕЗО ВСЯКИХ ГАРАНТИЙ; даже без неявной гарантии ТОВАРНОГО ВИДА
   или ПРИГОДНОСТИ ДЛЯ ОПРЕДЕЛЕННЫХ ЦЕЛЕЙ. Подробнее см. в Стандартной
   общественной лицензии GNU.

   Вы должны были получить копию Стандартной общественной лицензии GNU
   вместе с этой программой. Если это не так, см.
   <https://www.gnu.org/licenses/>.)
*/

#include "enc.h"
#ifdef ENCODER

#include "tm1637display.hpp"
#ifdef DS18B20
#include "DS18B20.h"
#endif

#include "lamp.h"
#include "actions.hpp"
#include "evtloop.h"

uint8_t currDynCtrl;        // текущий контрол, с которым работаем
uint8_t currAction;         // идент текущей операции: 0 - ничего, 1 - крутим яркость, 2 - меняем эффекты, 3 - меняем динамические контролы
uint16_t currEffNum;        // текущий номер эффекта
uint16_t anyValue;          // просто любое значение, которое крутим прямо сейчас, очищается в enc_loop
uint8_t loops;              // счетчик псевдотаймера
bool done;                  // true == все отложенные до enc_loop операции выполнены.
bool inSettings;            // флаг - мы в настройках эффекта
uint8_t speed, fade;

uint8_t txtDelay = 40U;
CRGB txtColor = CRGB::Orange;

struct triggers_t {
  volatile bool turn;
  volatile bool click;
  volatile bool clicks;
  volatile bool hold;
};

// сюда собираем события
triggers_t trig;

/**
 * @brief a callback for encoder's object that is triggered back from ISR
 * let's make it as short as possible
 */
void IRAM_ATTR cb_turn(){
  trig.turn = true;
}
void IRAM_ATTR cb_click(){
  trig.click = true;
}
void IRAM_ATTR cb_clicks(){
  trig.clicks = true;
}
void IRAM_ATTR cb_hold(){
  trig.hold = true;
}

/**
 * @brief запускаем этот вызов из петли для отработки накопленых событий 
 * есть вероятность что-то пропустить если прерывание прилетит во время колбека,
 * но с учетом того что альтернатива тупо падать считаем риск приемлимым пока здесь
 * не появится нормальный обработчик энкодера
 */
void callback_runner(){
  if (trig.turn){ isTurn(); trig.turn = false; }
  if (trig.click){ isClick(); trig.click = false; }
  if (trig.clicks){ myClicks(); trig.clicks = false; }
  if (trig.hold){ isHolded(); trig.hold = false; }
}

void encLoop() {
  static uint16_t valRepiteChk = anyValue;
  //noInterrupt();
  EVERY_N_MILLIS(100){
    enc.tick();   // lazy calls
  }
  callback_runner();

  if (inSettings) { // Время от времени выводим название контрола (в режиме "Настройки эффекта")
    resetTimers();
    //if (tm1637) tm1637->getSetDelay() = TM_TIME_DELAY;
    EVERY_N_SECONDS(10) {
      loops++;
      if (inSettings && loops == ENC_EXIT_TIMEOUT) {  // timeout выхода из режима "Настройки эффекта"
        exitSettings();
        return;
      }
      //encSendString(myLamp.getEffControls()[currDynCtrl]->getName(), txtColor, false, txtDelay); 
    }
  }
#ifdef DS18B20
  else {
    EVERY_N_SECONDS(1) {  // раз в секунду проверяем возможность выводить температуру
      canDisplayTemp() = done;
    }
  }
#endif

/*
*     Оперативные изменения, яркость например, чтобы было видно что происходит
*/
  if (currAction != 2) {
    if (valRepiteChk != (currAction == 3 ? myLamp.getEffControls()[currDynCtrl]->getVal().toInt() : anyValue) && !done) {  // проверим менялось ли значение, чтобы не дергать почем зря 
      valRepiteChk = (currAction == 3 ? myLamp.getEffControls()[currDynCtrl]->getVal().toInt() : anyValue);
      switch (currAction)
      {
      case 1: // регулировка яркости
 				run_action(ra::brt_nofade, anyValue);		// change brightness without fade effect
        done = true;
        break;
      case 3: // регулировка любого из динамических контролов в режиме "Настройки эффекта"
      	run_action(String(T_effect_dynCtrl)+myLamp.getEffControls()[currDynCtrl]->getId(), myLamp.getEffControls()[currDynCtrl]->getVal());
        done = true;
      break;
      default:
        break;
      } 
    }
  } else
/*
*     Отложенные операция смены эффекта
*/
  {
    EVERY_N_MILLIS(150) {
      static bool printed = false;
      if (valRepiteChk == currEffNum) {
        if (!printed) {
          //encSendStringNumEff(currEffNum <= 255 ? String(currEffNum) : (String((byte)(currEffNum & 0xFF)) + "." + String((byte)(currEffNum >> 8) - 1U)), txtColor);
          printed = true;
        }
      } 
      else {
        valRepiteChk = currEffNum;
        printed = false;
      }
    }
    if (!done && digitalRead(ENC_SW)) { // если эффект еще не меняли и кнопка уже отпущена - переключаем эффект
      resetTimers();
      LOG(printf_P, PSTR("Enc: New effect number: %d\n"), currEffNum);
      myLamp.switcheffect(effswitch_t::num, currEffNum);
      //run_action(ra::eff_switch, myLamp.effwrkr.getSelected());
      //encSendString(String(TINTF_00A) + ": " + (currEffNum <= 255 ? String(currEffNum) : (String((byte)(currEffNum & 0xFF)) + "." + String((byte)(currEffNum >> 8) - 1U))), txtColor, true, txtDelay);
      done = true;
      currAction = 0;
    }
  }
  //interrupt();
}

// Обработчик прерываний
void IRAM_ATTR isrEnc() { 
  //noInterrupt();
  enc.tick();  // отработка в прерывании
  //interrupt();
}

// Функция восстанавливает прерывания энкодера
void interrupt() {
  attachInterrupt(digitalPinToInterrupt(ENC_DT), isrEnc, CHANGE);   // прерывание на DT пине
  attachInterrupt(digitalPinToInterrupt(ENC_CLK), isrEnc, CHANGE);  // прерывание на CLK пине
  attachInterrupt(digitalPinToInterrupt(ENC_SW), isrEnc, FALLING);   // прерывание на SW пине
}

// Функция запрещает прерывания от энкодера, на время других операций, чтобы не спамить контроллер
void noInterrupt() {
  detachInterrupt(ENC_DT);
  detachInterrupt(ENC_CLK);
  detachInterrupt(ENC_SW);
}

// Функция обрабатывает повороты энкодера
void isTurn() {
  if (!myLamp.isLampOn()) return;
  //noInterrupt();
  resetTimers();
  uint8_t turnType = 0;

  // тут опрос эвентов с энкодера Right, Left, etc.
    if (enc.isLeft()) {
        if (enc.isFast()) {turnType = 2; }  // Fast left
        else {turnType = 1; } // Left
    } else if (enc.isLeftH()) {
        if (enc.isFast()) {turnType = 4; }  // Fast left hold
        else {turnType = 3; }  // Hold left
    } else 
    if (enc.isRight()) {
        if (enc.isFast()) {turnType = 6; }  // Fast right
        else {turnType = 5; }  // Right
    } else if (enc.isRightH()) {
        if (enc.isFast()) {turnType = 8; }  // Fast right hold
        else {turnType = 7; } // Hold right
    }

  switch (turnType)
  {
  case 1: // Влево 
  case 2: // Влево быстро
    if (inSettings) {
      encSetDynCtrl(turnType == 1 ? -1 : -10);
    } 
    else 
      encSetBri(turnType == 1 ? -1 : -10);
    break;
  case 3: // нажатый влево 
    encSetEffect(-1);
    break;
  case 4: // влево нажатый и быстро
    encSetEffect(-5);
    break;
  case 5: // Вправо  
  case 6:  // Вправо быстро  
    if (inSettings) {
      encSetDynCtrl(turnType == 5 ? 1 : 10);
    } 
    else 
      encSetBri(turnType == 5 ? 1 : 10);
    break;
  case 7: // вправо нажатый 
    encSetEffect(1);
    break;
  case 8: // вправо нажатый и быстро
    encSetEffect(5);
    break;
  
  default:
    break;
  }
  LOG(printf_P, PSTR("Enc: Turn type: %d\n"), turnType);
  //interrupt();
}

// Функция обрабатывает клики по кнопке
void isClick() {
  //noInterrupt();
  resetTimers();
/*
  if (!inSettings) encDisplay(enc.clicks, String("CL."));
  else {
    enc.clicks = 0;
    resetTimers();
    loops = 0;
    while (1) {
      currDynCtrl ++;
      if (currDynCtrl == myLamp.getEffControls().size()) {
        currDynCtrl = 1;
        break;
      }


      if (validControl(myLamp.getEffControls()[currDynCtrl]->getType())) break;
    }
    //encDisplay(myLamp.getEffControls()[currDynCtrl]->getVal().toInt(), String(myLamp.getEffControls()[currDynCtrl]->getId()) + ".");
    //encSendString(myLamp.getEffControls()[currDynCtrl]->getName(), txtColor, true, txtDelay);  
  }
*/
  //interrupt();
}

// Функция проверяет может ли контрол быть использоваан (проверка на скрытость, на скрытость по микрофону и т.п.)
bool validControl(const CONTROL_TYPE ctrlCaseType) {
  bool isOk = false;
#ifdef MIC_EFFECTS
  bool isMicOn = myLamp.isMicOnOff();
  if (myLamp.getEffControls()[myLamp.getEffControls().size()-1]->getName().startsWith(TINTF_020))
    isMicOn = isMicOn && myLamp.getEffControls()[myLamp.getEffControls().size()-1]->getVal().toInt();
#endif

  switch (ctrlCaseType & 0x0F) {
    case CONTROL_TYPE::RANGE:     // мы меняем только ползунки или чекбоксы
    case CONTROL_TYPE::CHECKBOX:
      isOk = true;
      break;
    default:
      isOk = false;
      break;
    }

    switch (ctrlCaseType >> 4) 
    {
    case CONTROL_CASE::HIDE: // Если спрятанный контрол, возвращаем ложь.
      return false;
      break;
#ifdef MIC_EFFECTS
    case CONTROL_CASE::ISMICON: // проверка "спрятан по микрофону"
      if (!isMicOn && (!myLamp.isMicOnOff() || !(myLamp.getEffControls()[currDynCtrl]->getId() == 7 && myLamp.getEffControls()[currDynCtrl]->getName().startsWith(TINTF_020) == 1)))
        isOk = false;
      break;
#endif
    default:
      break;
    }
    
  return isOk;
}

// Функция обрабатывает состояние "кнопка нажата и удержана"
void isHolded() {
  //noInterrupt();
  LOG(printf_P, PSTR("Enc: Pressed and holded\n"));
  
  if (!inSettings) {
    inSettings = true;
    resetTimers();
    loops = 0;
#ifdef DS18B20
    canDisplayTemp() = false; // в режиме "Настройки Эффекта" запрещаем выводить температуру, в функции exitSettings() снова разрешим
    currEffNum = myLamp.effwrkr.getCurrent();
    LOG(printf_P, PSTR("Enc: Effect number: %d controls amount %d\n"), currEffNum, myLamp.getEffControls().size());
#endif
    //encSendString(String(TINTF_01A), CRGB::Green, true, txtDelay);
    //encDisplay(myLamp.getEffControls()[currDynCtrl]->getVal().toInt(), String(currDynCtrl) + String("."));
    //encSendString(myLamp.getEffControls()[currDynCtrl]->getName(), txtColor, false, txtDelay);
  } else {
      exitSettings();
  }
  //interrupt();
}

// Функция выхода из режима "Настройки эффекта", восстанавливает состояния до, форсирует запись конфига эффекта
void exitSettings() {
  if (!inSettings) return;
  //noInterrupt();
  currDynCtrl = 1;
  done = true;
  loops = 0;
  currAction = 0;
  anyValue = 0;
  inSettings = false;
  //encDisplay(String("done"));
  //encSendString(String(TINTF_exit), CRGB::Red, true, txtDelay);
  myLamp.effwrkr.autoSaveConfig();
#ifdef DS18B20
  canDisplayTemp() = true;
#endif
  LOG(printf_P, PSTR("Enc: exit Settings\n"));
  //interrupt();
}

// Функция обрабатывает клики по кнопке
void myClicks() {
  //noInterrupt();
  resetTimers();
	if (myLamp.isAlarm()) {
		// нажатие во время будильника
    enc.clicks = 0;
		return;
	}
  
  switch (enc.clicks)
  {
  case 1: // Включение\выключение лампы
    if (myLamp.isLampOn()) {
      EVT_POST(LAMP_SET_EVENTS, e2int(evt::lamp_t::pwroff));
    } else {
      EVT_POST(LAMP_SET_EVENTS, e2int(evt::lamp_t::pwron));
    }
    break;
#ifdef ENC_DEMO_CLICK
  case ENC_DEMO_CLICK:  // Вкл\выкл Демо
    toggleDemo();
    break;
#endif
#ifdef ENC_GBRI_CLICK
  case ENC_GBRI_CLICK:
    toggleGBright();
    break;
#endif
#if defined(MIC_EFFECTS) && defined(ENC_MIC_CLICK)
  case ENC_MIC_CLICK:
    toggleMic();
    break;
#endif
/*
#ifdef ENC_IP_CLICK
  case ENC_IP_CLICK:
    sendIP();
    break;
#endif
*/
#ifdef ENC_TIME_CLICK
  case ENC_TIME_CLICK:
    sendTime();
    break;
#endif
#if defined(ENC_AUX_CLICK)
  case ENC_AUX_CLICK:
    toggleAUX();
    break;
#endif
  default:
    LOG(printf_P, PSTR("Enc: Click: %d\n"), enc.clicks);
    break;
  }
  //interrupt();
}

// Поместить в общий setup()
void enc_setup() {
  currAction = 0; // id операции, котору нужно выпонить в enc_loop
  currEffNum = 0;
  anyValue = 0; // просто любое значение, которое крутить прямо сейчас, очищается в enc_loop
  done = true; // true == все отложенные до enc_loop операции выполнены.
  loops = 0;
  inSettings = false;
  currDynCtrl = 1;
  //enc.counter = 100;      // изменение счётчика
  enc.attach(TURN_HANDLER, cb_turn);
  enc.attach(CLICK_HANDLER, cb_click);
  enc.attach(HOLDED_HANDLER, cb_hold);
 // enc.attach(STEP_HANDLER, myStep);
  enc.attach(CLICKS_HANDLER, cb_clicks);
  //enc.attachClicks(6, isClick6);
  interrupt(); // включаем прерывания энкодера и кнопки
}

// Функция регулировки яркости в обычном режиме
void encSetBri(int val) {
  resetTimers();

  if (done or currAction !=1) { 
    anyValue = myLamp.getBrightness();
    done = false;
  }
  if (currAction == 2) {
    currAction = 1; // сменим мод, но на вылет. Крутить яркостью будем в следующем цикле. Так уменьшим количество ошибок юзера, когда при отпускании энкодера он проворачивает его.
    return;
  }
  currAction = 1;
  anyValue = constrain(anyValue + val, 1, 255);
/*   if (myLamp.getGaugeType()!=GAUGETYPE::GT_NONE){
      GAUGE::GaugeShow(anyValue, 255);
  }
 */  //encDisplay(anyValue, String("b."));
}

// Функция смены эффекта зажатым энкодером
void encSetEffect(int val) {
  noInterrupt();
  resetTimers();
  if (inSettings) { // если в режиме "Настройки эффекта" выходим из него
    exitSettings();
    return;
  }

  if (done or currAction !=2) { // если сеттер отработал или предыдущий мод не отвечает текущему, перечитаем значение, и взведем сеттер
    anyValue = myLamp.effwrkr.effIndexByList(myLamp.effwrkr.getCurrent());
    done = false;
    //encDisplay(anyValue, "");
  }

  currAction = 2;

  anyValue = anyValue + val;
  
  while (1)  // в цикле проверим может быть текущий накрученный выбранным
  {
    if (myLamp.effwrkr.effCanBeSelected(anyValue)) break;

    if (val > 0) { // если курутили вперед по списку - скипим в том же направлении, если назад - в обратном
      anyValue++; 
      if(anyValue >= myLamp.effwrkr.getEffectsListSize()) // если ничего не нашли, - снова начинаем сначала
        anyValue = 0;
    }
    else {
      anyValue--;
      if (anyValue == 0) // если ничего не нашли, - снова начинаем с конца
        anyValue = myLamp.effwrkr.getEffectsListSize()-1;
    }
  }
  currEffNum = myLamp.effwrkr.realEffNumdByList(anyValue);
  //encDisplay(currEffNum <= 255 ? String(currEffNum) : (String((byte)(currEffNum & 0xFF)) + "." + String((byte)(currEffNum >> 8) - 1U)));  
  interrupt();
}

// Функция настройки динамического контрола в режиме "Настройки эффекта"
void encSetDynCtrl(int val) {
  noInterrupt();
  resetTimers();
  loops = 0;
  
  if (done or currAction !=3) { 
    done = false;
  }
  currAction = 3;
  // тут магия, некоторые чекбоксы у нас особенные, типа локальный "Микрофон". 
  // Придется проверять что это - ползунок или чекбокс и по разному подходить к процессу внесения нового значения. Бля...
  if ((myLamp.getEffControls()[currDynCtrl]->getType() & 0x0F) == 0) // если ползунок
    myLamp.getEffControls()[currDynCtrl]->setVal(String(myLamp.getEffControls()[currDynCtrl]->getVal().toInt() + val));
  else // если чекбокс
    myLamp.getEffControls()[currDynCtrl]->setVal(String(constrain(myLamp.getEffControls()[currDynCtrl]->getVal().toInt() + val, 0, 1)));
/*
  if ((myLamp.getEffControls()[currDynCtrl]->getType() & 0x0F) == 2) encSendString(myLamp.getEffControls()[currDynCtrl]->getName() + String(myLamp.getEffControls()[currDynCtrl]->getVal().toInt() ? ": ON" : ": OFF"), txtColor, true, txtDelay); 
  else if (myLamp.getGaugeType()!=GAUGETYPE::GT_NONE){
      GAUGE::GaugeShow(myLamp.getEffControls()[currDynCtrl]->getVal().toInt(), myLamp.getEffControls()[currDynCtrl]->getMax().toInt());
  }
*/
  //encDisplay(myLamp.getEffControls()[currDynCtrl]->getVal().toInt(), String(myLamp.getEffControls()[currDynCtrl]->getId()) + String("."));
  interrupt();
}

/*
void encDisplay(uint16_t value, String type) {
  if (tm1637) {
    tm1637->getSetDelay() = TM_TIME_DELAY;
    tm1637->display(value, true, false, value >= 100 ? 1 : (value >= 10 ? 2 : 3) );
    tm1637->display(type);
  }

}

void encDisplay(String str) {
  if (tm1637) {
    //tm1637->getSetDelay() = TM_TIME_DELAY;
    tm1637->clearScreen();
    tm1637->display(str);
  }
}
*/
// Ресетим таймера автосохранения конфигов и Демо на время "баловства" с энкодером 
void resetTimers() {
  myLamp.demoTimer(T_RESET);
  myLamp.effwrkr.autoSaveConfig();
  embui.autosave();
}
/*
// Функция выводит информацию, с помощью бегущей строки
void encSendString(String str, CRGB color, bool force, uint8_t delay) {
  fade = myLamp.getBFade();
  myLamp.setBFade(FADETOBLACKVALUE);
  myLamp.setTextMovingSpeed(delay);
  speed = myLamp.getTextMovingSpeed();
  myLamp.sendStringToLamp(str.c_str(), color, false, force);
  myLamp.setBFade(fade);
  myLamp.setTextMovingSpeed(speed);
  
}

void encSendStringNumEff(String str, CRGB color) {
  fade = myLamp.getBFade();
  myLamp.setBFade(FADETOBLACKVALUE);
  myLamp.setTextMovingSpeed(ENC_STRING_EFFNUM_DELAY);
  speed = myLamp.getTextMovingSpeed();
  String tmp = " " + str + " ";
  myLamp.sendStringToLampDirect(tmp.c_str(), color, false, true);
  myLamp.setBFade(fade);
  myLamp.setTextMovingSpeed(speed);
}
*/

void toggleDemo() {
  if (myLamp.getMode() == LAMPMODE::MODE_DEMO) {
    run_action(ra::demo, false);
    //encSendString(String("Demo OFF"), txtColor, true, txtDelay);
  }
  else 
    run_action(ra::demo, true);
}
/*
void toggleGBright() {
  run_action(ra::brt_global, myLamp.IsGlobalBrightness());
  encSendString(String(TINTF_00C) + myLamp.IsGlobalBrightness() ? ": ON" : ": OFF", txtColor, true, txtDelay);
}
*/
void toggleMic() {
#ifdef MIC_EFFECTS
  run_action(ra::miconoff, myLamp.isMicOnOff());
  //encSendString(String(TINTF_021) + String(myLamp.isMicOnOff() ? ": ON" : ": OFF"), txtColor, true, txtDelay);
#endif
}

void toggleAUX() {
  if ( embui.paramVariant(TCONST_aux_gpio) == -1) return;
  run_action(ra::aux_flip);
  //encSendString(String(TCONST_AUX) + digitalRead(embui.paramVariant(TCONST_aux_gpio)) == embui.paramVariant(TCONST_aux_ll) ? ": ON" : ": OFF", txtColor, true, txtDelay);
}

void sendTime() {
  myLamp.showTimeOnScreen(NULL);
}
/*
void sendIP() {
  remote_action(RA::RA_SEND_IP, NULL);
  if (tm1637) tm1637->showip();
}
*/

uint8_t getEncTxtDelay(){ return txtDelay;}
void setEncTxtDelay(const uint8_t speed){ txtDelay = speed;}
CRGB getEncTxtColor(){ return txtColor;}
void setEncTxtColor(const CRGB color){ txtColor = color;}

// экземпляр объекта энкодер с кнопкой <A, B, KEY>
EncButton<EB_CALLBACK, ENC_DT, ENC_CLK, ENC_SW> enc;
#endif
