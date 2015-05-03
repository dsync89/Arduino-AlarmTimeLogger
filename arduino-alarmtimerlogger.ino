/**************************************************************************/
/*!
	@file arduino-alarmtimerlogger.ino
	@author dsync89 / dsync89@yahoo.com
	@version 1.0
	@date May 2, 2015

	@brief
	An implementation of ALARM, TIMER, and LOGGER (A.T.Logger) for the
	Arduino microcontroller. 

	Read README.txt for more information.

	Copyright 2015 - Under Creative Commons License 4.0:
					 Attribution-ShareAlike 4.0 International (CC BY-SA 4.0)	

	For the full copyright and license information, please view the LICENSE
	file that was distributed with this source code.
/**************************************************************************/

#include <LiquidCrystal_I2C.h> 
#include <Keypad.h>
#include <Wire.h>
#include "Log.h"

#include "tone_pitches.h" // for Nyan Cat and Tetris

// Arduino version compatibility Pre-Compiler Directives
#if defined(ARDUINO) && ARDUINO >= 100   // Arduino v1.0 and newer
  #define I2C_WRITE Wire.write 
  #define I2C_READ Wire.read
#else                                   // Arduino Prior to v1.0 
  #define I2C_WRITE Wire.send 
  #define I2C_READ Wire.receive
#endif

#define ON true
#define OFF false


//---------------------------------------------------------------------------------------------######
/**----------------------------------------------------------------------
 |	USER DEFINABLE CONSTANTS
 ----------------------------------------------------------------------**/

// Display
#define MSG_DISPLAY_PAUSE_INTERVAL 500 // time to wait when showing the message on each key press
#define LCD_PRINT_TIME_SECOND // whether to print the seconds in main screen (NOT YET IMPLEMENTED!)

bool lcdBacklightOn = true; // turn on LCD backlight by default, can turn it off in Main Menu

// Time Display
bool dispAMPM = false; // whether to display the time in 24 hours format, i.e. 23:59
bool showSeconds = true; // whether to show the seconds in the Main Screen

// Countdown timer
bool autoStartCountdownTimer = true; // whether to automatically start the countdown when timer is set

byte repeatCount = 0; // how many times to repeat, 0: Infinite

// Snooze
int snoozeDuration = 300; // default snooze duration in seconds = 5 minutes

// Alarm Preset Interval (pressing C in MAIN MENU for quick countdown
#define PRESET_1_INTERVAL 15*60 // 15 minutes
#define PRESET_2_INTERVAL 30*60 // 30 minutes
#define PRESET_3_INTERVAL 1*60*60 // 1 hour
#define PRESET_4_INTERVAL 8*60*60 // 8 hours, for sleeping zZZ

// Melody
byte selectedMelody = 0; // default selected melody

// Melody (Tunes that play when alarm is set)
// NOTE: Due to memory limitations only select songs that you want.
// 	     Must have at least 1 MELODY chosen, no checking is done if there is no selected melody
#define MELODY_1 		// Star Wars Theme 1
//#define MELODY_2 		// Star Wars Theme 2
// #define MELODY_3 	// Tetris, uses 1,568 bytes (76% in UNO)
// #define MELODY_4 	// Ngan Cat, uses 1,868 bytes (91% in UNO)


bool keyPadTone = true; // whether to sound keypad on each press

// Tones when user press the keys on the keypad
#define KEYPAD_TONE_DURATION 250 // how long to sound the tone for each key click
#define KEY_TONE_DIFF 100 // incremental differences between each key tone

#define KEY_0_TONE_HZ 500
#define KEY_1_TONE_HZ KEY_0_TONE_HZ + KEY_TONE_DIFF
#define KEY_2_TONE_HZ KEY_1_TONE_HZ + KEY_TONE_DIFF
#define KEY_3_TONE_HZ KEY_2_TONE_HZ + KEY_TONE_DIFF
#define KEY_4_TONE_HZ KEY_3_TONE_HZ + KEY_TONE_DIFF
#define KEY_5_TONE_HZ KEY_4_TONE_HZ + KEY_TONE_DIFF
#define KEY_6_TONE_HZ KEY_5_TONE_HZ + KEY_TONE_DIFF
#define KEY_7_TONE_HZ KEY_6_TONE_HZ + KEY_TONE_DIFF
#define KEY_8_TONE_HZ KEY_7_TONE_HZ + KEY_TONE_DIFF
#define KEY_9_TONE_HZ KEY_8_TONE_HZ + KEY_TONE_DIFF

#define KEY_A_TONE_HZ KEY_9_TONE_HZ + KEY_TONE_DIFF
#define KEY_B_TONE_HZ KEY_A_TONE_HZ + KEY_TONE_DIFF
#define KEY_C_TONE_HZ KEY_B_TONE_HZ + KEY_TONE_DIFF
#define KEY_D_TONE_HZ KEY_C_TONE_HZ + KEY_TONE_DIFF

#define KEY_STAR_TONE_HZ KEY_D_TONE_HZ + KEY_TONE_DIFF
#define KEY_HASH_TONE_HZ KEY_STAR_TONE_HZ + KEY_TONE_DIFF

const int tones[] = {KEY_0_TONE_HZ, KEY_1_TONE_HZ, KEY_2_TONE_HZ, KEY_3_TONE_HZ, KEY_4_TONE_HZ,
					 KEY_5_TONE_HZ, KEY_6_TONE_HZ, KEY_7_TONE_HZ, KEY_8_TONE_HZ, KEY_9_TONE_HZ,
					 KEY_A_TONE_HZ, KEY_B_TONE_HZ, KEY_C_TONE_HZ, KEY_D_TONE_HZ,
					 KEY_STAR_TONE_HZ, KEY_HASH_TONE_HZ};

// Log
// Whether to show the last log first in the LOG MENU.
// If set to false, the log will show starting with entry #1
bool showLastLogFirst = true;

//---------------------------------------------------------------------------------------------######

/**----------------------------------------------------------------------
 |	HARDWARE MODULE DEFINITION
 ----------------------------------------------------------------------**/
//===================
// Speaker
//===================
#define SPEAKER_PIN 2 // digital pin that connects to the speaker, e.g. 8Ohm 0.5W

//===================
// LCD1602 I2C 2x16 (2 Rows, 16 columns)
//===================
#define LCD_1602_2x16

#ifdef LCD_1602_2x16 // mjkdz.com i2c @http://forum.arduino.cc/index.php?topic=157817.0
  #define I2C_ADDR    0x27  // Define I2C Address for the PCF8574T 
  //---(Following are the PCF8574 pin assignments to LCD connections )----
  // This are different than earlier/different I2C LCD displays
  #define BACKLIGHT_PIN  7
  #define En_pin  4
  #define Rw_pin  5
  #define Rs_pin  6
  #define D4_pin  0
  #define D5_pin  1
  #define D6_pin  2
  #define D7_pin  3

  #define LED_OFF  0
  #define LED_ON  1
#endif

LiquidCrystal_I2C lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);

// Variables used to refresh the LCD in every interval for time display
unsigned long prevLcdUpdateMillies = 0; // last time the LCD is refreshed

// period to refresh the LCD. Default is refresh it every 1 second.
// NOTE: DO NOT CHANGE THIS FOR NOW SINCE THE COUNTDOWN TIMER IS DECREASING BASED ON THIS INTERVAL
// TODO: Create internal count down timer interval in countdown() function
#define LCD_REFRESH_INTERVAL 1000 // in millisecond, default is 1 second

