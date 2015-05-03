# Arduino Alarm-Timer-Logger All-In-One
####by dsync89

An implementation of alarm, countdown timer and logger ALL-IN-ONE for the
Arduino microcontroller.

# A Little Bit of History...
I first started this project to implement a simple bedtime clock with nothing but alarm in mind. As time goes by I thought why not adding countdown, melodies and logging for my quick tracking needs. I could only found simple examples but not having the features that I want for my tracking needs

I do not have intention to wiring this program with the Internet (due to the memory limitation on the Uno after loading this program). There are indeed other alarm apps, such as Intelligent Clock that will set alarm based on Google calendar. This program does not meant to compete with that category.

# Note
**The program is run and tested using Arduino UNO Revision 3 having 2,048 bytes of memory space**

**It heavily make use of the 4x4 matrix keypad to transition to various state and capturing inputs. LCD Keypad Shield with buttons are not supported.**

**Refer to the *Hardware* section for a list of hardware required for it to work out of the box**


> **This program occupies is 91% of the total availble memory on the UNO**
>
> Due to this reason it is advisible **NOT** to include additional code when running this program.

# Program Features
- Alarm (obviously!).
- Countdown timer (in minutes & seconds)
- Timer logging (A-B). Useful for quick logging duration for an activity.
- Viewing log. View the recorded logs with start time, end time, and duration (Default is 5 logs)
- Preset mode for countdown duration (user definable). Default preset: 15min, 30min, 1 hour, and 8 hours
- Main menu to enable/disable certain features
- Clock setting mode. Clock is set to DS1307 NVRAM.
- Common alarm features such as repeat, snooze...
- Selectable Melody (star wars, nyan cat, and tetris) (see *Credits* section for the original author)
- Switchable time display between 24 hours format or AM/PM mode
- Quick setting mode when in Main Screen
- Status mode indicator on LCD for activated/enabled mode
... and possibly more (refer the end of this README)

# Referenced Libraries
- LiquidCrystal_I2C.h LCD library written by Francisco Malpartida (fmalpartida@gmail.com)
- Keypad.h keypad library written by Mark Stanley and Alexander Brevig.

# Hardware

