/**************************************************************************/
/*!
	@file Log.cpp
	@author dsync89 / dsync89@yahoo.com
	@version 1.0

	@brief
	Stores the properties of logs.

	Read README.txt for more information.

	Copyright 2015 - Under Creative Commons License 4.0:
					 Attribution-ShareAlike 4.0 International (CC BY-SA 4.0)

	For the full copyright and license information, please view the LICENSE
	file that was distributed with this source code.
/**************************************************************************/

#include "Log.h"

Log::Log(int startHour, int startMinute, int startSecond, int endHour, int endMinute, int endSecond) {

	this->startHour = startHour;
	this->startMinute = startMinute;
	this->startSecond = startSecond;
	this->endHour = endHour;
	this->endMinute = endMinute;
	this->endSecond = endSecond;
}

Log::Log() {
	Log(0,0,0,0,0,0);
}

const char* Log::getLogStartTime() {
	String str = String(startHour) + ":" + String(startMinute);
	return str.c_str();
	// return numToChar(startHour);
}

const char* Log::getLogEndtime() {
	String str = String(this->endHour) + ":" + String(this->endMinute);
	return str.c_str();
}

char* Log::numToChar(int number) {
	char chr[sizeof(number)];
	String str = String(number);
	str.toCharArray(chr, 5);
	return chr;
}

/**
  * Initialize a single log record
  */
void Log::initLogRecord() {
	this->startHour = 0;
	this->startMinute = 0;
	this->startSecond = 0;
	this->endHour = 0;
	this->endMinute = 0;
	this->endSecond = 0;	
	this->duration = 0;
}

