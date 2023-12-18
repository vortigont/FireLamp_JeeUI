#include "lamp.h"
#include "buttons.h"
#include "actions.hpp"
#include "evtloop.h"

const char *btn_get_desc(BA action){
	switch (action) {
		case BA_BRIGHT: return PSTR("BRIGHT");
		case BA_SPEED: return PSTR("SPEED");
		case BA_SCALE: return PSTR("SCALE");
		case BA_ON: return PSTR("ON");
		case BA_OFF: return PSTR("OFF");
		case BA_DEMO: return PSTR("DEMO");
		case BA_AUX_TOGLE: return PSTR("AUX");
		case BA_OTA: return PSTR("OTA");
		case BA_EFF_NEXT: return PSTR("NEXT");
		case BA_EFF_PREV: return PSTR("PREV");
		case BA_SEND_TIME: return PSTR("TIME");
		case BA_SEND_IP: return PSTR("IP");
		case BA_WHITE_HI: return PSTR("WHITE HI");
		case BA_WHITE_LO: return PSTR("WHITE LO");
		case BA_WIFI_REC: return PSTR("WIFI REC");
		case BA_EFFECT: return PSTR("EFFECT");
		default:;
	}
	return PSTR("");
}

Task *tReverseTimeout = nullptr; // задержка переключения направления
bool Button::activate(btnflags& flg, bool reverse){
		uint8_t newval;
		//RA ract = RA_UNKNOWN;
		ra act = ra::end;		// for transition period, let's make it a new var
		bool ret = false;
		if (reverse) flags.direction = !flags.direction;
		switch (action) {
			case BA_BRIGHT:
				newval = constrain(myLamp.getBrightness() + (myLamp.getBrightness() / 25 + 1) * (flags.direction * 2 - 1), 1 , 255);
				if ((newval == 1 || newval == 255) && tReverseTimeout==nullptr){
					tReverseTimeout = new Task(2 * TASK_SECOND, TASK_ONCE,
									[this](){ flags.direction = !flags.direction; LOG(println,"reverse"); },
									&ts, false, nullptr, [](){ tReverseTimeout = nullptr;}, true
							);
					tReverseTimeout->enableDelayed();
				} 
				// if (myLamp.getGaugeType()!=GAUGETYPE::GT_NONE){
				// 		GAUGE::GaugeShow(newval, 255, 10);
				// }
				run_action(ra::brt_nofade, newval);		// change brightness without fade effect
				return true;
			case BA_SPEED: {
				byte speed = (myLamp.effwrkr.getControls()[1]->getVal()).toInt();
				newval = constrain( speed + (speed / 25 + 1) * (flags.direction * 2 - 1), 1 , 255);
				if ((newval == 1 || newval == 255) && tReverseTimeout==nullptr){
					tReverseTimeout = new Task(2 * TASK_SECOND, TASK_ONCE,
									[this](){ flags.direction = !flags.direction; LOG(println,"reverse"); },
									&ts, false, nullptr, [](){ tReverseTimeout = nullptr;}, true
							);
					tReverseTimeout->enableDelayed();
				}
			/*	if (myLamp.getGaugeType()!=GAUGETYPE::GT_NONE){
						GAUGE::GaugeShow(newval, 255, 100);
				}*/
				run_action(String(T_effect_dynCtrl)+1, newval);
				return true;
			}
			case BA_SCALE: {
				byte scale = (myLamp.effwrkr.getControls()[2]->getVal()).toInt();
				newval = constrain(scale + (scale / 25 + 1) * (flags.direction * 2 - 1), 1 , 255);
				if ((newval == 1 || newval == 255) && tReverseTimeout==nullptr){
					tReverseTimeout = new Task(2 * TASK_SECOND, TASK_ONCE,
									[this](){ flags.direction = !flags.direction; LOG(println,"reverse"); },
									&ts, false, nullptr, [](){ tReverseTimeout = nullptr; }, true
							);
					tReverseTimeout->enableDelayed();
				}
			/*	if (myLamp.getGaugeType()!=GAUGETYPE::GT_NONE){
						GAUGE::GaugeShow(newval, 255, 150);
				}*/
				run_action(String(T_effect_dynCtrl)+2, newval);
				return true;
			}
			default:;
		}

		if((flg.onetime&2)) return ret; // если не установлен бит сработавшего однократного события - выходим

		// проверяем дальше
		switch (action) {
			case BA_ON:
				EVT_POST(LAMP_SET_EVENTS, e2int(evt::lamp_t::pwron));
				return ret;
			case BA_OFF:
				EVT_POST(LAMP_SET_EVENTS, e2int(evt::lamp_t::pwron));
				return ret;
			case BA_DEMO: run_action(ra::demo, !param.isEmpty()); return ret;
			case BA_AUX_TOGLE: run_action(ra::aux_flip); return ret;			// set AUX pin
			case BA_EFF_NEXT: run_action(ra::eff_next); return ret;
			case BA_EFF_PREV: run_action(ra::eff_prev); return ret;
			//case BA_SEND_TIME: myLamp.showTimeOnScreen(NULL); return ret;		// show time on screen
			//case BA_SEND_IP: ract = RA_SEND_IP; break;
			//case BA_WIFI_REC: ract = RA_WIFI_REC; break;
			case BA_EFFECT: { run_action(ra::eff_switch, param.toInt()); return ret; }
			default:;
		}

		//LOG(printf_P,PSTR("Button send action: %d\n"), act != ra::end ? static_cast<int>(act) : static_cast<int>(ract));
		if (act != ra::end)	return ret;	// уже отработали, выходим
/*
		if(param.isEmpty())
			remote_action(ract, NULL);
		else
			remote_action(ract, param.c_str(), NULL);
*/
		return ret;
}

