/***********************************************************************
 * 
 * Arduino Meteo station.
 * Ver. 2: Reading values from temperature/humidity sensor DHT12 and
 *         sending them to ThingSpeak cloud via WiFi module ESP8266.
 * ATmega328P (Arduino Uno), 16 MHz, Arduino IDE 1.8.13
 *
 * Copyright (c) 2018-Present Tomas Fryza
 * Dept. of Radio Electronics, Brno University of Technology, Czechia
 * This work is licensed under the terms of the MIT license.
 *
 **********************************************************************/

/* Includes ----------------------------------------------------------*/
// Wire library allows you to communicate with I2C/TWI devices
#include <Wire.h>


/* Global variables --------------------------------------------------*/
// Air temperature in the form T0.T1, such as 21.3
unsigned char t0 = 0;
unsigned char t1 = 0;
// Relative humidity in the form H0.H1, such as 25.7
unsigned char h0 = 0;
unsigned char h1 = 0;

// ENTER YOUR VALUES
String ssid = "xxx";        // SSID of your WiFi network
String password = "xxx";    // Password of your WiFi network
String writeApiKey = "xxx"; // Write API Key from ThingSpeak cloud


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
    delay(60000);
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
        h0 = Wire.read();
        h1 = Wire.read();
        t0 = Wire.read();
        t1 = Wire.read();
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
    cmd = cmd + String(t0);
    cmd = cmd + ".";
    cmd = cmd + String(t1);
    cmd = cmd + "&field2=";
    cmd = cmd + String(h0);
    cmd = cmd + ".";
    cmd = cmd + String(h1);
    cmd = cmd + "\r\n";

    // Send number of bytes first
    Serial.print("AT+CIPSEND=");
    Serial.println(cmd.length()); delay (100);

    // Send request including temperature/humidity data
    Serial.print(cmd);
}
