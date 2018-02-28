#include "HardwareConfiguration.h"
#include "Logger.h"
#include "wire.h"

#ifdef LOG_TO_FLASH
#include "Driver_Flash.h"
#endif

#define N_CHARS_PER_FIELD 20 //'space pad' unused charecters so lines will all 'look nice'

//If Using Cache:
#ifdef LOG_USING_CACHE
#define LOG_CACHE_SIZE 1000 //Number of lines
String cache[LOG_CACHE_SIZE];
int cacheHead = 0;
#endif

float lineData[N_OF_LOG_FIELDS];
unsigned short logicState;
unsigned long timems;
String lineNote;
String  fNames;
int nFields;

//Deletes all data from line
void clearLine ()
{
	for (int i=0;i<N_OF_LOG_FIELDS;i++)
	{
		lineData[i]=0;
	}
	lineNote = "";
}

//If Compiling for QDUINOMINI, use LED desplay to indicate logic state
#ifdef ARDUINO_SAMD_FEATHER_M0_EXPRESS
#include <Adafruit_NeoPixel.h>
#define PIN8 8
Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, PIN8);
void LED_Init() {
  Wire.begin();
	strip.begin();
	strip.show(); // Initialize all pixels to 'off'
}
void LED_SetColor(unsigned short Color)
{
	const int dimmingFactor = 2; //% Brightness

	//Select Color
	int r = 0; int g = 0;  int b = 0; 
	switch (Color % 8)
	{
	case 0:		r = 255; g = 100; break; //Orange
	case 1:		r = 255; break; //Red
	case 2:		g = 255; break; //Green
	case 3:		b = 255; break; //Blue
	case 4:		r = 255; g = 050; b = 050; break; //Pink
	case 5:		r = 255; b = 255; break; //Purple
	case 6:		g = 255; b = 150; break; //Cyan
	case 7:		r = 255; g = 255; b = 255; break; //White
	}

	//Turn LED On
	strip.setPixelColor(0, 
		(r*dimmingFactor)/100,
		(g*dimmingFactor)/100, 
		(b*dimmingFactor)/100);
    strip.show();
}
#else
//No LED, No desplay
void LED_Init() {}
void LED_SetColor(unsigned short Color) {};
#endif

//Private functions for hardware interactions for logging
/////////////////////////////////////////////////////////
void logHWInit()
{
#ifdef LOG_TO_FLASH
	Flash_Init();
#else
	Serial.begin(9600);

#ifdef LOG_USING_CACHE
	//If using Cache initialization is done only when serial is connected
	//Otherwise data will be 'spit out' before initialization complete
	while (!Serial)
	{
		delay(100);
	}
	for (int i = 0; i < 4; i++)
	{
		Serial.println("Start Printing Cache");
		delay(1000);
	}
#endif 

#endif
}
void logHWClose()
{
#ifdef defined LOG_TO_FLASH
	Flash_Close();
#endif
}
void logHWWrite(String message)
{
#ifdef LOG_TO_FLASH
	Flash_Log(message);
#else
	Serial.println(message);
#endif
}
/////////////////////////////////////////////////////////

void Log_Init ()
{
#ifndef LOG_USING_CACHE
	logHWInit();
#else
	//If Cache is not used, no need to initialize hardware at this point only at the end
	Serial.begin(9600); // Initialize comm port for reporting cache errors
	cacheHead = 0;
#endif

	LED_Init(); //If LED is available, display logic state as LED color

	//Initiate filed
	fNames="";
	clearLine();
	nFields = 0;
}

void logWrite(String message)
{
#ifndef LOG_USING_CACHE
	logHWWrite(message);
#else
	//Using Cache
	if (cacheHead < LOG_CACHE_SIZE)
	{
		cache[cacheHead] = message;
		cacheHead++;
	}
	else
	{
		Serial.println("Log cache has run out of memory!");
	}
#endif

}

void Log_Close ()
{
	LED_Init(); //Shut down LED
	logWrite("Closing Log");

#ifndef LOG_USING_CACHE
	logHWClose();
#else
	//If Cache is not used, open HW interface, write all messages and close
	logHWInit();
	for (int i = 0; i < cacheHead; i++)
		logHWWrite(cache[i]);
	logHWClose();
#endif
	
}

//'space pad' unused charecters so lines will all 'look nice'
String padWithSpaces(String str)
{
	String out = str;
	while (out.length() < N_CHARS_PER_FIELD)
		out = out + " ";
	return out;
}

void Log_DefineNextField (String fName, String fUnit)
{
	if (nFields >= (N_OF_LOG_FIELDS))
		logWrite("Too many loged fields, consider increasing N_OF_LOG_FIELDS");

	fNames += padWithSpaces(fName + " [" + fUnit + "],");
	nFields++;

}
void Log_SetData (int fI, float data)
{
	if (fI >= (N_OF_LOG_FIELDS))
		logWrite("Too many loged fields, consider increasing N_OF_LOG_FIELDS");
	else
		lineData[fI] = data;
}
void Log_SetLoigcState(unsigned short newState)
{
	logicState = newState;
	LED_SetColor(newState); //If LED is available, display logic state as LED color
}
void Log_SetTime(unsigned long time){timems= time;}
void Log_AddNote(String note) {lineNote += note + ".";}

void Log_WriteLogHeader () //Write log header
{
	//Header
	String message = padWithSpaces("Time [msec],") + fNames + padWithSpaces("LogicState,") + "Notes";
	logWrite(message);
}

void Log_WriteLine ()
{
	String message = padWithSpaces(String(timems) + ",");
	for (int i=0;i<nFields;i++)
		message += padWithSpaces(String(lineData[i]) + ",");

	message += padWithSpaces(String(logicState) + ",") + lineNote;
	logWrite(message);
	clearLine();
}

//This function implements the log tester configuration
//The idea is to print a demo log
void Log_Test ()
{
	Log_Init();

	logWrite("Hello World!");

	Log_DefineNextField("Speed","m/sec");
	Log_DefineNextField("Length","m");
	Log_DefineNextField("Area","m^2");

	Log_WriteLogHeader();

	Log_SetTime(10);
	Log_SetLoigcState(10);
	Log_SetData(0,-1);   //Speed
	Log_SetData(1,2.4); //Length
	Log_SetData(2,1);  //Area
	Log_AddNote("TestData");

	Log_WriteLine();

  Log_Close();
}

void LED_Test() 
{
    LED_Init();
    for (int i = 0; i < 8; i++) 
	{
		LED_SetColor(i);
        delay(1000);
    }
}