String Button::getName(){
		String buffer;
		buffer.concat(flags.on? "ON: " : "OFF: ");
		if (flags.hold) {
			if (flags.click) {
				buffer.concat(String(flags.click));
				buffer.concat(" Click + ");
			}
			buffer.concat("HOLD - ");
		} else
		if (flags.click) {
			buffer.concat(String(flags.click));
			buffer.concat(" Click - ");
		}

		buffer.concat(btn_get_desc(action));

		return buffer;
};

Buttons::Buttons(uint8_t _pin, uint8_t _pullmode, uint8_t _state): buttons(), touch(_pin, _pullmode, _state){
	pin = _pin;
	pullmode = _pullmode;
	state = _state;
	holding = false;
	holded = false;
	buttonEnabled = true; // кнопка обрабатывается если true, пока что обрабатывается всегда :)
	pinTransition = true;
	onoffLampState = myLamp.isLampOn();

	clicks = 0;

	if (pullmode == LOW_PULL)
		pinMode(pin, INPUT);
	else
		pinMode(pin, INPUT_PULLUP);

	touch.setType(pullmode);
	touch.setTickMode(MANUAL);    // мы сами говорим когда опрашивать пин
	touch.setStepTimeout(BUTTON_STEP_TIMEOUT);
	touch.setClickTimeout(BUTTON_CLICK_TIMEOUT);
	touch.setTimeout(BUTTON_TIMEOUT);
	touch.setDebounce(BUTTON_DEBOUNCE);   // т.к. работаем с прерываниями, может пригодиться для железной кнопки
	touch.resetStates();

	attachInterrupt(pin, std::bind(&Buttons::isrPress,this), pullmode!=LOW_PULL ? RISING : FALLING );
	isrEnable();
}

