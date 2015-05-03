/**************************************************************************/
/*!
	@file Log.h
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

#include <Arduino.h>

class Log
{
	public:
        Log(int startHour, int startMinute, int startSecond, int endHour, int endMinute, int endSecond);
        Log();
		const char* getLogStartTime();
		const char* getLogEndtime();
		char* numToChar(int number);
		void initLogRecord();

		int startHour;
		int startMinute;
		int startSecond;

		int endHour;
		int endMinute;
		int endSecond;

		int duration = 0;
};