//==============================
// DS1307 RTC
//==============================
#define DS1307_I2C_ADDRESS 0x68  // This is the I2C address
byte second, minute, hour, dayOfWeek, dayOfMonth, month, year = 0;
byte zero;
const char  *Day[] = {"","Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
const char  *Mon[] = {"","Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};


//===================
// Keypad (4x4 Matrix)
//===================
const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns
const char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {3, 4, 5, 6}; // connect to the row pinouts of the keypad
byte colPins[COLS] = {7, 8, 9, 10}; // connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

//---------------------------------------------------------------------------------------------######
/**----------------------------------------------------------------------
 |	FINITE STATE MACHINES
 ----------------------------------------------------------------------**/
// Main FSM States
enum states {
	/**--------------------------
	  *	MAIN SCREEN that displays the following information:
	  *	Row 1: Time & Date
	  *	Row 2: Alarm (if set), various status indicator (snooze, repeat, melody playing...)
	  * 
	  *-------------------
	  *| 23:55:55  Sat 2 |
	  *| 5m15s		RS  !|
	  *-------------------
	  *
	  * Various key bindings will change this state to the following:
	  *-------------------------*/
	SHOW_TIME,

	/**--------------------------
	  * MAIN MENU SCREEN
	  * Key Binding: #
	  *-------------------------*/
	MAIN_MENU,

	/**--------------------------
	  * SET ALARM/TIMER SCREEN
	  *-------------------------*/	
	SET_ALARM,


	/**--------------------------
	  * SET ALARM
	  *-------------------------*/	
	SET_EXACT_TIME,

	/**--------------------------
	  * SET COUNTDOWM TIMER
	  *-------------------------*/	
	SET_COUNTDOWN_TIMER_MINUTES,
	SET_COUNTDOWN_TIMER_SECONDS,	

	/**--------------------------
	  * SET CLOCK SCREEN
	  *-------------------------*/	
	SET_TIME,

	/**--------------------------
	  * SELECT MELODY SCREEN
	  * Key Binding: #
	  *-------------------------*/	
	SELECT_MELODY,

	/**--------------------------
	  * PLAY THE TUNE
	  *-------------------------*/	
	PLAY_TUNE,

	/**--------------------------
	  * SET REPEAT MODE
	  *-------------------------*/		
	SET_REPEAT,

	/**--------------------------
	  * REPEAT SCREEN OPTION
	  *-------------------------*/	
	SET_REPEAT_OPTIONS,	

	/**--------------------------
	  * SET SNOOZE MODE
	  *-------------------------*/	
	SET_SNOOZE,

	/**--------------------------
	  * SNOOZE SCREEN OPTION
	  *-------------------------*/	
	SET_SNOOZE_OPTIONS,

	/**--------------------------
	  * LOGGING SCREEN
	  * Key Binding: 0 in MAIN SCREEN
	  *-------------------------*/	
	SHOW_LOG,

	/**--------------------------
	  * CLEAR LOG STATE
	  * Key Binding: D in LOGGING SCREEN
	  *-------------------------*/		
	CLEAR_LOG,

	/**--------------------------
	  * COUNTDOWN PRESET
	  * Key Binding: D in MAIN SCREEN
	  *-------------------------*/	
	COUNTDOWN_PRESET,

	/**--------------------------
	  * SET KEYPAD TONE
	  *-------------------------*/	
	SET_KEYPAD_TONE,

	/**--------------------------
	  * SET AUTO COUNTDOWN
	  *-------------------------*/		
	SET_AUTO_COUNTDOWN
};

// FSM sub-states for SET_TIME
enum settimestates {
	SET_YEAR,
	SET_MONTH,
	SET_DAY_OF_MONTH,
	SET_DAY_OF_WEEK,
	SET_HOUR,
	SET_MINUTE
};

// FSM sub-states for SET_SNOOZE_OPTIONS
enum snoozestates {
	SET_SNOOZE_STATUS, // ON or OFF
	SET_SNOOZE_DURATION // snooze interval
};

// FSM sub-states for SET_REPEAT_OPTIONS
enum repeatstates {
	SET_REPEAT_STATUS, // ON or OFF
	SET_REPEAT_DURATION // how many times to repeat
};

states state;

// Setting the time
settimestates settimestate;

// Snooze states
snoozestates snoozestate;

// Repeat states
repeatstates repeatstate;


//---------------------------------------------------------------------------------------------######
/**----------------------------------------------------------------------
 |	RUNTIME VARIABLES DEFINITION
 ----------------------------------------------------------------------**/
/**	===========================================================
|	USER KEY INPUT
============================================================= */
byte cursorPos = 0; // the cursor column position on the LCD, move to the right each time a key is pressed

byte idx = 0; // index of keyInput

// Store user inputs for up to 3 characters
// NOTE: There is no checking whether the array is out of bounds!
char keyInput[3] = {0,0,0}; 

// Flag to interrupt when the melody is playing
bool interrupt = false;

// does user press the key? if yes, update the display. This is to prevent every loop LCD refresh
// TODO: Need consistency to enforce it
bool keyPress = false;


//---------------------------------------------------------------------------------------------######
/**----------------------------------------------------------------------
 |	VARIABLES IN FSM
 ----------------------------------------------------------------------**/
/**	===========================================================
|	MAIN MENU
============================================================= */
byte currPageNum = 0; // the currently selected pageNum
byte totalPageNum = 4;


/**	===========================================================
|	COUNTDOWN TIMER
============================================================= */
byte inputMinute = 0; // minutes to countdown
byte inputSecond = 0; // seconds to countdown
int inputCountdownTime = 0; // total countdown time in millies

bool pauseCountdown = false; // a flag to pause the countdown timer when it is set 

/**----------------------------------------------------------------------
 |	STATUS MODE FLAG
 ----------------------------------------------------------------------**/
bool alarmSet = false; // is the alarm set?
bool countdownSet = false; // is the countdown timer set?
bool repeat = false; // is repeat mode set? If set, melody will repeat until user interrupt it by clicking 'C'
bool snooze = false; // is snooze mode on?

bool lcdDisplayOn = true; // lcd display is ON by default

bool statusModeChanged = false;

/**	===========================================================
|	TIMER
============================================================= */
char exactTimeInput[4]; // store user time in 24hr format, e.g. 1500 (3PM)

byte alarmHour, alarmMinute = 0;

// stores the current time in millies (since Arduino is started) as references
unsigned long currentMillies = 0L;

// stores the prev time to countdown
unsigned long prevCountdownMillies = 0L;

/**	===========================================================
|	SNOOZE MODE
============================================================= */
bool snoozeActivate = false; // does user click the 'D' button to snooze it when the melody is playing

/**	===========================================================
|	REPEAT MODE
============================================================= */
byte repeatCounter = 0; // current repeat counter (decrement each time melody is played)

/**	===========================================================
|	MELODY
============================================================= */
bool cancelTune = false; //  whether user is stopping the tune when it is playing
bool melodyFinish = false;
bool preview = false; //whether the melody is PREVIEW in the PLAY_TUNE state

int melodyIdx = 0;


/**----------------------------------------------------------------------
 |	LOGGING FEATURE
 ----------------------------------------------------------------------**/
// The number of Log entry to store. The oldest log is overwritten if the log is full
// NOTE: Set this according to the memory of your device and your needs.
// Future work might include assigning log name using T9 keypad
#define LOG_SIZE 3

Log *logArr[LOG_SIZE]; // create array to store the Log objects

byte logIdx = 0;

bool logStarted = false; // whether the log has started, i.e. user click the 'A' button in the SHOW_TIME state

byte selectedLogNum = 0; // the currently selected log in LOG MENU by user

byte logCount = 0; // how many VALID logs is stored (i.e. with STARTTIME, ENDTIME & DURATION)


/*
=====================================================================================================
|										POINT OF ENTRY												|			  	
=====================================================================================================
*/
void setup() {
	#ifdef DEBUG // print some text when in debuggin mode
		Serial.begin(9600);
	#endif

	initLCD(); // initialize the LCD

	initSpeaker(); // initialize the speaker pin

	// Remove this welcoming screen if you want
	lcd.print("Arduino A.T.Log");
	lcd.setCursor(0,1);
	lcd.print("by dsync89");

	state = SHOW_TIME; // initial state

	// Initialize the logs array
	initLogArray();

	keypad.addEventListener(keypadEvent); //add an event listener for this keypad

	delay(2000);
	lcd.clear();
}

void execAlarm() {
	if (alarmSet) { // if the alarm is set and the alarm is not rang		
		if (interrupt) {
			lcdPrint("Timer Cancel!", 0, 0);
			interrupt = false;
			setAlarmMode(OFF);
			clearKeyInput();
			state = SHOW_TIME;		
			delay(MSG_DISPLAY_PAUSE_INTERVAL);
			lcd.clear();
			return;
		}

		if (timeToBuzz()) { // check whether it is time to rang
			state = PLAY_TUNE; // play the alarm melody
			return;
		}

	}
}

/*
=====================================================================================================
|										THE BIG LOOP												|			  	
=====================================================================================================
*/
void loop() {
	char key = keypad.getKey();
  
	#ifdef DEBUG
		if (key) Serial.println(key);		
	#endif

	currentMillies = millis(); // reference time in each loop, used to update the screen and countdown

	// still counting down (if set) even if user is not in the Main Screen, e.g. in Main Menu
	execCountdown(); 

	execAlarm();

  	switch (state) {
	  	case SHOW_TIME:
	  		if (timeToUpdateLCD()) { // refresh lcd every 1 second
	  			lcdPrintTimeDate(0,0);
				lcdPrintAlarm(0,1);
	  			lcdPrintStatusIcon();  	

	  			prevLcdUpdateMillies = currentMillies;

			}
	  	break;

	  	// To minimize LCD printing, only update the following screen on each key press
	  	if (isKeyPress()) {
		  	case SHOW_LOG:
		  		printLogDetail(selectedLogNum);
		  	break;

		  	case CLEAR_LOG:
		  		lcdPrint("Are you sure?",0,0);
		  		lcdPrint("1.YES  2.NO",0,1);
		  	break;

		  	case SET_KEYPAD_TONE:
		  		lcdPrint("Set Keypad Tone",0,0);
		  		lcdPrint("1.ON  2.OFF",0,1);
		  	break;

		  	case COUNTDOWN_PRESET:
		  		lcdPrint("1.15m  2.30m",0,0);
		  		lcdPrint("3.1h   4.8h",0,1);
		  	break;

		  	case SET_AUTO_COUNTDOWN:
		  		lcdPrint("Set A.Countdown",0,0);
		  		lcdPrint("1.ON  2.OFF",0,1);
		  	break;		  	

		  	case MAIN_MENU:
		  		switch (currPageNum) {
		  			case 0: // Page 1
			  			lcdPrint("1.Set Time",0,0);
			  			lcdPrint("2.Set Repeat",0,1);  
			  			lcdPrint(">",15,1);	  			
			  		break;

			  		case 1:
			  			lcdPrint("3.Set Snooze",0,0);
			  			lcdPrint("4.24hr/AMPM",0,1); 
			  			lcdPrint("<",14,1);
			  			lcdPrint(">",15,1);	  			
		  			break;

		  			case 2:
			  			lcdPrint("5.Keypad Tone",0,0);  
			  			lcdPrint("6.LCD B.Light",0,1);
						lcdPrint("<",14,1);	  
						lcdPrint(">",15,1);	
					break;

					case 3:
						lcdPrint("7.Auto Countdown",0,0);
						lcdPrint("<",14,1);
					break;					
			  	}		
		  	break;

		  	case SET_TIME:
		  		switch(settimestate) {
		  			case SET_YEAR:
		  			  	lcdPrint("Set YEAR, 00-99",0,0);
		  			break; 

		  			case SET_MONTH:
		  			  	lcdPrint("Set MONTH, 1-12",0,0);
		  			break; 

		  			case SET_DAY_OF_MONTH:
		  			  	lcdPrint("Set M.DAY, 1-31",0,0);
		  			break; 

		  			case SET_DAY_OF_WEEK:
		  			  	lcdPrint("Set W.DAY, 1 Sun",0,0);
		  			break; 

		  			case SET_HOUR:
		  			  	lcdPrint("Set HOUR, 0-23",0,0);
		  			break;   

		  			case SET_MINUTE:
		  			  	lcdPrint("Set MIN, 0-59",0,0);
		  			break;   	  						  			  			

		  		}  	
		  	break;  

		  	case SET_REPEAT:
		  		lcdPrint("1.Status",0,0);
		  		lcdPrint("2.Duration",0,1);
		  	break;

		  	case SET_REPEAT_OPTIONS:
		  		switch (repeatstate) {
		  			case SET_REPEAT_STATUS:
				  		lcdPrint("Repeat Status",0,0);
				  		lcdPrint("1.ON 2.OFF",0,1);
				  	break;

				  	case SET_REPEAT_DURATION:
				  		lcdPrint("Repeat Count",0,0);
				  		lcd.setCursor(0,1);
				  		lcd.print("[");
				  		lcd.print(repeatCount);
				  		lcd.print("!]:");
				  	break;
				}
		  	break; 		  	

		  	case SET_SNOOZE:
		  		lcdPrint("1.Status",0,0);
		  		lcdPrint("2.Duration",0,1);
		  	break;  	

		  	case SET_SNOOZE_OPTIONS:
		  		switch (snoozestate) {
		  			case SET_SNOOZE_STATUS:
				  		lcdPrint("Snooze Status",0,0);
				  		lcdPrint("1.ON 2.OFF",0,1);
				  	break;

				  	case SET_SNOOZE_DURATION:
				  		lcdPrint("Snooze Period",0,0);
				  		lcd.setCursor(0,1);
				  		lcd.print("[");
				  		lcd.print(snoozeDuration/60);
				  		lcd.print("m]:");
				  	break;
				}
		  	break;  	  	


		  	case SELECT_MELODY:
		  		if (isKeyPress()) { 
			  		lcdPrint("Select Melody:", 0, 0);
					switch (selectedMelody) {
						case 0:
							lcdPrint("Star Wars 1", 0, 1);
						break;
						case 1:
							lcdPrint("Star Wars 2", 0, 1);
						break;
					}
				}
		  	break;

		  	case SET_ALARM:
		  		lcdPrint("1. Duration",0,0);
		  		lcdPrint("2. Exact Time",0,1);
		  	break;

		  	case SET_EXACT_TIME:
		  		lcdPrint("Enter 24hr time:",0,0);
		  	break;  	

		  	case SET_COUNTDOWN_TIMER_MINUTES:
				lcdPrint("Countdown Timer", 0, 0);
				lcdPrint("MM: ",0, 1);
				// lcd.cursor();
		  	break;

		  	case SET_COUNTDOWN_TIMER_SECONDS:
				lcdPrint("Countdown Timer", 0, 0);
				lcdPrint("SS: ",0, 1);
				// lcd.cursor();
		  	break; 
		}
	  	
	  	case PLAY_TUNE:
	  		if (preview) { // if previewing the melody
		  		if (!cancelTune && !melodyFinish) {
		  			playAlertTune();
		  		}

		  		else if (melodyFinish) {
		  			state = SELECT_MELODY;
		  			preview = false;
		  			cancelTune = false;
		  			melodyFinish = false;
		  			melodyIdx = 0;
		  			lcdPrint("Preview DONE", 0, 1);
		  			delay(MSG_DISPLAY_PAUSE_INTERVAL);
		  			lcd.clear();
		  		}

		  		else if (cancelTune) {
		  			state = SELECT_MELODY;
		  			cancelTune = false;
		  			preview = false;
		  			melodyIdx = 0;
		  			lcdPrint("Stop Preview", 0, 1);
		  			delay(MSG_DISPLAY_PAUSE_INTERVAL);
		  			lcd.clear();
		  		}
	  		}

	  		else { // not preview

		  		/** Show the time too in first row **/
		  		if (timeToUpdateLCD()) { // refresh lcd every 1 second
		  			lcdPrintTimeDate(0,0);
					lcdPrintAlarm(0,1);
		  			lcdPrintStatusIcon();

		 	  		// lcdPrintAlarm(0,1);		

		  			prevLcdUpdateMillies = currentMillies;
				}
			  			
		  		if (!cancelTune && !melodyFinish) {
		  			playAlertTune();
		  		}

		  		else if (melodyFinish) {
		  			// if REPEAT is ON, replay the tune
		  			if (repeat) {
		  				if (repeatCount == 0) {} // repeat indefinitely until user cancel or snooze

		  				else if (repeatCounter == 0) {
				  			lcd.clear();
				  			lcdPrint("Melody Finish!", 0, 0);
				  			state = SHOW_TIME;
				  			melodyFinish = false;
				  			melodyIdx = 0;
				  			setCountdownMode(OFF);
				  			setAlarmMode(OFF);
				  			delay(MSG_DISPLAY_PAUSE_INTERVAL);
				  			lcd.clear();
				  			return;		  					
		  				}

		  				else if (repeatCounter > 0)
		  					repeatCounter--;

			  			melodyFinish = false;
			  			melodyIdx = 0;
		  			}

		  			else { // if REPEAT is OFF
			  			lcd.clear();
			  			lcdPrint("Melody Finish!", 0, 0);
			  			state = SHOW_TIME;
			  			melodyFinish = false;
			  			melodyIdx = 0;
			  			setCountdownMode(OFF);
			  			setAlarmMode(OFF);
			  			delay(MSG_DISPLAY_PAUSE_INTERVAL);
			  			lcd.clear();
			  		}
		  		}


		  		else if (cancelTune) { // if melody is playing and user press 'C' to interrupt
		  			state = SHOW_TIME;
		  			cancelTune = false;
		  			melodyIdx = 0;	  		
		  			setCountdownMode(OFF);
		  			setAlarmMode(OFF);	  	
		  			snoozeActivate = false; // cancel the snooze	
		  			
		  			// melody that rang for the first one doesn't count towards repeat count
		  			repeatCounter = repeatCount-1; 
	
		  			lcd.clear();
		  			lcdPrint("Tune CANCEL",0, 0);
		  			delay(MSG_DISPLAY_PAUSE_INTERVAL);
		  			lcd.clear();
		  		}

		  		if (snooze) { // if user enabled snooze mode
		  			if (snoozeActivate) { // user wants to snooze by clicking the 'D' key
		  				if (alarmSet) {
		  					setAlarmMode(OFF); // disable the alarm
		  					setCountdownMode(ON); // treat the snooze just like countdown timer

		 					inputCountdownTime = snoozeDuration; // use the snooze duration as reference
		 					state = SHOW_TIME; // go back to show the MAIN SCREEN

		 					snoozeActivate = false; // reset the snooze switch so that user can trigger again
		 					melodyIdx = 0; // start the melody tune from beginning when countdown reaches 0	  					
		  				}
		  					  			
		  				else if (countdownSet) {
		  					prevCountdownMillies = 0; // make sure countdown and LCD time is refresh on sync
		 					inputCountdownTime = snoozeDuration;
		 					state = SHOW_TIME;

		 					snoozeActivate = false; // reset the snooze switch
		 					melodyIdx = 0; // start the melody tune from beginning
		  				}
		  			}
		  			lcd.clear();
		  		}
		  	}

	  	break;
	}

	// force update LCD immediately when any other state change to SHOW_TIME (MAIN SCREEN)
	if (state != SHOW_TIME)
		prevLcdUpdateMillies = 0; 
}

/*
=====================================================================================================
|									HARDWARE INITIALIZATION											|			  	
=====================================================================================================
*/
void initSpeaker() {
	pinMode(SPEAKER_PIN, OUTPUT);
}

void initLCD() {
  //TODO: Add more LCD types here...
  #ifdef LCD_1602_4x20
    // lcd.begin(20,4); // initialized the LCD, 20 char * 4 rows.
  #endif

  #ifdef LCD_1602_2x16
    lcd.begin(16,2); // initialized the LCD, 20 char * 4 rows
  #endif

  lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE);
}


/*
=====================================================================================================
|										KEYPAD EVENT												|			  	
=====================================================================================================
*/
void keypadEvent(KeypadEvent key){
	switch (state) {
  		case SHOW_TIME:
		  	switch (keypad.getState()) {
			    case PRESSED:
		        	playKeyTone(key);
			    	lcd.clear();
			      	switch (key) {
			      		case 'A':
			      			// Start logging only if there is no existing log that is running
			      			if (!logStarted)
			      				startLog();
			      			else {
			      				lcd.print("Log STARTED...");
			      				lcd.setCursor(0,1);
			      				lcd.print("Press B to STOP log");
			      				delay(MSG_DISPLAY_PAUSE_INTERVAL);
			      			}
			      		break;

			      		case 'B':
			      			// End logging
			      			if (logStarted) // only end the log if the log is started
			      				endLog();
			      			else {
			      				lcd.print("Log NOT STARTED...");
			      				lcd.setCursor(0,1);
			      				lcd.print("Press A to START");
			      				delay(MSG_DISPLAY_PAUSE_INTERVAL);
			      			}
			      		break;

			      		case '0':
			      			// Show the recorded log
			      			showLog();
			      		break;

				        case '*': 
				        	state = SET_ALARM;
				        break;
				        case '9': // Select melody
				        	state = SELECT_MELODY;
				        break;				 

			    		case 'C':
			    			// only register the interrupt if alarm or countdown timer is set
			    			if (alarmSet || countdownSet) 
			    				interrupt = true;
			    		break;

			    		case 'D': // set PRESET duration
			    			state = COUNTDOWN_PRESET;
			    		break;

				        case '#':
				        	state = MAIN_MENU;
					  		lcdPrint("Main Menu",0,0);
					  		delay(MSG_DISPLAY_PAUSE_INTERVAL);
				        break;

				        case '1':
				        	setRepeatMode(toggle(repeat));
				        break;

				        case '2':
				        	setSnoozeMode(toggle(snooze));
				        break;

				        case '4':
				        	toggle(showSeconds);
				        break;

				        case '5':
				        	toggle(pauseCountdown);
				        break;

				        case '7': // turn the display on/off
				        	toggle(lcdDisplayOn);
				        	if (lcdDisplayOn) {
				        		lcd.setBacklight(LED_OFF);
				        		lcd.display();
				        	}

				        	else {
				        		lcd.setBacklight(LED_ON);
				        		lcd.noDisplay();
				        	}
				        break;

				        // default:
				        // 	lcdPrint((key), 0, 1);
			      	}
			      	lcd.clear();			
			    	keyPress = true;
			    break;

			    case RELEASED:
			    	// lcd.clear();
					switch (key) {
						case '*': 
						  // lcdPrint("Minute:", 0, 1);
						  // lcd.cursor();
						  break;
						case '0':
						  // setLcdPageNum(0);
						  break;
						case '1':
						  // setLcdPageNum(1);
						  break;
						case '2':
						  // setLcdPageNum(2);          
						break;        
		      		}
		    		break;
			    case HOLD:
			    	// lcd.clear();
			      	switch (key) {
			        	case '*':
			        		// lcdPrint("HOLD *", 0, 1); 
			        	break;
			      	}
		    		break;
			}
			break;

		case SHOW_LOG:
		  	switch (keypad.getState()) {
			    case PRESSED:
			    	playKeyTone(key);
			    	lcd.clear();
			    	switch (key) {
			    		case 'C': // back to menu
			    			state = SHOW_TIME;

							if (showLastLogFirst) {	
								if (logStarted)												
									selectedLogNum = logIdx;
								else
									// get the previous log, otherwise we are getting the next log (which is not valid)
									selectedLogNum = logIdx-1; 
							}

			    			else
			    				selectedLogNum = 0; // always show the first log if user does not enable show last log first option
			    		break;

			    		case 'D':
			    			state = CLEAR_LOG;
			    		break;
			    					    		
			    		case '*': // PREV...
			    			prevLogPage();
			    		break;

			    		case '#': // NEXT...
			    			nextLogPage(); 			
			    		break;
				    }
				    lcd.clear();
					keyPress = true;
				break;
			}
		break;

		case CLEAR_LOG:
		  	switch (keypad.getState()) {
			    case PRESSED:
			    	playKeyTone(key);
			    	lcd.clear();
			    	switch (key) {
			    		case 'C':
			    		case '2': // back to showing the log
			    			state = SHOW_LOG;			    		
			    		break;

			    		case '1':
			    			clearLog();
			    			lcd.print("All LOGS Cleared!");
			    			delay(MSG_DISPLAY_PAUSE_INTERVAL);
			    			state = SHOW_TIME;
			    		break;
				    }
				    lcd.clear();
				   	keyPress = true;
				break;
			}
		break;

		case SET_KEYPAD_TONE:
		  	switch (keypad.getState()) {
			    case PRESSED:
			    	playKeyTone(key);
			    	lcd.clear();
			    	switch (key) {
			    		case 'B':
			    			state = MAIN_MENU;
			    		break;

			    		case 'C':
			    			state = SHOW_TIME;
			    		break;

			    		case '1':
			    			keyPadTone = true;
			    			lcdPrint("Keypad Tone ON",0,0);
			    			delay(MSG_DISPLAY_PAUSE_INTERVAL);
			    			state = MAIN_MENU;			    			
			    		break;

			    		case '2':
			    			keyPadTone = false;
			    			lcdPrint("Keypad Tone OFF",0,0);
			    			delay(MSG_DISPLAY_PAUSE_INTERVAL);
			    			state = MAIN_MENU;
			    		break;
				    }
				    lcd.clear();
					keyPress = true;
				break;
			}
		break;			

		case COUNTDOWN_PRESET:
		  	switch (keypad.getState()) {
			    case PRESSED:
			    	playKeyTone(key);
			    	lcd.clear();
			    	switch (key) {
			    		case '1':
			    			inputCountdownTime = PRESET_1_INTERVAL;
			    			setCountdownMode(true);
			    			state = SHOW_TIME;	    		
			    		break;
			    					    		
			    		case '2':
			    			inputCountdownTime = PRESET_2_INTERVAL;
			    			setCountdownMode(true);
			    			state = SHOW_TIME;			    					    			
			    		break;

			    		case '3':
			    			inputCountdownTime = PRESET_3_INTERVAL;
			    			setCountdownMode(true);
			    			state = SHOW_TIME;			    					    			
			    		break;

			    		case '4':
			    			inputCountdownTime = PRESET_4_INTERVAL;
			    			setCountdownMode(true);
			    			state = SHOW_TIME;			    					    			
			    		break;			   

			    		case 'B':
			    		case 'C':
			    			state = SHOW_TIME;
			    		break; 					    		
				    }
					lcd.clear();
					keyPress = true;
				break;
			}
		break;		

		case SET_AUTO_COUNTDOWN:
		  	switch (keypad.getState()) {
			    case PRESSED:
			    	playKeyTone(key);
			    	lcd.clear();
			    	switch (key) {
			    		case '1':
			    			autoStartCountdownTimer = true;
			    			lcdPrint("A.Start Countdown",0,0);
			    			lcdPrint("Set to ON",0,1);
			    			delay(MSG_DISPLAY_PAUSE_INTERVAL);
			    		break;
			    					    		
			    		case '2':
			    			autoStartCountdownTimer = false;
			    			lcdPrint("A.Start Countdown",0,0);
			    			lcdPrint("Set to OFF",0,1);
			    			delay(MSG_DISPLAY_PAUSE_INTERVAL);
			    		break;

			    		case 'B':
			    			state = MAIN_MENU;
			    		break;

			    		case 'C':
			    			state = SHOW_TIME;
			    		break;
				    }
				    lcd.clear();
					keyPress = true;
				break;
			}
		break;		

		case MAIN_MENU:
		  	switch (keypad.getState()) {
			    case PRESSED:
			    	playKeyTone(key);
			    	lcd.clear();
			    	switch (key) {
			    		case '1':
			    			state = SET_TIME;
			    			settimestate = SET_YEAR;
			    		break;
			    					    		
			    		case '2':
			    			state = SET_REPEAT;			    			
			    		break;

			    		case '3':
			    			state = SET_SNOOZE;
			    			lcd.print("Snooze Mode");
			    			delay(MSG_DISPLAY_PAUSE_INTERVAL);
			    		break;

			    		case '4':
			    			toggle(dispAMPM);
			    			if (dispAMPM) {
			    				lcd.print("Disp: AM/PM");
			    				delay(MSG_DISPLAY_PAUSE_INTERVAL);
			    			}

			    			else {
			    				lcd.print("Disp: 24hrs");
			    				delay(MSG_DISPLAY_PAUSE_INTERVAL);
			    			}   
			    		break;

			    		case '5':
			    			state = SET_KEYPAD_TONE;	
			    		break;

			    		case '6':
			    			if (!lcdBacklightOn) {			    				
			    				lcd.print("LCD B.Light ON");
			    				lcd.setBacklight(LED_OFF);
			    				delay(MSG_DISPLAY_PAUSE_INTERVAL);
			    			}

			    			else {
			    				lcd.print("LCD B.Light OFF");
			    				lcd.setBacklight(LED_ON);
			    				delay(MSG_DISPLAY_PAUSE_INTERVAL);
			    			}   
			    			toggle(lcdBacklightOn);

			    		break;			

			    		case '7':
			    			state = SET_AUTO_COUNTDOWN;
			    		break;			    		    		

			    		case '*':
			    			prevPage();
			    		break;

			    		case '#':
			    			nextPage();
			    		break;

			    		case 'C':
			    			state = SHOW_TIME;
			    		break;
				    }				    
				    lcd.clear();
					keyPress = true;
				break;
			}
		break;

		case SET_TIME: // set the clock
		  	switch (keypad.getState()) {
			    case PRESSED:
			    	playKeyTone(key);
			    	if (idx == 0) lcd.clear();

			    	switch (key) {
			    		case 'C': // Pressing C will return to the MAIN SCREEN
			    			state = SHOW_TIME;
			    			return; // immediately return back, no point to process the settimestate
			    		break;
			    	}

					switch (settimestate) {
						case SET_YEAR:					    	
					    	switch (key) {
					    		case '#':
					    			settimestate = SET_MONTH;
					    			year = readTimeInput();
					    			clearKeyInput();
					    		break;

					    		case 'B':
					    			state = MAIN_MENU;
					    			clearKeyInput();
					    		break;

					    		case 'D':
					    			clearKeyInput();
					    		break;

					    		default:
							    	keyInput[idx++] = key;
							      	lcdPrint(key, (++cursorPos)+3, 1);					      	
						    }
				    	break;

				    	case SET_MONTH:
					    	switch (key) {
					    		case '#':
					    			settimestate = SET_DAY_OF_MONTH;
					    			month = readTimeInput();
					    			clearKeyInput();
					    		break;

					    		case 'B':
					    			settimestate = SET_YEAR;
					    			clearKeyInput();
					    		break;

					    		case 'D':
					    			clearKeyInput();
					    		break;					    		

					    		default:
							    	keyInput[idx++] = key;
							      	lcdPrint(key, (++cursorPos)+3, 1);					      	
						    }
				    	break;		

				    	case SET_DAY_OF_MONTH:
					    	switch (key) {
					    		case '#':
					    			settimestate = SET_DAY_OF_WEEK;
					    			dayOfMonth = readTimeInput();					    			

					    			clearKeyInput();
					    		break;

					    		case 'B':
					    			settimestate = SET_MONTH;
					    			clearKeyInput();
					    		break;

					    		case 'D':
					    			clearKeyInput();
					    		break;						    		

					    		default:
							    	keyInput[idx++] = key;
							      	lcdPrint(key, (++cursorPos)+3, 1);					      	
						    }
				    	break;	

				    	case SET_DAY_OF_WEEK:
					    	switch (key) {
					    		case '#':
					    			settimestate = SET_HOUR;
					    			dayOfWeek = readTimeInput();
					    			clearKeyInput();
					    		break;

					    		case 'B':
					    			settimestate = SET_DAY_OF_MONTH;
					    			clearKeyInput();
					    		break;

					    		case 'D':
					    			clearKeyInput();
					    		break;	

					    		default:
							    	keyInput[idx++] = key;
							      	lcdPrint(key, (++cursorPos)+3, 1);					      	
						    }
				    	break;	

				    	case SET_HOUR:
					    	switch (key) {
					    		case '#':
					    			settimestate = SET_MINUTE;
					    			hour = readTimeInput();
					    			clearKeyInput();
					    		break;

					    		case 'B':
					    			settimestate = SET_DAY_OF_WEEK;
					    			clearKeyInput();
					    		break;

					    		case 'D':
					    			clearKeyInput();
					    		break;						    		

					    		default:
							    	keyInput[idx++] = key;
							      	lcdPrint(key, (++cursorPos)+3, 1);					      	
						    }
				    	break;		

				    	case SET_MINUTE:
					    	switch (key) {
					    		case '#':					    			
					    			// settimestate = SET_YEAR; //reset the time state in case user want to set it again
					    			minute = readTimeInput();
					    			clearKeyInput();

					    			setClock(); // set the alarm clock

					    			lcdPrint("Time SET!", 0,0);		
					    			delay(MSG_DISPLAY_PAUSE_INTERVAL);	

					    			state = SHOW_TIME;
					    		break;

					    		case 'B':
					    			settimestate = SET_HOUR;
					    			clearKeyInput();
					    		break;

					    		case 'D':
					    			clearKeyInput();
					    		break;						    		

					    		default:
							    	keyInput[idx++] = key;
							      	lcdPrint(key, (++cursorPos)+3, 1);					      	
						    }
				    	break;					    				    					    					    			    		

				    }
					if (idx == 0) lcd.clear();
					keyPress = true;
				break;
			}
		break;		

		case SET_REPEAT:
		  	switch (keypad.getState()) {
			    case PRESSED:
			    	playKeyTone(key);
			    	lcd.clear();
			    	switch (key) {
			    		case '1':
			    			repeatstate = SET_REPEAT_STATUS;
			    			state = SET_REPEAT_OPTIONS;
			    		break;
			    					    		
			    		case '2':
			    			repeatstate = SET_REPEAT_DURATION;
			    			state = SET_REPEAT_OPTIONS;
			    		break;

			    		case 'B':
			    			state = MAIN_MENU;
			    		break;

			    		case 'C':
			    			state = SHOW_TIME;
			    		break;				    		
				    }
				    lcd.clear();
					keyPress = true;
				break;
			}
		break;	

		case SET_SNOOZE:
		  	switch (keypad.getState()) {
			    case PRESSED:
			    	playKeyTone(key);
			    	lcd.clear();
			    	switch (key) {
			    		case '1':
			    			snoozestate = SET_SNOOZE_STATUS;
			    			state = SET_SNOOZE_OPTIONS;	   
			    		break;
			    					    		
			    		case '2':
			    			snoozestate = SET_SNOOZE_DURATION;
			    			state = SET_SNOOZE_OPTIONS;	    			
			    		break;
				    }
				    lcd.clear();
					keyPress = true;
				break;
			}
		break;

		case SET_REPEAT_OPTIONS:
		  	switch (keypad.getState()) {
			    case PRESSED:
			    	playKeyTone(key);
			    	if (idx == 0) lcd.clear();

			    	// common actions
					switch (key) {
						// keys to ignore
						case '*':
						case 'A':
						return;

			    		case 'B':
			    			state = SET_REPEAT;
			    			clearKeyInput();
			    		break;

			    		case 'C':
			    			state = SHOW_TIME;
			    			clearKeyInput();
			    		break;	
			    	}	

				    switch (repeatstate) {
			    		case SET_REPEAT_STATUS:
					    	switch (key) {
					    		case '1':			
					    			setRepeatMode(ON);
					    			lcdPrint("Repeat ON",0,0);
					    			delay(MSG_DISPLAY_PAUSE_INTERVAL);						    					    			
					    		break;

					    		case '2':		
					    			setRepeatMode(OFF);	    			
					    			lcdPrint("Repeat OFF",0,0);
					    			delay(MSG_DISPLAY_PAUSE_INTERVAL);
					    		break;					    		
						    }			    		
			    		break;
			    					    		
			    		case SET_REPEAT_DURATION:
					    	switch (key) {
					    		case '#':			
					    			state = SET_REPEAT;		    			
					    			repeatCount = charToNum(keyInput); // unit is minute
					    			// melody that rang for the first one doesn't count towards repeat count
					    			repeatCounter = repeatCount-1; 
					    			clearKeyInput();

					    			lcd.clear();
					    			lcdPrint("Rpt Count SET:", 0,0);
					    			lcd.setCursor(0,1);
					    			lcd.print(repeatCount);		
					    			delay(MSG_DISPLAY_PAUSE_INTERVAL);			    			
					    		break;

					    		case 'D':
					    			clearKeyInput();
					    		break;					    		

					    		default:
							    	keyInput[idx++] = key;
							      	lcdPrint(key, (++cursorPos)+4, 1);								      	
						    }				    						    			
			    		break;	    							 
					}
					if (idx == 0) lcd.clear();
					keyPress = true;
				break;
			}
		break;		

		case SET_SNOOZE_OPTIONS:
		  	switch (keypad.getState()) {
			    case PRESSED:
			    	playKeyTone(key);
			    	if (idx == 0) lcd.clear();
				    switch (snoozestate) {
			    		case SET_SNOOZE_STATUS:
					    	switch (key) {
					    		case '1':			
					    			setSnoozeMode(ON);
					    			state = SHOW_TIME;	
					    			lcd.home();
					    			lcd.print("Snooze ON");		
					    			delay(MSG_DISPLAY_PAUSE_INTERVAL);						    					    			
					    		break;

					    		case '2':		
					    			setSnoozeMode(OFF);
					    			lcd.home();
					    			lcd.print("Snooze OFF");		
					    			delay(MSG_DISPLAY_PAUSE_INTERVAL);						    			
					    			state = SHOW_TIME;	
					    		break;					    		
						    }			    		
			    		break;
			    					    		
			    		case SET_SNOOZE_DURATION:
					    	switch (key) {
					    		case '#':					    			
					    			state = SHOW_TIME;
					    			// settimestate = SET_YEAR; //reset the time state in case user want to set it again
					    			snoozeDuration = charToNum(keyInput) * 60; // unit is minute

					    			clearKeyInput();

					    			lcd.clear();
					    			lcdPrint("Snooze Set to:", 0,0);
					    			lcd.setCursor(0,1);
					    			lcd.print(snoozeDuration);		
					    			delay(MSG_DISPLAY_PAUSE_INTERVAL);			    			
					    		break;

					    		case 'B':
					    			state = SET_SNOOZE;
					    			clearKeyInput();
					    		break;

					    		case 'C':
					    			state = SHOW_TIME;
					    			clearKeyInput();
					    		break;

					    		default:
							    	keyInput[idx++] = key;
							      	lcd.print(key);				      	
							      	cursorPos++;
						    }				    						    			
			    		break;					 
					}
					if (idx == 0) lcd.clear();
					keyPress = true;
				break;
			}
		break;				

		case SET_ALARM:
		  	switch (keypad.getState()) {
			    case PRESSED:
			    	playKeyTone(key);
			    	lcd.clear();
			    	switch (key) {
			    		case '1':
			    			state = SET_COUNTDOWN_TIMER_MINUTES;
			    		break;
			    					    		
			    		case '2':
			    			state = SET_EXACT_TIME;
			    		break;

			    		case 'B':
			    		case 'C':
			    			state = SHOW_TIME;
			    		break;
				    }
				    lcd.clear();
					keyPress = true;
				break;
			}
		break;

		case SET_EXACT_TIME: // Setting the alarm exactly in 24 hours format
		  	switch (keypad.getState()) {
			    case PRESSED:
			    	playKeyTone(key);
			    	if (idx == 0) lcd.clear();
			    	switch (key) {
			    		case '#':
			    			state = SHOW_TIME;
			    			processAlarmString(); // extract the hours and minutes from the char array
			    			lcd.clear();
			    			lcdPrint("Alarm SET!", 0, 0);
			    			lcdPrintAlarm(0,1);
			    			delay(MSG_DISPLAY_PAUSE_INTERVAL);
			    			clearKeyInput();
							// only either alarm or countdown can be active at a time
			    			if (countdownSet) toggle(countdownSet); 
			    		break;

			    		case 'B':
			    			state = SET_ALARM;
			    			clearKeyInput();
			    		break;

			    		case 'C':
			    			state = SHOW_TIME;
			    			clearKeyInput();
			    		break;

			    		case 'D':
			    			clearKeyInput();
			    		break;

			    		default:
					    	exactTimeInput[idx++] = key;
					      	lcdPrint(key, (++cursorPos)+3, 1);					      	
				    }
				    if (idx == 0) lcd.clear();
					keyPress = true;
				break;
			}			
		break;

		case SET_COUNTDOWN_TIMER_MINUTES:
		  	switch (keypad.getState()) {
			    case PRESSED:
			    	playKeyTone(key);
			    	if (idx == 0) lcd.clear(); // only clear the screen if the keyinput is empty
			    	switch (key) {
			    		case '#':
			    			state = SET_COUNTDOWN_TIMER_SECONDS;
			    			inputMinute = charToNum(keyInput);	
			    			clearKeyInput();
			    		break;

			    		case 'B':
			    			state = SET_ALARM;
			    			clearKeyInput();
			    		break;

			    		case 'C':
			    			state = SHOW_TIME;
			    			clearKeyInput();
			    		break;

			    		case 'D': // delete the input
			    			clearKeyInput();
			    		break;

			    		default:
					    	keyInput[idx++] = key;
					      	lcdPrint(key, (++cursorPos)+3, 1);					      	
				    }
					if (idx == 0) lcd.clear();
					keyPress = true;
				break;
			}
		break;

		case SET_COUNTDOWN_TIMER_SECONDS:
		  	switch (keypad.getState()) {
			    case PRESSED:
			    	playKeyTone(key);
			    	if (idx == 0) lcd.clear();
			    	switch (key) {
			    		case '#':
			    			state = SHOW_TIME;
			    			setCountdownMode(true);		

			    			inputSecond = charToNum(keyInput);
			    			clearKeyInput();
			    			inputCountdownTime = (inputMinute*60) + inputSecond; // update the totaltime for countdown

			    			lcd.clear();
			    			lcdPrint("Countdown SET!", 0, 0);
			    			lcd.setCursor(0,1);
			    			// lcd.print(inputSecond);

			    			lcdPrintMinuteSecond(inputCountdownTime);

			    			delay(MSG_DISPLAY_PAUSE_INTERVAL);
			    			clearKeyInput();

			    			if (alarmSet) toggle(alarmSet); 
			    		break;

			    		case 'B': // back to set countdown minutes
			    			state = SET_COUNTDOWN_TIMER_MINUTES;
			    			clearKeyInput();
			    		break;			    		

			    		case 'C':
			    			state = SHOW_TIME;
			    			clearKeyInput();
			    		break;			

			    		case 'D': // delete the input
			    			clearKeyInput();
			    		break;			    		    		

			    		default:
					    	keyInput[idx++] = key;
					      	lcdPrint(key, (++cursorPos)+3, 1);					      	
				    }
				    if (idx == 0) lcd.clear();
				    keyPress = true;
				break;
			}
		break;		

		case SELECT_MELODY:
		  	switch (keypad.getState()) {
			    case PRESSED:
			    	playKeyTone(key);
			    	lcd.clear();
			    	switch (key) {
			    		case '1':
			    			selectedMelody = 0;
			    			lcdPrint("Star Wars 1", 0, 1 );
			    			// state = SHOW_TIME;
			    			delay(MSG_DISPLAY_PAUSE_INTERVAL);			    						   			
			    		break;

			    		case '2':
			    			selectedMelody = 1;
			    			lcdPrint("Star Wars 2", 0, 1 );	
			    			// state = SHOW_TIME;
			    			delay(MSG_DISPLAY_PAUSE_INTERVAL);	
			    		break;

			    		case '*': // PREVIEW MELODY
			    			state = PLAY_TUNE;
			    			preview = true;
			    			lcdPrint("PREVIEWING...", 0, 1);
			    		break;

			    		case '#': // SET MELODY
			    			state = SHOW_TIME;
			    			lcdPrint("Melody SET!", 0, 0);
			    			switch (selectedMelody) {
			    				case 0:
			    					lcdPrint("Star Wars 1", 0, 1);
			    				break;
			    				case 1:
			    					lcdPrint("Star Wars 2", 0, 1);
			    				break;
			    			}
			    			delay(MSG_DISPLAY_PAUSE_INTERVAL);
			    		break;

			    		case 'B':
			    		case 'C':
			    			state = SHOW_TIME;
			    		break;
				    }
				    lcd.clear();
				    keyPress = true;
				break;
			}
		break;

		case PLAY_TUNE:
		  	switch (keypad.getState()) {
			    case PRESSED:
			    	playKeyTone(key);
			    	lcd.clear();
			    	switch (key) {
			    		case 'C':
			    			cancelTune = true;	
			    			return;	    						   			
			    		break;	

			    		case 'D': // snooze
			    			snoozeActivate = true;
			    			return;
			    		break;
				    }
				    lcd.clear();
					keyPress = true;
				break;
			}
	}
  
}

/*
=====================================================================================================
|										FUNCTION DEFINITIONS										|			  	
=====================================================================================================


/*
=====================================================================================================
|																							   MELODY
=====================================================================================================
*/ 
// MELODIES and TIMING //
//  melody[] is an array of notes, accompanied by beats[], 
//  which sets each note's relative length (higher #, longer note)
#ifdef MELODY_1
	// Melody 1: Star Wars Imperial March
	const int melody1[] = {  a4, R,  a4, R,  a4, R,  f4, R, c5, R,  a4, R,  f4, R, c5, R, a4, R,  e5, R,  e5, R,  e5, R,  f5, R, c5, R,  g5, R,  f5, R,  c5, R, a4, R};
	const byte beats1[]  = {  50, 20, 50, 20, 50, 20, 40, 5, 20, 5,  60, 10, 40, 5, 20, 5, 60, 80, 50, 20, 50, 20, 50, 20, 40, 5, 20, 5,  60, 10, 40, 5,  20, 5, 60, 40};
#endif 

#ifdef MELODY_2
	// Melody 2: Star Wars Main Theme
	const int melody2[] = {  f4,  f4, f4,  a4s,   f5,  d5s,  d5,  c5, a5s, f5, d5s,  d5,  c5, a5s, f5, d5s, d5, d5s,   c5};
	const byte beats2[]  = {  21,  21, 21,  128,  128,   21,  21,  21, 128, 64,  21,  21,  21, 128, 64,  21, 21,  21, 128 }; 
 #endif

#ifdef MELODY_3 // Tetris, Credits @https://github.com/electricmango/Arduino-Music-Project
	int melody3[] = {
	  NOTE_E5, NOTE_E3, NOTE_B4, NOTE_C5, NOTE_D5, NOTE_E5, NOTE_D5, NOTE_C5, 
	  NOTE_B4, NOTE_A4, NOTE_A3, NOTE_A4, NOTE_C5, NOTE_E5, NOTE_A3, NOTE_D5,
	  NOTE_C5, NOTE_B4, NOTE_E4, NOTE_G4, NOTE_C5, NOTE_D5, NOTE_E3, NOTE_E5,
	  NOTE_E3, NOTE_C5, NOTE_A3, NOTE_A4, NOTE_A3, NOTE_A4, NOTE_A3, NOTE_B2, 
	  NOTE_C3, NOTE_D3, NOTE_D5, NOTE_F5, NOTE_A5, NOTE_C5, NOTE_C5, NOTE_G5, 
	  NOTE_F5, NOTE_E5, NOTE_C3, 0, NOTE_C5, NOTE_E5, NOTE_A4, NOTE_G4, NOTE_D5,
	  NOTE_C5, NOTE_B4, NOTE_E4, NOTE_B4, NOTE_C5, NOTE_D5, NOTE_G4, NOTE_E5, 
	  NOTE_G4, NOTE_C5, NOTE_E4, NOTE_A4, NOTE_E3, NOTE_A4, 0, 
	  NOTE_E5, NOTE_E3, NOTE_B4, NOTE_C5, NOTE_D5, NOTE_E5, NOTE_D5, NOTE_C5, 
	  NOTE_B4, NOTE_A4, NOTE_A3, NOTE_A4, NOTE_C5, NOTE_E5, NOTE_A3, NOTE_D5,
	  NOTE_C5, NOTE_B4, NOTE_E4, NOTE_G4, NOTE_C5, NOTE_D5, NOTE_E3, NOTE_E5,
	  NOTE_E3, NOTE_C5, NOTE_A3, NOTE_A4, NOTE_A3, NOTE_A4, NOTE_A3, NOTE_B2, 
	  NOTE_C3, NOTE_D3, NOTE_D5, NOTE_F5, NOTE_A5, NOTE_C5, NOTE_C5, NOTE_G5, 
	  NOTE_F5, NOTE_E5, NOTE_C3, 0, NOTE_C5, NOTE_E5, NOTE_A4, NOTE_G4, NOTE_D5,
	  NOTE_C5, NOTE_B4, NOTE_E4, NOTE_B4, NOTE_C5, NOTE_D5, NOTE_G4, NOTE_E5, 
	  NOTE_G4, NOTE_C5, NOTE_E4, NOTE_A4, NOTE_E3, NOTE_A4, 0,
	  NOTE_E4, NOTE_E3, NOTE_A2, NOTE_E3, NOTE_C4, NOTE_E3, NOTE_A2, NOTE_E3, 
	  NOTE_D4, NOTE_E3, NOTE_GS2, NOTE_E3, NOTE_B3, NOTE_E3, NOTE_GS2, NOTE_E3,
	  NOTE_C4, NOTE_E3, NOTE_A2, NOTE_E3, NOTE_A3, NOTE_E3, NOTE_A2, NOTE_E3, 
	  NOTE_GS3, NOTE_E3, NOTE_GS2, NOTE_E3, NOTE_B3, NOTE_E3, NOTE_GS2, NOTE_E3, 
	  NOTE_E4, NOTE_E3, NOTE_A2, NOTE_E3, NOTE_C4, NOTE_E3, NOTE_A2, NOTE_E3, 
	  NOTE_D4, NOTE_E3, NOTE_GS2, NOTE_E3, NOTE_B3, NOTE_E3, NOTE_GS2, NOTE_E3,
	  NOTE_C4, NOTE_E3, NOTE_E4, NOTE_E3, NOTE_A4, NOTE_E3, NOTE_A2, NOTE_E3, 
	  NOTE_GS4, NOTE_E3, NOTE_GS2, NOTE_E3, NOTE_GS2, NOTE_E3, NOTE_GS2, NOTE_E3,
	  NOTE_E5, NOTE_E3, NOTE_B4, NOTE_C5, NOTE_D5, NOTE_E5, NOTE_D5, NOTE_C5, 
	  NOTE_B4, NOTE_A4, NOTE_A3, NOTE_A4, NOTE_C5, NOTE_E5, NOTE_A3, NOTE_D5,
	  NOTE_C5, NOTE_B4, NOTE_E4, NOTE_G4, NOTE_C5, NOTE_D5, NOTE_E3, NOTE_E5,
	  NOTE_E3, NOTE_C5, NOTE_A3, NOTE_A4, NOTE_A3, NOTE_A4, NOTE_A3, NOTE_B2, 
	  NOTE_C3, NOTE_D3, NOTE_D5, NOTE_F5, NOTE_A5, NOTE_C5, NOTE_C5, NOTE_G5, 
	  NOTE_F5, NOTE_E5, NOTE_C3, 0, NOTE_C5, NOTE_E5, NOTE_A4, NOTE_G4, NOTE_D5,
	  NOTE_C5, NOTE_B4, NOTE_E4, NOTE_B4, NOTE_C5, NOTE_D5, NOTE_G4, NOTE_E5, 
	  NOTE_G4, NOTE_C5, NOTE_E4, NOTE_A4, NOTE_E3, NOTE_A4, 0, 
	  NOTE_E5, NOTE_E3, NOTE_B4, NOTE_C5, NOTE_D5, NOTE_E5, NOTE_D5, NOTE_C5, 
	  NOTE_B4, NOTE_A4, NOTE_A3, NOTE_A4, NOTE_C5, NOTE_E5, NOTE_A3, NOTE_D5,
	  NOTE_C5, NOTE_B4, NOTE_E4, NOTE_G4, NOTE_C5, NOTE_D5, NOTE_E3, NOTE_E5,
	  NOTE_E3, NOTE_C5, NOTE_A3, NOTE_A4, NOTE_A3, NOTE_A4, NOTE_A3, NOTE_B2, 
	  NOTE_C3, NOTE_D3, NOTE_D5, NOTE_F5, NOTE_A5, NOTE_C5, NOTE_C5, NOTE_G5, 
	  NOTE_F5, NOTE_E5, NOTE_C3, 0, NOTE_C5, NOTE_E5, NOTE_A4, NOTE_G4, NOTE_D5,
	  NOTE_C5, NOTE_B4, NOTE_E4, NOTE_B4, NOTE_C5, NOTE_D5, NOTE_G4, NOTE_E5, 
	  NOTE_G4, NOTE_C5, NOTE_E4, NOTE_A4, NOTE_E3, NOTE_A4, 0,
	  NOTE_E4, NOTE_E3, NOTE_A2, NOTE_E3, NOTE_C4, NOTE_E3, NOTE_A2, NOTE_E3, 
	  NOTE_D4, NOTE_E3, NOTE_GS2, NOTE_E3, NOTE_B3, NOTE_E3, NOTE_GS2, NOTE_E3,
	  NOTE_C4, NOTE_E3, NOTE_A2, NOTE_E3, NOTE_A3, NOTE_E3, NOTE_A2, NOTE_E3, 
	  NOTE_GS3, NOTE_E3, NOTE_GS2, NOTE_E3, NOTE_B3, NOTE_E3, NOTE_GS2, NOTE_E3, 
	  NOTE_E4, NOTE_E3, NOTE_A2, NOTE_E3, NOTE_C4, NOTE_E3, NOTE_A2, NOTE_E3, 
	  NOTE_D4, NOTE_E3, NOTE_GS2, NOTE_E3, NOTE_B3, NOTE_E3, NOTE_GS2, NOTE_E3,
	  NOTE_C4, NOTE_E3, NOTE_E4, NOTE_E3, NOTE_A4, NOTE_E3, NOTE_A2, NOTE_E3, 
	  NOTE_GS4, NOTE_E3, NOTE_GS2, NOTE_E3, NOTE_GS2, NOTE_E3, NOTE_GS2, NOTE_E3,
	};

	//note durations: 4 = quarter note, 8 = eighth note, etc
	int beats3[] = {
	  8,8,8,8,8,16,16,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
	  8,4,8,8,16,16,8,8,8,8,8,8,8,16,16,8,8,8,8,8,8,8,8,8,8,8,8,8,8,4,4,
	  8,8,8,8,8,16,16,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
	  8,4,8,8,16,16,8,8,8,8,8,8,8,16,16,8,8,8,8,8,8,8,8,8,8,8,8,8,8,4,4,
	  8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
	  8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
	  8,8,8,8,8,16,16,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
	  8,4,8,8,16,16,8,8,8,8,8,8,8,16,16,8,8,8,8,8,8,8,8,8,8,8,8,8,8,4,4,
	  8,8,8,8,8,16,16,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
	  8,4,8,8,16,16,8,8,8,8,8,8,8,16,16,8,8,8,8,8,8,8,8,8,8,8,8,8,8,4,4,
	  8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
	  8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
	};
#endif

#ifdef MELODY_4 // Nyan Cat, Credits: https://github.com/electricmango/Arduino-Music-Project
	int melody4[] = {
	  NOTE_DS5, NOTE_E5, NOTE_FS5, 0, NOTE_B5, NOTE_E5, NOTE_DS5, NOTE_E5, NOTE_FS5, NOTE_B5, NOTE_DS6, NOTE_E6, NOTE_DS6, NOTE_AS5, NOTE_B5, 0,
	  NOTE_FS5, 0, NOTE_DS5, NOTE_E5, NOTE_FS5, 0, NOTE_B5, NOTE_CS6, NOTE_AS5, NOTE_B5, NOTE_CS6, NOTE_E6, NOTE_DS6, NOTE_E6, NOTE_CS6,
	  NOTE_FS4, NOTE_GS4, NOTE_D4, NOTE_DS4, NOTE_FS2, NOTE_CS4, NOTE_D4, NOTE_CS4, NOTE_B3, NOTE_B3, NOTE_CS4,
	  NOTE_D4, NOTE_D4, NOTE_CS4, NOTE_B3, NOTE_CS4, NOTE_DS4, NOTE_FS4, NOTE_GS4, NOTE_DS4, NOTE_FS4, NOTE_CS4, NOTE_DS4, NOTE_B3, NOTE_CS4, NOTE_B3,
	  NOTE_DS4, NOTE_FS4, NOTE_GS4, NOTE_DS4, NOTE_FS4, NOTE_CS4, NOTE_DS4, NOTE_B3, NOTE_D4, NOTE_DS4, NOTE_D4, NOTE_CS4, NOTE_B3, NOTE_CS4,
	  NOTE_D4, NOTE_B3, NOTE_CS4, NOTE_DS4, NOTE_FS4, NOTE_CS4, NOTE_D4, NOTE_CS4, NOTE_B3, NOTE_CS4, NOTE_B3, NOTE_CS4,
	  NOTE_FS4, NOTE_GS4, NOTE_D4, NOTE_DS4, NOTE_FS2, NOTE_CS4, NOTE_D4, NOTE_CS4, NOTE_B3, NOTE_B3, NOTE_CS4,
	  NOTE_D4, NOTE_D4, NOTE_CS4, NOTE_B3, NOTE_CS4, NOTE_DS4, NOTE_FS4, NOTE_GS4, NOTE_DS4, NOTE_FS4, NOTE_CS4, NOTE_DS4, NOTE_B3, NOTE_CS4, NOTE_B3,
	  NOTE_DS4, NOTE_FS4, NOTE_GS4, NOTE_DS4, NOTE_FS4, NOTE_CS4, NOTE_DS4, NOTE_B3, NOTE_D4, NOTE_DS4, NOTE_D4, NOTE_CS4, NOTE_B3, NOTE_CS4,
	  NOTE_D4, NOTE_B3, NOTE_CS4, NOTE_DS4, NOTE_FS4, NOTE_CS4, NOTE_D4, NOTE_CS4, NOTE_B3, NOTE_CS4, NOTE_B3, NOTE_B3,
	  NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_CS4, NOTE_DS4, NOTE_B3, NOTE_E4, NOTE_DS4, NOTE_E4, NOTE_FS4,
	  NOTE_B3, NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_FS3, NOTE_E4, NOTE_DS4, NOTE_CS4, NOTE_B3, NOTE_E3, NOTE_DS3, NOTE_E3, NOTE_FS3,
	  NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_B3, NOTE_CS4, NOTE_DS4, NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_FS3,
	  NOTE_B3, NOTE_B3, NOTE_AS3, NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_E4, NOTE_DS4, NOTE_E4, NOTE_FS4, NOTE_B3, NOTE_AS3,
	  NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_CS4, NOTE_DS4, NOTE_B3, NOTE_E4, NOTE_DS4, NOTE_E4, NOTE_FS4,
	  NOTE_B3, NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_FS3, NOTE_E4, NOTE_DS4, NOTE_CS4, NOTE_B3, NOTE_E3, NOTE_DS3, NOTE_E3, NOTE_FS3,
	  NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_B3, NOTE_CS4, NOTE_DS4, NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_FS3,
	  NOTE_B3, NOTE_B3, NOTE_AS3, NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_E4, NOTE_DS4, NOTE_E4, NOTE_FS4, NOTE_B3, NOTE_CS4,
	  NOTE_FS4, NOTE_GS4, NOTE_D4, NOTE_DS4, NOTE_FS2, NOTE_CS4, NOTE_D4, NOTE_CS4, NOTE_B3, NOTE_B3, NOTE_CS4,
	  NOTE_D4, NOTE_D4, NOTE_CS4, NOTE_B3, NOTE_CS4, NOTE_DS4, NOTE_FS4, NOTE_GS4, NOTE_DS4, NOTE_FS4, NOTE_CS4, NOTE_DS4, NOTE_B3, NOTE_CS4, NOTE_B3,
	  NOTE_DS4, NOTE_FS4, NOTE_GS4, NOTE_DS4, NOTE_FS4, NOTE_CS4, NOTE_DS4, NOTE_B3, NOTE_D4, NOTE_DS4, NOTE_D4, NOTE_CS4, NOTE_B3, NOTE_CS4,
	  NOTE_D4, NOTE_B3, NOTE_CS4, NOTE_DS4, NOTE_FS4, NOTE_CS4, NOTE_D4, NOTE_CS4, NOTE_B3, NOTE_CS4, NOTE_B3, NOTE_CS4,
	  NOTE_FS4, NOTE_GS4, NOTE_D4, NOTE_DS4, NOTE_FS2, NOTE_CS4, NOTE_D4, NOTE_CS4, NOTE_B3, NOTE_B3, NOTE_CS4,
	  NOTE_D4, NOTE_D4, NOTE_CS4, NOTE_B3, NOTE_CS4, NOTE_DS4, NOTE_FS4, NOTE_GS4, NOTE_DS4, NOTE_FS4, NOTE_CS4, NOTE_DS4, NOTE_B3, NOTE_CS4, NOTE_B3,
	  NOTE_DS4, NOTE_FS4, NOTE_GS4, NOTE_DS4, NOTE_FS4, NOTE_CS4, NOTE_DS4, NOTE_B3, NOTE_D4, NOTE_DS4, NOTE_D4, NOTE_CS4, NOTE_B3, NOTE_CS4,
	  NOTE_D4, NOTE_B3, NOTE_CS4, NOTE_DS4, NOTE_FS4, NOTE_CS4, NOTE_D4, NOTE_CS4, NOTE_B3, NOTE_CS4, NOTE_B3, NOTE_B3,
	  NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_CS4, NOTE_DS4, NOTE_B3, NOTE_E4, NOTE_DS4, NOTE_E4, NOTE_FS4,
	  NOTE_B3, NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_FS3, NOTE_E4, NOTE_DS4, NOTE_CS4, NOTE_B3, NOTE_E3, NOTE_DS3, NOTE_E3, NOTE_FS3,
	  NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_B3, NOTE_CS4, NOTE_DS4, NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_FS3,
	  NOTE_B3, NOTE_B3, NOTE_AS3, NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_E4, NOTE_DS4, NOTE_E4, NOTE_FS4, NOTE_B3, NOTE_AS3,
	  NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_CS4, NOTE_DS4, NOTE_B3, NOTE_E4, NOTE_DS4, NOTE_E4, NOTE_FS4,
	  NOTE_B3, NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_FS3, NOTE_E4, NOTE_DS4, NOTE_CS4, NOTE_B3, NOTE_E3, NOTE_DS3, NOTE_E3, NOTE_FS3,
	  NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_B3, NOTE_CS4, NOTE_DS4, NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_FS3,
	  NOTE_B3, NOTE_B3, NOTE_AS3, NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_E4, NOTE_DS4, NOTE_E4, NOTE_FS4, NOTE_B3, NOTE_CS4,
	};

	// note durations: 4 = quarter note, 8 = eighth note, etc
	int beats4[] = {
	  16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
	  16,16,16,16,16,16,8,16,16,16,16,16,16,16,16,
	  8,8,16,16,16,16,16,16,8,8,8,
	  8,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
	  8,8,16,16,16,16,16,16,16,16,16,16,16,16,
	  8,16,16,16,16,16,16,16,16,8,8,8,
	  8,8,16,16,16,16,16,16,8,8,8,
	  8,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
	  8,8,16,16,16,16,16,16,16,16,16,16,16,16,
	  8,16,16,16,16,16,16,16,16,8,8,8,
	  8,16,16,8,16,16,16,16,16,16,16,16,16,16,
	  8,8,16,16,16,16,16,16,16,16,16,16,16,16,
	  8,16,16,8,16,16,16,16,16,16,16,16,16,16,
	  8,16,16,16,16,16,16,16,16,16,16,8,8,
	  8,16,16,8,16,16,16,16,16,16,16,16,16,16,
	  8,8,16,16,16,16,16,16,16,16,16,16,16,16,
	  8,16,16,8,16,16,16,16,16,16,16,16,16,16,
	  8,16,16,16,16,16,16,16,16,16,16,8,8,
	  8,8,16,16,16,16,16,16,8,8,8,
	  8,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
	  8,8,16,16,16,16,16,16,16,16,16,16,16,16,
	  8,16,16,16,16,16,16,16,16,8,8,8,
	  8,8,16,16,16,16,16,16,8,8,8,
	  8,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
	  8,8,16,16,16,16,16,16,16,16,16,16,16,16,
	  8,16,16,16,16,16,16,16,16,8,8,8,
	  8,16,16,8,16,16,16,16,16,16,16,16,16,16,
	  8,8,16,16,16,16,16,16,16,16,16,16,16,16,
	  8,16,16,8,16,16,16,16,16,16,16,16,16,16,
	  8,16,16,16,16,16,16,16,16,16,16,8,8,
	  8,16,16,8,16,16,16,16,16,16,16,16,16,16,
	  8,8,16,16,16,16,16,16,16,16,16,16,16,16,
	  8,16,16,8,16,16,16,16,16,16,16,16,16,16,
	  8,16,16,16,16,16,16,16,16,16,16,8,8,
	};
#endif

int MAX_COUNT = 0; // Melody length, for looping.
 
long tempo = 10000; // Set overall tempo
 
int pause = 1000; // Set length of pause between notes
 
byte rest_count = 50; // Loop variable to increase Rest length (BLETCHEROUS HACK; See NOTES)
 
// Initialize core variables
int toneM = 0;
int beat = 0;
long duration  = 0;

void playTone() {
  long elapsed_time = 0;
  if (toneM > 0) { // if this isn't a Rest beat, while the tone has 
    //  played less long than 'duration', pulse speaker HIGH and LOW
    while (elapsed_time < duration) {
 
      digitalWrite(SPEAKER_PIN,HIGH);
      delayMicroseconds(toneM / 2);
 
      // DOWN
      digitalWrite(SPEAKER_PIN, LOW);
      delayMicroseconds(toneM / 2);
 
      // Keep track of how long we pulsed
      elapsed_time += (toneM);
    } 
  }
  else { // Rest beat; loop times delay
    for (int j = 0; j < rest_count; j++) { // See NOTE on rest_count
      delayMicroseconds(duration);  
    }                                
  }                                 
}

/**
  * Play the melody when alert/countdown timer is triggered
  */
void playAlertTune() {
	switch (selectedMelody) {
		case 0:
			#ifdef MELODY_1
				MAX_COUNT = sizeof(melody1) / 2; // Melody length, for looping.
				
				if (melodyIdx < MAX_COUNT) {
				    toneM = melody1[melodyIdx];
				    beat = beats1[melodyIdx];
				 
				    duration = beat * tempo; // Set up timing
				 
				    playTone(); // A pause between notes
				    delayMicroseconds(pause);				
					melodyIdx++;
				}

				else {
					melodyFinish = true;
				}	
			#endif		

		break;

		case 1:
			#ifdef MELODY_2
				MAX_COUNT = sizeof(melody2) / 2; // Melody length, for looping.
				if (melodyIdx < MAX_COUNT) {
				    toneM = melody2[melodyIdx];
				    beat = beats2[melodyIdx];
				 
				    duration = beat * tempo; // Set up timing
				 
				    playTone(); // A pause between notes
				    delayMicroseconds(pause);				
					melodyIdx++;
				}

				else {
					melodyFinish = true;
				}
			#endif
		break;

		case 2:
			#ifdef MELODY_3
				MAX_COUNT = 1000; // Melody length, for looping.
				if (melodyIdx < MAX_COUNT) {
				    toneM = melody3[melodyIdx];
				    beat = beats3[melodyIdx];
				 
				    duration = beat * tempo; // Set up timing
				 
				    playTone(); // A pause between notes
				    delayMicroseconds(pause);				
					melodyIdx++;
				}

				else {
					melodyFinish = true;
				}
			#endif
		break;		

		case 3:
			#ifdef MELODY_4
				MAX_COUNT = 1000; // Melody length, for looping.
				if (melodyIdx < MAX_COUNT) {
				    toneM = melody4[melodyIdx];
				    beat = beats4[melodyIdx];
				 
				    duration = beat * tempo; // Set up timing
				 
				    playTone(); // A pause between notes
				    delayMicroseconds(pause);				
					melodyIdx++;
				}

				else {
					melodyFinish = true;
				}
			#endif
		break;			
	}	
}

/*
=====================================================================================================
|																							  KEYPAD
=====================================================================================================
*/ 
/**
  * Play the key tone when the key is pressed
  */
void playKeyTone(KeypadEvent &key) {
	if (keyPadTone) {
		byte toneIdx = 0;
		switch (key) {
			case 'A':
				toneIdx = 10;
			break;

			case 'B':
				toneIdx = 11;
			break;

			case 'C':
				toneIdx = 12;
			break;

			case 'D':
				toneIdx = 13;
			break;

			case '*':
				toneIdx = 14;
			break;

			case '#':
				toneIdx = 15;
			break;

			default:
				toneIdx = charToNum(key);
		}
		buzz(SPEAKER_PIN, tones[toneIdx], KEYPAD_TONE_DURATION); // buzz the buzzer on pin 4 at 2500Hz for 1000 milliseconds	
	}
}


/*
=====================================================================================================
|																							  DS1307
=====================================================================================================
*/ 
/**
 * Set the clock to DS1307 NVRAM
 **/
void setClock() {
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(byte(0));
  Wire.write(decToBcd(second));
  Wire.write(decToBcd(minute));
  Wire.write(decToBcd(hour));
  Wire.write(decToBcd(dayOfWeek));
  Wire.write(decToBcd(dayOfMonth));
  Wire.write(decToBcd(month));
  Wire.write(decToBcd(year));
  Wire.write(byte(0));
  Wire.endTransmission();	
}

// Convert normal decimal numbers to binary coded decimal
byte decToBcd(byte val)
{
  return ( (val/10*16) + (val%10) );
}
 
// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
  return ( (val/16*10) + (val%16) );
}