void Buttons::buttonTick(){
	if (!buttonEnabled) return;

	touch.tick();
	bool reverse = false;

	if ((holding = touch.isHolded())) {
		// начало удержания кнопки
		byte tstclicks = touch.getHoldClicks();
		if(!tClicksClear || (tstclicks && tstclicks!=clicks)) // нажатия после удержания не сбрасываем!!! они сбросятся по tClicksClear или по смене кол-ва нажатий до удержания
			clicks=tstclicks;
		if(!tClicksClear){
			tClicksClear = new Task(NUMHOLD_TIME, TASK_ONCE, [this](){ holded = true; clicks=0; }, &ts, false, nullptr, [this](){tClicksClear=nullptr;}, true);
			tClicksClear->enableDelayed();
		}
		onoffLampState = myLamp.isLampOn(); // получить статус на начало удержания
		reverse = true;
		if(tReverseTimeout){ // сброс реверса, если он включен
			LOG(println,"reverce canceled");
			tReverseTimeout->cancel();
		}
		LOG(printf_P, PSTR("start hold - buttonEnabled=%d, onoffLampState=%d, holding=%d, holded=%d, clicks=%d, reverse=%d\n"), buttonEnabled, onoffLampState, holding, holded, clicks, reverse);
	} else if ((holding = touch.isStep())) {
		// кнопка удерживается
		if(tClicksClear)
			tClicksClear->restartDelayed(); // отсрочиваем сброс нажатий
	} else if (!touch.hasClicks() || !(clicks = touch.getClicks())) {
		if( (!touch.isHold() && holded) )	{ // кнопку уже не трогают
			LOG(println,"Сброс состояния кнопки после окончания удержания");
			resetStates();
			onoffLampState = myLamp.isLampOn(); // сменить статус после удержания
			LOG(printf_P, PSTR("reset - buttonEnabled=%d, onoffLampState=%d, holding=%d, holded=%d, clicks=%d, reverse=%d\n"), buttonEnabled, onoffLampState, holding, holded, clicks, reverse);
			for (unsigned i = 0; i < buttons.size(); i++) {
				buttons[i]->flags.onetime&=1;
			}
			isrEnable(); // переключение на ленивый опрос
		}
		// здесь баг, этот выход часто перехватывает "одиночные" нажатия и превращает их в "клик"
		return;
	}
	
	if (myLamp.isAlarm()) {
		// нажатие во время будильника
		// ALARMTASK::stopAlarm();
		return;
	}

	if(!holding){
		onoffLampState=myLamp.isLampOn(); // обновить статус, если не удерживается и это однократное нажатие
		LOG(printf_P, PSTR("onetime click - buttonEnabled=%d, onoffLampState=%d, holding=%d, holded=%d, clicks=%d, reverse=%d\n"), buttonEnabled, onoffLampState, holding, holded, clicks, reverse);
	}
	
	Button btn(onoffLampState, holding, clicks, true); // myLamp.isLampOn() - анализироваться будет состояние на начало нажимания кнопки
	for (unsigned i = 0; i < buttons.size(); i++) {
		if (btn == *buttons[i]) {
			//if(buttons[i]->action==1) continue; // отладка, отключить действие увеличения яркости
			if (!buttons[i]->activate(buttons[i]->flags, reverse)) {
				//LOG(println,buttons[i]->action); // отладка
				// действие не подразумевает повтора
				if(buttons[i]->flags.onetime && touch.isHold()){ // в процессе удержания
					buttons[i]->flags.onetime|=3; // установить старший бит сработавшего действия
				}
			}
			// break; // Не выходим после первого найденного совпадения. Можем делать макросы из нажатий
		}
	}

	// Здесь уже все отработало, и кнопка точно не удерживается
	if(!holding){
		LOG(println,"Сброс состояния кнопки");
		resetStates();
		onoffLampState = myLamp.isLampOn(); // обновить статус по итогу работы
		isrEnable();
	}
}

void Buttons::clear() {
	while (buttons.size()) {
		Button *btn = buttons.shift();
		delete btn;
	}
}