- 1602 16x2 LCD and I2C controller designed by mjkdz.com
- 4x4 Matrix membrane keypad (with 0..9, A..D, * and # key)
- Tiny RTC (with onboard DS1307 RTC and DS08B20 digital temperature)
- Speaker (8Ohm, 0.5W)
- Resistor (220 Ohm or above) or Potentiometer (connecting to the Speaker)


# Implementation
All the states are modeled using FSM. State transition are triggered on each
key PRESSING event on the keypad using Keypad library written by Mark Stanley 
and Alexander Brevig.


#Navigation
	<Item>													<Key Binding>
	| Main Screen
	|-- Main Menu
	    |---- Set Time (clock time is stored in DS1307 RTC NVRAM)	'1'
		|---- Set Repeat (on/off)									'2'
		|     |---- Status (on/off)    								'1'
		|     |---- Duration (in times)    							'2'
		|---- Set Snooze    										'3'
		|     |---- Status (on/off)    								'1'
		|     |---- Duration (in minutes)    						'2'
		|---- Display Time (24 military or AM/PM)    				'4'
		|---- Keypad Tone (on/off)    								'5'
		|---- LCD Backlight (on/off)    							'6'
		|---- Auto Countdown (on/off)    							'7'

You may also trigger/toogle some commonly used settings while in the Main Screen
(the default screen that display the time/date). 

See **Key Bindings** section below.

## Screen Layout
### Main Screen
Displays the following information:

- Row 1: Time & Date
- Row 2: Alarm (if set), various status indicator (snooze, repeat, melody playing...)

The status indicator are shown in the second row at the rightmost corner. That includes: 

	A: Alarm Set
	C: Countdown timer Set
	R: Repeat Mode (melody will keep on sounding unless user press the key 'C' (cancel) or 'D' (snooze).
	S: Snooze Mode (cancel the melody and sound it again after snooze duration (settable in main menu).
	P: Countdown timer Pause
	!: Indicate that Melody is playing when alarm is triggered or countdown timer reaches 0

## Key Bindings
General key bindings

	'B': Back
	'C': Cancel
	'D': Delete (clear input)
	'#': Confirm

> **Note: The aforementioned general keys behaviour might be overwritten depending on the current state as follows:**

### Main Screen
	'*': Set Timer Mode (Countdown or Alarm)
	'#': Main Menu
	'1': Quick toggle Repeat mode
	'2': Quick toggle Snooze mode
	'4': Quick toggle Show Seconds on main screen
	'7': Turn the LCD (backlight and text display) on/off (default: on)
	'9': Melody Selector
	'0': View Log Mode
	'A': Start logging
	'B': Stop logging
	'C': Cancel active timer or countdown (before they are triggered)
		 Stop melody (if alarm or countdown timer is triggered)
	'D': Countdown timer preset,
	     Snooze, only if Snooze mode is activated

### Set Timer Mode
	'1': Set Countdown duration (in minutes and seconds. Press # to set as 0)
	'2': Set Alarm time (in 24 hours format, e.g. 2355)

> **Note: Only ONE timer mode can be active at a time. Activating one while other is active will disable it.**

### Melody Selector Mode
	'1': Star Wars Melody 1 (Imperial March)
	'2': Star Wars Melody 2 (Theme Song)
	'*': Preview Selected Melody (must select a melody first)
	'C': Cancel

> **Tips: You can press C to cancel previewing**

### Main Menu
	'1': Set the clock
	'2': Set repeat mode
	'3': Set snooze mode
	'4': Toggle time display in 24 hours or AMPM
	'5': Set keypad tone (on/off)
	'6': Set LCD Backlight (on/off)
	'7': Set auto countdown (on/off) (on: start countdown immediately when the countdown timer is set)

	'*': Previous page
	'#': Next Page

### Repeat Menu
	'1': Set Repeat status (on/off) (default: disabled)
	'2': Set Repeat duration (how many times to repeat the melody) (default: repeat forever) 
		 Set to '0' to repeat forever

> **Tips : You can quickly toggle the *Repeat* status by pressing *1* on the Main Screen**

### Snooze Menu
	'1': Set Snooze status (on/off) (default: disabled)
	'2': Set Snooze duration (how long to snooze in minutes) (default: 5 minutes)

> **Tips : You can quickly toggle the *Snooze* status by pressing *2* on the Main Screen**

### Log Mode
	'D': Delete all logs (a confirmation screen will appear next)
	'*': Previous log
	'#': Next log
> **Note : A confirmation menu will show when you are trying to delete all logs**

## Features
### Melody
There are four user selectable melody (only 1 is enabled by default) that can be played when the alarm/timer is triggered. 


> Note : The Uno will behave strangely when enabling more than 1 melody and when time logging is used due to its available memory left when running this program.

Go to the **USER DEFINABLE CONSTANTS** sections and comment/uncomment melody that you want. By default both Star Wars Melody are enabled.

    /**----------------------------------------------------------------------
     |	USER DEFINABLE CONSTANTS
     ----------------------------------------------------------------------**/
     ...
    // Melody (Tunes that play when alarm is set)
    // NOTE: Due to memory limitations only select songs that you want.
    // 	 Must have at least 1 MELODY chosen, no checking is done if there is no selected melody
    #define MELODY_1 		// Star Wars Theme 1
    //#define MELODY_2 		// Star Wars Theme 2
    // #define MELODY_3 	// Tetris, uses 1,568 bytes (76% in UNO)
    // #define MELODY_4 	// Ngan Cat, uses 1,868 bytes (91% in UNO)
    ...
> The default melody is MELODY_1 - Star Wars Imperial March


### Time Log
By default, the log entries is set to 3 due to the available memory left for the Uno. Setting it beyond 3 might result in some data  corruption when viewing the logs due to its available memory left. Only increase the number of logs if your Arduino has higher memory capacity, such as Arduino Mega 2560.

Logging a time is simple, just press A while in the Main Screen to start and B to cancel. You can then
view the log by clicking '0' while in the Main Screen.

	/**----------------------------------------------------------------------
	 |	LOGGING FEATURE
	 ----------------------------------------------------------------------**/
	// The number of Log entry to store. The oldest log is overwritten if the log is full
	// NOTE: Set this according to the memory of your device and your needs.
	// Future work might include assigning log name using T9 keypad
	#define LOG_SIZE 3
	...

> Note : By default, it will show the most recent logs when accessing the Time Log Screen (Pressing Key 0 in the Main Screen). You can disable this behaviour by always showing the first log by setting the following to false:
> 
    // Log
    // Whether to show the last log first in the LOG MENU.
    // If set to false, the log will show starting with entry #1
    bool showLastLogFirst = true;

### Key Tone
By default the key tone is played for each key press starting from 500 Hz and increment by 100Hz for each subsequent key in the following order: Key 0 (500Hz), Key 1 (750Hz)... Key A, Key B... Key * and Key #. Each sound will last for 250ms.

You change the tone as you like by referring to the *USER DEFINABLE CONSTANTS* section.
> The tone can be enabled/disabled via the Main Menu (Key 5).


### Other Customization
Please refer to the *USER DEFINABLE CONSTANTS* in *arduino-alarmtimerlogger.ino* to tweak some settings according to your preference.

# Limitations
- The smallest time accuracy unit is seconds. It is not meant to replace dedicated sport watch timer that provides sub-seconds accuracy for recording lap time, for instance. Think of it as a kitchen timer on steroids.
- Due to the memory restriction on the Arduino microconroller (especially the UNO with only 2,048 bytes
of memory allocatable to variables, only use a small number of melodies by commenting/uncommenting
the MELODY_<N> definition in the arduino-alarmtimerlogger.ino file. 
- Arduino UNO, for example, cannot occupy either the Nyan Cat or Tetris tunes due to the limited memory restrictions.


# Known Bugs
- None so far...


# Support / Suggestions
- Please send me an email at **dsync89@yahoo.com** for any bugs or suggestions.
- You are most welcome to fork and submit pull requests.

# Possible Future Features
Although it is possible to add more features, it might not be deployable on Arduino Uno due to its limited memory space. Currently it is occupying **91%** of the total memory. 

I might even create another repository if the newly added features cannot be supported for Uno. Following are some of the additional features that I might be adding depending on the spare time that I might have. Or you could actually contribute by forking it and make this project grows :-)

- Ability to assign different alarm to each day and automatically set them.
- Present some mathematical question (in three difficulties levels) when the alarm is triggered that you have to solve to disable it. This is great for those that want to wake up in semi- or full alert state.
- Assigning names to your log. This could be complicated since it requires implementing using T9 character input and could use some larger display like 4x20 to display the information.

# TODO (for Developers)
- Checking key input to prevent buffer overflow
- Character printing position that depends on the LCD type instead of hard coded cursor position
- Create appropriate function handler that handles each state instead of implementing the code in a big switch case statement.
- Prevent unnecessary LCD update, only perform lcd.print() when needed


# Credits
- Star Wars Melody 1 (Imperial March) and Star Wars Melody 2 (Main Theme) created by [MarieSpliid](http://courses.ischool.berkeley.edu/i262/s13/content/mariespliid/lab-5-star-wars-song-selector)
- Nyan Cat and Tetris Melody created by [electricmango](https://github.com/electricmango/Arduino-Music-Project)
- [Stino](https://github.com/Robot-Will/Stino) - A Sublime Text 2 plugin that provides Arduino like environment for editing, compiling and uploading sketches

# License
The code is available at [GitHub Project](https://github.com/dsync89/Arduino-AlarmTimerLogger) under **Creative Commons Attribution-ShareAlike 4.0 International (CC BY-SA 4.0) License**


**Creative Commons Attribution-ShareAlike 4.0 International (CC BY-SA 4.0)**

You are free to:

Share — copy and redistribute the material in any medium or format Adapt — remix, transform, and build upon the material for any purpose, even commercially. The licensor cannot revoke these freedoms as long as you follow the license terms.

Under the following terms:

Attribution — You must give appropriate credit, provide a link to the license, and indicate if changes were made. You may do so in any reasonable manner, but not in any way that suggests the licensor endorses you or your use. ShareAlike — If you remix, transform, or build upon the material, you must distribute your contributions under the same license as the original.

No additional restrictions — You may not apply legal terms or technological measures that legally restrict others from doing anything the license permits.

The full license is available at https://creativecommons.org/licenses/by-sa/4.0/legalcode

***Copyright (c) 2015 dsync89***