/*
=====================================================================================================
|																							   CLOCK
=====================================================================================================
*/ 
/**
  * Follows the AMPM rule
  * If set to AMPM and hour > 12, return hour  = hour - 12. This is handled in lcdPrintTimeDate()
 */
int getCurrentHour() {
	return (int)hour;
}

int getCurrentMinute() {
	return (int)minute;
}

int getCurrentSecond() {
	return (int)second;	
}

/**
  * Is it time to sound the alarm?
  * A very crude and simple way to check for now. 
  * It could have been improved by checking against the millies
  */
bool timeToBuzz() {
	if (alarmHour == getCurrentHour() && alarmMinute == getCurrentMinute()) {
		return true;
	}
	return false;
}


/**
  * Convert the 4 digits 24 hours alarm into separate alarmHour and alarmMinute variables
  */
void processAlarmString() {

	alarmHour = ( (exactTimeInput[0]-'0') * 10 ) + ( (exactTimeInput[1]-'0') * 1 );
	alarmMinute = ( (exactTimeInput[2]-'0') * 10 ) + ( (exactTimeInput[3]-'0') * 1 );

	// if (dispAMPM) {
	// 	if (alarmHour > 12)
	// 		alarmHour -= 12;
	// }

	setAlarmMode(ON);
}



/*
=====================================================================================================
|																							  	LCD
=====================================================================================================
*/
void lcdPrint(char* text, int col, int row) {
  lcd.setCursor(col,row);
  lcd.print(text);
}

