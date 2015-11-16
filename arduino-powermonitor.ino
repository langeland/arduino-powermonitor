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

/***********************************************************************************************
 ** Interrupt timing
 ***********************************************************************************************/
// the debounce time; increase if the output flickers
const unsigned long interruptDebounceDelay = 500;

// Time of last sensor interrupt (ms)
volatile unsigned long interruptTimeLast = 0;

// Time between the two latest interrupts (ms)
volatile unsigned int interruptTimeDelta = 0;

/***********************************************************************************************
 ** Log timing
 ***********************************************************************************************/

// Time between log calls (ms)
const unsigned long logTimeInterval = 60000;

// Time of last log call (ms)
volatile unsigned long logTimeLast = 0;

// Time between the two latest log calls (ms)
volatile unsigned int logTimeDelta = 0;

/***********************************************************************************************
 **
 ***********************************************************************************************/

// Number of pulses, since last log call
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

	// We start by connecting to a WiFi network
	Serial.println();
	Serial.println();
	Serial.print("Connecting to ");
	Serial.println(ssid);

	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED) {
		Serial.print(".");
		delay(500);
	}

	delay(1000);

	Serial.println("");
	Serial.println("WiFi connected");
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());
	Serial.println();
	Serial.println();

	// Start sensor
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
	noInterrupts();
	logTimeDelta = millis() - logTimeLast;

	/***********************************************************************************************
	 **
	 ***********************************************************************************************/
	// Calculate powerAverage over the last log interval
	// This is not correct needs to take logTimeDelta in account
	if(logTimeDelta > 0) {
		powerAverage = ((float)interruptCount * 3600000) / logTimeDelta;
	}

	// Calculate powerCurrent based on the time between the last two interrupts (interruptTimeDelta)
	if(interruptTimeDelta > 0) {
		powerCurrent = 3600000 / (float)interruptTimeDelta;
	}

	/***********************************************************************************************
	 **
	 ***********************************************************************************************/
	Serial.print("Starting data logging @ ");
	Serial.println(millis());

	Serial.print(" -- interruptCount:");
	Serial.println(interruptCount);

	Serial.print(" -- interruptCountTotal:");
	Serial.println(interruptCountTotal);

	Serial.print(" -- logTimeDelta:");
	Serial.println(logTimeDelta);

	Serial.print(" -- powerAverage:");
	Serial.println(powerAverage);

	Serial.print(" -- powerCurrent:");
	Serial.println(powerCurrent);	

	/***********************************************************************************************
	 **
	 ***********************************************************************************************/
	Serial.print(" -- connecting to: ");
	Serial.println(host);

	// Use WiFiClient class to create TCP connections
	WiFiClient client;
	const int httpPort = 8080;
	if (!client.connect(host, httpPort)) {
		Serial.println(" -- connection failed");
		return;
	}

	// We now create a URI for the request
	String PostData = "data={";
	PostData += "  \"node\": {";
	PostData += "    \"identifier\": " + String(ESP.getChipId()) + ",";
	PostData += "    \"data\": {";
	PostData += "      \"interruptCount\": " + String(interruptCount) + ",";
	PostData += "      \"interruptCountTotal\": " + String(interruptCountTotal) + ",";
	PostData += "      \"logTimeDelta\": " + String(logTimeDelta) + ",";
	PostData += "      \"powerAverage\": " + String(powerAverage) + ",";
	PostData += "      \"powerCurrent\": " + String(powerCurrent) + ",";
	PostData += "      \"uptime\": " + String(millis()/1000) + ",";
	PostData += "      \"freeHeap\": " + String(ESP.getFreeHeap()) + ",";
	PostData += "      \"vcc\": " + String(ESP.getVcc());
	PostData += "    }";
	PostData += "  }";
	PostData += "}";

	client.println("POST " + String(endpoint) + " HTTP/1.1");
	client.println("Host: " + String(host));
	client.println("User-Agent: Arduino/1.0");
	client.println("Content-Type: application/x-www-form-urlencoded");
	client.println("Connection: close");
	client.print("Content-Length: ");
	client.println(PostData.length());
	client.println();
	client.println(PostData);
	delay(100);

	// Read all the lines of the reply from server and print them to Serial
	while(client.available()){
		String line = client.readStringUntil('\r');
		Serial.print("   ");
		Serial.print(line);
	}

	Serial.println();
	Serial.println("-- closing connection");
	Serial.println();
	Serial.println();
	
	/***********************************************************************************************
	 **  Reset counter and timer
	 ***********************************************************************************************/
	interruptCount = 0;
	logTimeLast = millis();
	interrupts();
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

