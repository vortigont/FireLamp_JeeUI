/*
LOG macro will enable/disable logs to serial depending on LAMP_DEBUG build-time flag
*/
#pragma once

#if defined(LOG)
  #undef LOG
#endif
#if defined(LAMP_DEBUG)
	#define LOG(func, ...) Serial.func(__VA_ARGS__)
#else
	#define LOG(func, ...) ;
#endif