void lcdPrint(char text, int col, int row) {
  lcd.setCursor(col,row);
  lcd.print(text);
}

/**
  * Print the time and date
  */
void lcdPrintTimeDate(int col, int row) {
	// Reset the register pointer
	Wire.beginTransmission(DS1307_I2C_ADDRESS);
	I2C_WRITE(zero);
	Wire.endTransmission();

	Wire.requestFrom(DS1307_I2C_ADDRESS, 7);

	// A few of these need masks because certain bits are control bits
	second     = bcdToDec(I2C_READ() & 0x7f);
	minute     = bcdToDec(I2C_READ());
	hour       = bcdToDec(I2C_READ() & 0x3f);  // Need to change this if 12 hour am/pm
	dayOfWeek  = bcdToDec(I2C_READ());
	dayOfMonth = bcdToDec(I2C_READ());
	month      = bcdToDec(I2C_READ());
	year       = bcdToDec(I2C_READ());

	lcd.home();

	if (dispAMPM) { // if displaying AM/PM
		if (hour > 12)
			hour-=12;
	}
	if (hour < 10)
		lcd.print("0");
			
	lcd.print(hour);
	lcd.print(":");

	if (minute < 10)
	lcd.print("0");
	lcd.print(minute);


	if (showSeconds) {
		lcd.print(":");		
		if (second < 10)
			lcd.print("0");
		lcd.print(second);
	}


	lcd.print("  ");
	lcd.print(Day[dayOfWeek]);
	lcd.print(" ");
	lcd.print(dayOfMonth);

	if (!showSeconds) {
		lcd.print(" ");
		lcd.print(Mon[month]);
	}
  
}

