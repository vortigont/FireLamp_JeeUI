/*
LOG macro will enable/disable logs to serial depending on LAMP_DEBUG build-time flag
*/
#pragma once

#ifndef LAMP_DEBUG_PORT
#define LAMP_DEBUG_PORT Serial
#endif

// undef possible LOG macros
#ifdef LOG
  #undef LOG
#endif
#ifdef LOGV
  #undef LOGV
#endif
#ifdef LOGD
  #undef LOGD
#endif
#ifdef LOGI
  #undef LOGI
#endif
#ifdef LOGW
  #undef LOGW
#endif
#ifdef LOGE
  #undef LOGE
#endif

#if defined(LAMP_DEBUG_LEVEL) && LAMP_DEBUG_LEVEL == 5
	#define LOGV(tag, func, ...) LAMP_DEBUG_PORT.print(tag); LAMP_DEBUG_PORT.print(" V: "); LAMP_DEBUG_PORT.func(__VA_ARGS__)
#else
	#define LOGV(...)
#endif

#if defined(LAMP_DEBUG_LEVEL) && LAMP_DEBUG_LEVEL > 3
	#define LOGD(tag, func, ...) LAMP_DEBUG_PORT.print(tag); LAMP_DEBUG_PORT.print(" D: "); LAMP_DEBUG_PORT.func(__VA_ARGS__)
#else
	#define LOGD(...)
#endif

#if defined(LAMP_DEBUG_LEVEL) && LAMP_DEBUG_LEVEL > 2
	#define LOGI(tag, func, ...) LAMP_DEBUG_PORT.print(tag); LAMP_DEBUG_PORT.print(" I: "); LAMP_DEBUG_PORT.func(__VA_ARGS__)
	// compat macro
	#define LOG(func, ...) LAMP_DEBUG_PORT.func(__VA_ARGS__)
#else
	#define LOGI(...)
	// compat macro
	#define LOG(...)
#endif

#if defined(LAMP_DEBUG_LEVEL) && LAMP_DEBUG_LEVEL > 1
	#define LOGW(tag, func, ...) LAMP_DEBUG_PORT.print(tag); LAMP_DEBUG_PORT.print(" W: "); LAMP_DEBUG_PORT.func(__VA_ARGS__)
#else
	#define LOGW(...)
#endif

#if defined(LAMP_DEBUG_LEVEL) && LAMP_DEBUG_LEVEL == 1
	#define LOGE(tag, func, ...) LAMP_DEBUG_PORT.print(tag); LAMP_DEBUG_PORT.print(" E: "); LAMP_DEBUG_PORT.func(__VA_ARGS__)
#else
	#define LOGE(...)
#endif
