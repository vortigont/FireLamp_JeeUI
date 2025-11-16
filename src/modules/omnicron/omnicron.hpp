/*
	This file is a part of FireLamp_JeeUI project
	https://github.com/vortigont/FireLamp_JeeUI

	Copyright © 2023-2024 Emil Muratov (vortigont)

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

#pragma once

#include "cronos.hpp"
#include "modules/mod_manager.hpp"

class OmniCron : public GenericModule {

	struct omni_task_t {
		// task active state
		enum class active_t {
			disabled,		// task is not active
			enabled,		// task is active
			pwron			// task is axtive when device is on
		};

		active_t active;
		cronos_tid tid;
		std::string descr;
		std::string crontab;
		std::string cmd;
	};

	struct omni_cmd_t {
		cronos_tid id;
		int32_t cmd, param;
		omni_cmd_t (cronos_tid id, int32_t cmd, int32_t param) : id(id), cmd(cmd), param(param) {}
	};

	// scheduler object
	CronoS _cronos;

	// a set of available tasks to maintain
	std::vector<omni_task_t> _tasks;

	// actions chains
	std::vector<omni_cmd_t> _actions;

public:
	OmniCron();
	~OmniCron();

  void start() override;
  void stop() override;

	// load tasks from json file on FS
	void load_cfg(JsonVariantConst cfg) override final;

	// serialize cron configuration
	void generate_cfg(JsonVariant cfg) const override final;

private:

	// track device's power state
	bool _device_pwr{false};

	esp_event_handler_instance_t _hdlr_lmp_change_evt = nullptr;

	// change events handler
    void _lmpChEventHandler(esp_event_base_t base, int32_t id, void* data);

	void _cron_callback(cronos_tid id, void* arg);

	void _parse_actions(cronos_tid id, const char* expr);

	void _purge_actions(cronos_tid id);

	/**
	 * @brief Construct an EmbUI page with module's state/configuration
	 * 
	 * @param interf 
	 * @param data 
	 * @param action 
	 */
	void mkEmbUIpage(Interface *interf, JsonVariantConst data, const char* action) override;

	void _task_get(Interface *interf, JsonVariantConst data, const char* action);

	void _task_set(Interface *interf, JsonVariantConst data, const char* action);

	void _task_remove(Interface *interf, JsonVariantConst data, const char* action);

};