/**
  * Print the Alarm time. It expects the caller to check
  * for whether the alarm is set first.
  *
  * i.e. if(alarmSet) {lcdPrintAlarm(0,0)}
  */
void lcdPrintAlarm(int col, int row) {
	if (alarmSet) {
		lcd.setCursor(col, row);

		if (dispAMPM) {
			if (alarmHour > 12)
				alarmHour -= 12;
		}

		if (alarmHour < 10)
			lcd.print("0");
		lcd.print(alarmHour);

		lcd.print(":");

		if (alarmMinute < 10)
			lcd.print("0");
		lcd.print(alarmMinute);
	}
}


/**
  * Print the time in hr, minute and second format dynamically depending on duration
  * 01m23s
  * NOTE: Make sure to set the LCD cursor first, lcd.setCursor()
  */
void lcdPrintMinuteSecond(int inputSeconds) {
	// print hours, minutes and seconds 
	if (inputSeconds/3600 >= 1) {
		lcd.print(inputSeconds/3600);
		lcd.print("h");

		lcd.print( (inputSeconds%3600) / 60);
		lcd.print("m");

		lcd.print( (inputSeconds%3600) % 60);
		lcd.print("s");	
	}

	// print minute and seconds
	else if (inputSeconds/60 > 0) {
		lcd.print(inputSeconds/60);
		lcd.print("m");

		lcd.print(inputSeconds%60);
		lcd.print("s");		
	}

	else {
		lcd.print(inputSeconds%60);
		lcd.print("s");			
	}

}