int Buttons::loadConfig(const char *cfg){
	if (LittleFS.begin()) {
		File configFile;
		if (cfg == nullptr) {
			LOG(println, "Load default buttons config file");
			configFile = LittleFS.open("/buttons_config.json", "r"); // PSTR("r") использовать нельзя, будет исключение!
		} else {
			LOG(printf_P, PSTR("Load %s buttons config file\n"), cfg);
			configFile = LittleFS.open(cfg, "r"); // PSTR("r") использовать нельзя, будет исключение!
		}
		String cfg_str = configFile.readString();
		if (cfg_str.isEmpty()){
			LOG(println, "Failed to open buttons config file");
			return 0;
		}

		DynamicJsonDocument doc(2048);
		DeserializationError error = deserializeJson(doc, cfg_str);
		if (error) {
			LOG(print, "deserializeJson error: ");
			LOG(println, error.code());
			LOG(println, cfg_str);
			return 0;
		}
		JsonArray arr = doc.as<JsonArray>();
		for (size_t i = 0; i < arr.size(); i++) {
			JsonObject item = arr[i];
			uint8_t mask = item["flg"].as<uint8_t>();
			BA ac = (BA)item["ac"].as<int>();
			if(item.containsKey("p")){
				String param = item["p"].as<String>();
				buttons.add(new Button(mask, ac, param));
			} else {
				buttons.add(new Button(mask, ac));
			}
		}
		doc.clear();
	}
	return 1;
}

void Buttons::saveConfig(const char *cfg){
	if (LittleFS.begin()) {
		File configFile;
		if (cfg == nullptr) {
			LOG(println, "Save default buttons config file");
			configFile = LittleFS.open("/buttons_config.json", "w"); // PSTR("w") использовать нельзя, будет исключение!
		} else {
			LOG(printf_P, PSTR("Save %s buttons config file\n"), cfg);
			configFile = LittleFS.open(cfg, "w"); // PSTR("w") использовать нельзя, будет исключение!
		}
		configFile.print("[");

		for (unsigned i = 0; i < buttons.size(); i++) {
			Button *btn = buttons[i];
			configFile.printf_P(PSTR("%s{\"flg\":%u,\"ac\":%u,\"p\":\"%s\"}"),
				(char*)(i? "," : ""), btn->flags.mask, btn->action, btn->getParam().c_str()
			);
			LOG(printf_P, PSTR("%s{\"flg\":%u,\"ac\":%u,\"p\":\"%s\"}"),
				(char*)(i? "," : ""), btn->flags.mask, btn->action, btn->getParam().c_str()
			);
		}
		configFile.print("]");
		configFile.flush();
		configFile.close();
	}
}

void IRAM_ATTR Buttons::isrPress() {
  detachInterrupt(pin);
	// какой нерюх создает новый объект в ISR???
	// todo: убрать
	if(tButton)
		tButton->restartDelayed();
	else
		tButton = new Task(20, TASK_FOREVER, std::bind(&Buttons::buttonTick, this), &ts, true, nullptr, [this](){ tButton=nullptr; }, true); // переключение в режим удержания кнопки
}

void Buttons::isrEnable(){
	LOG(println,"Button switch to isr");
	attachInterrupt(pin, std::bind(&Buttons::isrPress,this), pullmode==LOW_PULL ? RISING : FALLING );
	if(tButton)
		tButton->restartDelayed();
	else
		tButton = new Task(TASK_SECOND, 5, std::bind(&Buttons::buttonTick, this), &ts, true, nullptr, [this](){ tButton=nullptr;}, true);	// "ленивый" опрос 1 раз в сек в течение 5 секунд
}

void Buttons::setButtonOn(bool flag) {
	buttonEnabled = flag;
	resetStates();
	if (flag){
		LOG(println,"Button watch enabled");
		isrEnable();
	} else {
	  detachInterrupt(pin);
		if(tButton)
			tButton->cancel();
		LOG(println,"Button watch disabled");
	}
}
