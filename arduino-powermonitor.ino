/***********************************************************************************************
 **
 ***********************************************************************************************/
#include <ESP8266WiFi.h>
#include "Settings.h"
ADC_MODE(ADC_VCC);

/***********************************************************************************************
 **
 ***********************************************************************************************/
#define SENSOR_INPUT_PIN 15
#define SENSOR_POWER_PIN 13
#define SENSOR_PPKWH 1000

/***********************************************************************************************
 ** Interrupt timing
 ***********************************************************************************************/
// the debounce time; increase if the output flickers
const unsigned long interruptDebounceDelay = 500;

// Time of last sensor interrupt
volatile unsigned long interruptTimeLast = 0;

// Time between the two latest interrupts (ms)
volatile unsigned int interruptTimeDelta = 0;

/***********************************************************************************************
 ** Log timing
 ***********************************************************************************************/

// Time between log calls
const unsigned long logTimeInterval = 10000;

// Time of last log call
volatile unsigned long logTimeLast = 0;

// Time between the two latest log calls (ms)
volatile unsigned int logTimeDelta = 0;

/***********************************************************************************************
 **
 ***********************************************************************************************/

//Number of pulses, used to measure energy.
volatile unsigned int interruptCount = 0;   

volatile unsigned int interruptCountTotal = 0;

/***********************************************************************************************
 **
 ***********************************************************************************************/

// Power average over the time of the log interval
volatile float powerAverage = 0.00;

// Power calculated over the last two sensor interupts
volatile float powerCurrent = 0.00;


/***********************************************************************************************
 **
 ***********************************************************************************************/



void setup() {
	Serial.begin(115200);
	delay(1000);

	// We start by connecting to a WiFi network
	Serial.println();
	Serial.println();
	Serial.println("/***********************************************************************************************");
	Serial.println(" ** PowerMonitor v5.0.1");
	Serial.println(" ***********************************************************************************************/");
	Serial.println("");
	
	pinMode(SENSOR_POWER_PIN, OUTPUT);
	digitalWrite(SENSOR_POWER_PIN, HIGH);

	Serial.println("  Attaching interrupt");
	attachInterrupt(digitalPinToInterrupt(SENSOR_INPUT_PIN), sensorInterrupt, RISING);
	Serial.println("  Sensor power on");	
	digitalWrite(SENSOR_POWER_PIN, HIGH);
	Serial.println("##########################");
}


void loop() {
	// Send sensor data
	if(millis() - logTimeLast >= logTimeInterval) {
		logData();
	}

	delay(10);
}



void logData(){
	// digitalWrite(SENSOR_POWER_PIN, LOW);
	noInterrupts();

	logTimeDelta = millis() - logTimeLast;

	/***********************************************************************************************
	 **
	 ***********************************************************************************************/
	// Calculate powerAverage over the last log interval
	powerAverage = ((float)interruptCount / 3600) * 100000;
	
	
	// TODO: Calculate powerCurrent
	if(interruptTimeDelta > 0) {
		powerCurrent = (3600 / (float)interruptTimeDelta) / SENSOR_PPKWH;
	}
	


	/***********************************************************************************************
	 **
	 ***********************************************************************************************/
	Serial.print("logData @ ");
	Serial.println(millis());

	Serial.print(" -- interruptCount:");
	Serial.println(interruptCount);

	Serial.print(" -- logTimeDelta:");
	Serial.println(logTimeDelta);

	Serial.print(" -- powerAverage:");
	Serial.println(powerAverage);
	Serial.println();
	Serial.print(" -- powerCurrent:");
	Serial.println(powerCurrent);

	
	/*
	// TODO: Calculate powerAverage
	powerAverage = (interruptCount / 3600) * 100000;
	
	// TODO: Calculate powerCurrent
	if(interruptDelta > 0){
		powerCurrent = (3600 / interruptDelta) / SENSOR_PPKWH;
	}

	

	Serial.println("#####################################################");
	Serial.print("powerAverage: ");
	Serial.println(powerAverage);
	
	
	
	Serial.print("      Counts: ");
	Serial.println(interruptCount);
	


	*/
	Serial.println();
	
	interruptCount = 0;



	logTimeLast = millis();
	interrupts();
	// digitalWrite(SENSOR_POWER_PIN, HIGH);
}


/***********************************************************************************************
 **
 ***********************************************************************************************/

/**
 * The interrupt routine
 **/
void sensorInterrupt() {
	if((millis() - interruptTimeLast) > interruptDebounceDelay) {
		Serial.print("sensorInterrupt @ ");
		Serial.println(millis());

		interruptCount++;
		interruptCountTotal++;
		interruptTimeDelta = millis() - interruptTimeLast;
		interruptTimeLast = millis();
	}
}