/**
  * Clear the line in LCD. Used only if the values in the line is dynamically changing
  */
void lcdClearLine(int row) {
	lcd.setCursor(0, row);	
	lcd.print("                ");
}


/**
  * Print the status indicator in the Main Screen. 
  * Only update the status indicator when status is changing.
  * TODO: Beautify it by rendering custom icon using lcd.write(char) instead of just text, paying
  *		  the price of more memory consumption
  */
void lcdPrintStatusIcon() {
	if (countdownSet && pauseCountdown) { // if countdown is set and user pause the countdown
		lcd.setCursor(8,1);
		lcd.print("P");
	}

	if (repeat) {
		lcd.setCursor(9,1);
		lcd.print("R");
	}

	if (snooze) {
		lcd.setCursor(10,1);
		lcd.print("S");
	}	

	if (alarmSet) {
		lcd.setCursor(11,1);
		lcd.print("A");
	}

	if (countdownSet) {
		lcd.setCursor(12,1);
		lcd.print("C");
	}

	if (logStarted) {
		lcd.setCursor(13,1);
		lcd.print("T");
	}

	if (state == PLAY_TUNE) {
		lcd.setCursor(15,1);
		lcd.print("!");
	}
}


/**
  * Is it time to update LCD?
  */
bool timeToUpdateLCD() {
	if (currentMillies - prevLcdUpdateMillies > LCD_REFRESH_INTERVAL) return true;
	return false;
}

