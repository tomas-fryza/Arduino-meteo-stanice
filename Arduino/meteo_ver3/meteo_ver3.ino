/***********************************************************************
 * 
 * Arduino Meteo station.
 * Ver. 3: Read values from temperature/humidity sensor DHT12, use 
 *         functions from ESP8266 WiFi library and send data to ThingSpeak 
 *         cloud.
 * ATmega328P (Arduino Uno), 16 MHz, Arduino IDE 1.8.13
 *
 * Copyright (c) 2020-Present Tomas Fryza
 * Dept. of Radio Electronics, Brno University of Technology, Czechia
 * This work is licensed under the terms of the MIT license.
 *
 **********************************************************************/

/* Includes ----------------------------------------------------------*/
// Import required libraries
// Wire library allows you to communicate with I2C/TWI devices
// (see https://www.arduino.cc/en/reference/wire)
#include <Wire.h>
//TODO: Použít knihovnu přímo pro DHT12 senzor namísto obecné I2C Wire?

#include <SoftwareSerial.h>


/* Global variables --------------------------------------------------*/
// Air temperature in the form TEMP0.TEMP1, such as 21.3
unsigned char temp0 = 0;
unsigned char temp1 = 0;
// Relative humidity in the form HUMID0.HUMID1, such as 25.7
unsigned char humid0 = 0;
unsigned char humid1 = 0;

// SSID of your WiFi network
String ssid = "";
// Password of your WiFi network
String password = "";
// Write API Key from ThingSpeak cloud
String writeApiKey = "";


/* Functions ---------------------------------------------------------*/
/**********************************************************************
 * Function: setup()
 * Purpose:  Setup function where the program execution begins. Init 
 *           serial communications (I2C/TWI, UART) and WiFi module.
 * Returns:  none
 **********************************************************************/
void setup()
{
    // Setup I2C/TWI communication with the Temp/Humid sensor
    Wire.begin();
    // Setup UART communication with WiFi module
    Serial.begin(115200);
    // Setup WiFi module and get IP address from your router
    wifiSetup();
}

/**********************************************************************
 * Function: loop()
 * Purpose:  Infinite loop. This function is executed over and over 
 *           again. Ask the temperature/humidity sensor for new data 
 *           and send it to the UART. Use Serial Monitor to view it.
 * Returns:  none
 **********************************************************************/
void loop()
{
    // Ask sensor for new data
    getHumidTempData();

    // Send temperature/relative humidity data to ThingSpeak cloud
    wifiSend();

    // Wait 2 minutes (120,000 milisecs) and then continue
    delay(120000);
}

/**********************************************************************
 * Function: getHumidTempData()
 * Purpose:  Perform I2C/TWI communication with the DHT12 sensor and
 *           request data on humidity (2 bytes) and temperature (2 bytes).
 * Returns:  none
 **********************************************************************/
void getHumidTempData()
{
    // Begin communication with a sensor whose slave address is 0x5c
    Wire.beginTransmission(0x5c);
    // Set the internal address in the sensor to address 0
    Wire.write(byte(0x00));
    // Stop communication
    Wire.endTransmission();

    // Begin communication again and request 4 bytes from slave device
    Wire.requestFrom(0x5c, 4);
    // If 4 bytes were received, store them in global variables
    if (4 <= Wire.available())
    {
        humid0 = Wire.read();
        humid1 = Wire.read();
        temp0  = Wire.read();
        temp1  = Wire.read();
    }
}

/**********************************************************************
 * Function: wifiSetup()
 * Purpose:  Get IP address and connect to WiFi network.
 * Returns:  none
 **********************************************************************/
void wifiSetup()
{
    // Command to get IP address
    String cmd = "AT+CWJAP=\"" +ssid+ "\",\"" +password+ "\"";
    
    // Test WiFi module
    Serial.println("AT"); delay(1000);
    // Set mode to STA
    Serial.println("AT+CWMODE=1"); delay(2000);
    // Login to WiFi network and get IP address
    Serial.println(cmd); delay(5000);
}

/**********************************************************************
 * Function: wifiSend()
 * Purpose:  Create a GET request and send data to the server.
 * Returns:  none
 **********************************************************************/
void wifiSend()
{
    // Command to WiFi module
    String cmd = "AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",80";
    // Start communication with ThingSpeak server
    Serial.println(cmd); delay(500);

    // Prepare request including temperature/humidity data
    // GET /update?api_key=xxx&field1=21.6&field2=19.4
    cmd = "GET /update?api_key=";
    cmd = cmd + writeApiKey;
    cmd = cmd + "&field1=";
    cmd = cmd + String(temp0);
    cmd = cmd + ".";
    cmd = cmd + String(temp1);
    cmd = cmd + "&field2=";
    cmd = cmd + String(humid0);
    cmd = cmd + ".";
    cmd = cmd + String(humid1);
    cmd = cmd + "\r\n";

    // Send number of bytes first
    Serial.print("AT+CIPSEND=");
    Serial.println(cmd.length()); delay (100);

    // Send request including temperature/humidity data
    Serial.print(cmd);
}