/*
=====================================================================================================
|																							    SOUND
=====================================================================================================
*/
/**
  * Play the keypad tone
  */
void buzz(int targetPin, long frequency, long length) {
  long delayValue = 1000000/frequency/2; // calculate the delay value between transitions
  //// 1 second's worth of microseconds, divided by the frequency, then split in half since
  //// there are two phases to each cycle
  long numCycles = frequency * length/ 1000; // calculate the number of cycles for proper timing
  //// multiply frequency, which is really cycles per second, by the number of seconds to
  //// get the total number of cycles to produce
 for (long i=0; i < numCycles; i++){ // for the calculated length of time...
    digitalWrite(targetPin,HIGH); // write the buzzer pin high to push out the diaphram
    delayMicroseconds(delayValue); // wait for the calculated delay value
    digitalWrite(targetPin,LOW); // write the buzzer pin low to pull back the diaphram
    delayMicroseconds(delayValue); // wait againf or the calculated delay value
  }
}

/*
=====================================================================================================
|																							COUNTDOWN
=====================================================================================================
*/
void execCountdown() {
	if (countdownSet) { // if the countdown timer is set and auto startdown is on
		if (currentMillies - prevCountdownMillies > 1000) {// update every second

			if (onCountdownInterrupt()) return; // always process interrupt first

			lcdPrintCountdown();		


			// play the melody when countdown timer hits 0
			if (inputCountdownTime <= 0) {				
				clearKeyInput();
				state = PLAY_TUNE;
				return;
			}	


			if (!pauseCountdown) // decrement the counter if the user does not pause
				countdown();

			prevCountdownMillies = currentMillies;	
		}			
	}	
}

/**
  * Decrement the countdown timer on each tick.
  */
void countdown() {
	if (!interrupt && inputCountdownTime >= 0) {		
		inputCountdownTime--;
	}
}

/**
  * Print the remaining time for countdown on the LCD
  */
void lcdPrintCountdown() {
	// only print the countdown timer if user is in Main Screen
	if (state == SHOW_TIME) {
		// lcdClearLine(1); // clear row 1		
		lcd.setCursor(0,1);		
		lcd.print("        ");
		lcd.setCursor(0,1);
		lcdPrintMinuteSecond(inputCountdownTime);
	}	
}




/**
  * Execute when user interrupt the countdown
  */
bool onCountdownInterrupt() {
	if (interrupt) {
		lcdPrint("Timer Cancel!", 0, 0);
		interrupt = false;
		setCountdownMode(OFF);

		clearKeyInput();
		state = SHOW_TIME;		
		delay(MSG_DISPLAY_PAUSE_INTERVAL);
		lcd.clear();

		return true;
	}

	return false;

}

/*
=====================================================================================================
|																							  LOGGING
=====================================================================================================
*/
void showLog() {
	if (logIsEmpty()) { // if there is no logs
		state = SHOW_TIME; // switch back to MAIN SCREEN
		return;
	}

	lcd.print("Loading LOGS...");
	delay(MSG_DISPLAY_PAUSE_INTERVAL);			

	lcd.clear();	
	state = SHOW_LOG;	
}

void startLog() {
	// reset the log index if it is full, replacing the older ones
	if (logIdx >= LOG_SIZE) {
		logIdx = 0;
	}

	if (showLastLogFirst) { // show the active log when user enabled Showing Last Log First option
		selectedLogNum = logIdx;
	}	

	logArr[logIdx]->initLogRecord();

	logStarted = true;
	// Time t1;
	// t1.hour = getCurrentHour();
	// t1.minute = getCurrentMinute();
	// log.startTime = t1;
	logArr[logIdx]->startHour = getCurrentHour();
	logArr[logIdx]->startMinute = getCurrentMinute();
	logArr[logIdx]->startSecond = getCurrentSecond();

	lcd.clear();
	lcdPrint("Start LOG...", 0, 0);
	lcd.setCursor(0,1);

	if (logArr[logIdx]->startHour < 10)
		lcd.print("0");
	lcd.print(logArr[logIdx]->startHour);
	lcd.print(":");

	if (logArr[logIdx]->startMinute < 10)
		lcd.print("0");
	lcd.print(logArr[logIdx]->startMinute);
	lcd.print(":");

	if (logArr[logIdx]->startSecond < 10)
		lcd.print("0");	
	lcd.print(logArr[logIdx]->startSecond);

	delay(MSG_DISPLAY_PAUSE_INTERVAL);
	lcd.clear();	
}

void endLog() {
	logStarted = false;
	// Time t2;
	// t2.hour = getCurrentHour();
	// t2.minute = getCurrentMinute();
	// log.endTime = t2;
	logArr[logIdx]->endHour = getCurrentHour();
	logArr[logIdx]->endMinute = getCurrentMinute();
	logArr[logIdx]->endSecond = getCurrentSecond();

	// calculate the duration of the log
	logArr[logIdx]->duration = calcDuration(logIdx);

	lcd.clear();
	lcdPrint("End LOG...", 0, 0);
	lcd.setCursor(0,1);

	if (logArr[logIdx]->endHour < 10)
		lcd.print("0");
	lcd.print(logArr[logIdx]->endHour);
	lcd.print(":");

	if (logArr[logIdx]->endMinute < 10)
		lcd.print("0");	
	lcd.print(logArr[logIdx]->endMinute);
	lcd.print(":");

	if (logArr[logIdx]->endSecond < 10)
		lcd.print("0");	
	lcd.print(logArr[logIdx]->endSecond);			      			

	delay(MSG_DISPLAY_PAUSE_INTERVAL);

	lcd.clear();
	printLogDetail(logIdx);

	delay(2000); // a slightly longer pause to view the details of the log		
	


	// check whether the 
	if (logCount > LOG_SIZE-1)
		logCount = LOG_SIZE;

	else 
		logCount++;

	// whether to show the last recorded log first when in LOG MENU screen
//	if (showLastLogFirst) {
//		if (logCount > 0) 
//			selectedLogNum = logIdx;
//			// selectedLogNum = logCount-1;
//	}

	// move the log index to the next, no checking is necessary before it is checked in startLog()
	logIdx++; 

	lcd.clear();	
}



/**
  * Calculate the total duration of the log in seconds
  */
int calcDuration(int logIdx) {
	int diffInMillies = 	( (logArr[logIdx]->endHour*360) + (logArr[logIdx]->endMinute*60) + (logArr[logIdx]->endSecond) ) -
							( (logArr[logIdx]->startHour*360) + (logArr[logIdx]->startMinute*60) + (logArr[logIdx]->startSecond) );							
	return diffInMillies;
}

/**
  * Print the log details in following format:
  *
  * S: <HH:MM:SS>	#<LOGNUMBER>
  * E: <HH:MM:SS>	XXMXXS (Duration)
  */
void printLogDetail(int selectedLogNum) {
	// lcd.print("S:");
	if (logArr[selectedLogNum]->startHour < 10)
		lcd.print("0");
	lcd.print(logArr[selectedLogNum]->startHour);
	lcd.print(":");

	if (logArr[selectedLogNum]->startMinute < 10)
		lcd.print("0");	
	lcd.print(logArr[selectedLogNum]->startMinute);
	lcd.print(":");

	if (logArr[selectedLogNum]->startSecond < 10)
		lcd.print("0");		
	lcd.print(logArr[selectedLogNum]->startSecond);			      			

	lcd.setCursor(0,1);

	// lcd.print("E:");
	if (logArr[selectedLogNum]->endHour < 10)
		lcd.print("0");	
	lcd.print(logArr[selectedLogNum]->endHour);
	lcd.print(":");

	if (logArr[selectedLogNum]->endMinute < 10)
		lcd.print("0");	
	lcd.print(logArr[selectedLogNum]->endMinute);
	lcd.print(":");

	if (logArr[selectedLogNum]->endSecond < 10)
		lcd.print("0");	
	lcd.print(logArr[selectedLogNum]->endSecond);

	// print the selected log number
	lcd.setCursor(14,0);

	if (LOG_SIZE > 10)
		lcd.setCursor(13,0);

	lcd.print("#");
	lcd.print(selectedLogNum + 1); // display human readable #

	// Print duration
	lcd.setCursor(10,1);

	// lcdPrintMinuteSecond(logArr[selectedLogNum]->duration);


	printLogDuration(logArr[selectedLogNum]->duration);


	// int durationMinute = logArr[selectedLogNum]->duration / 60;
	// int durationSecond = logArr[selectedLogNum]->duration % 60;

	// if (durationMinute > 0) {
	// 	lcd.setCursor(10,1);
	// 	if (durationMinute < 10)
	// 		lcd.print("0");
	// 	lcd.print(durationMinute);
	// 	lcd.setCursor(12,1);
	// 	lcd.print("m");
	// }

	// lcd.setCursor(13,1);
	// if (durationSecond < 10)
	// 	lcd.print("0");
	// lcd.print(durationSecond);
	// lcd.setCursor(15,1);
	// lcd.print("s");
}

void printLogDuration(int duration) {
	// print hours, minutes and seconds 
	if (duration/3600 >= 1) {
		lcd.setCursor(8,1);
		lcd.print(duration/3600);
		lcd.print("h");

		lcd.print( (duration%3600) / 60);
		lcd.print("m");

		lcd.print( (duration%3600) % 60);
		lcd.print("s");	
	}

	// print minute and seconds
	else if (duration/60 > 0) {
		lcd.setCursor(10,1);
		if (duration/60 < 10);
			lcd.print("0");	
		lcd.print(duration/60);
		lcd.print("m");

		lcd.print(duration%60);
		lcd.print("s");		
	}

	else {
		lcd.setCursor(13,1);
		if (duration < 10)
			lcd.print("0");		
		lcd.print(duration);
		lcd.print("s");			
	}
}

/**

/**
  * Check whether the log is empty
  * Just checking the duration for simplicity
  */
bool logIsEmpty() {
	if (logCount <= 0) {
		lcd.clear();
		lcd.print("LOGS IS EMPTY!");
		delay(MSG_DISPLAY_PAUSE_INTERVAL);
		return true;		
	}
	return false;
}


void prevLogPage() {
	if (selectedLogNum > 0) {
		selectedLogNum--;	
	}
	else 
		selectedLogNum = LOG_SIZE - 1; // loop back	
	lcd.clear();
}

void nextLogPage() {
	if (selectedLogNum < LOG_SIZE - 1) {
		selectedLogNum++;
	}
	else 
		selectedLogNum = 0; // loop back
	lcd.clear();	
}

void clearLog() {
	memset(&logArr,0,sizeof(logArr));
	// initialize the log array
	initLogArray();

	logCount = 0;
	logIdx = 0;
	selectedLogNum = 0;
}

void initLogArray() {
	for (byte i = 0 ; i < LOG_SIZE ; i++)
		logArr[i] = new Log(0,0,0,0,0,0);	
}



/**====================================================================
|	SET TIME MENU
=====================================================================**/

byte readTimeInput() {
	byte input = (byte)charToNum(keyInput);

	if (input >= '0' && input <= '9')
		input = input * 10 + (input - '0');
	return input;
}
/*
=====================================================================================================
|																						MAIN MENU NAV.
=====================================================================================================
*/

void nextPage() {
	if (currPageNum < totalPageNum-1) {
		currPageNum++;
	}
	else 
		currPageNum = 0; // loop back
	lcd.clear();
}

void prevPage() {
	if (currPageNum > 0) {
		currPageNum--;	
	}
	else 
		currPageNum = totalPageNum - 1; // loop back
	lcd.clear();
}

/*
=====================================================================================================
|																					  DATA TYPE CONV.
=====================================================================================================
*/
int charToNum(char character) {
	return character-'0';
}

int charToNum(char* character) {
	return atoi(character);
}

char* numToChar(int number) {
	char chr[sizeof(number)];
	String str = String(number);
	str.toCharArray(chr, 5);
	return chr;
}


/*
=====================================================================================================
|																							  	MISC.
=====================================================================================================
*/
/**
  * Toggle the state of the input
  */
bool toggle(bool &input) {
	if (input)
		input = false;
	else
		input = true;
	return input;
}

/**
  * Does user click the key? LCD is refreshed if true. 
  */
bool isKeyPress() {
	if (keyPress) {
		keyPress = false;
		return true;
	}

	return false;
}

/**
  * Clear the user key input buffer
  */
void clearKeyInput() {
	memset(&keyInput,0,sizeof(keyInput));
	idx = 0;
	cursorPos = 0;
}

/*
=====================================================================================================
		|																			  SET STATUS MODE
=====================================================================================================
*/
void setRepeatMode(bool flag) {
	repeat = flag;
}

void setSnoozeMode(bool flag) {
	snooze = flag;
}

void setAlarmMode(bool flag) {
	alarmSet = flag;
}

void setLogMode(bool flag) {
	logStarted = flag;
}

/** 
  * Set the countdown flag when user had entered a countdown duration.
  */
void setCountdownMode(bool flag) {
	countdownSet = flag;

	// pause the countdown at start when user disable auto countdown via main menu
	if (autoStartCountdownTimer)
		pauseCountdown = false;
	else
		pauseCountdown = true;
}

//-------------------------------------------------------------------------------------- END OF FILE






























