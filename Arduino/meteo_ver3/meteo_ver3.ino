/***********************************************************************
 * 
 * Arduino Meteo station.
 * Ver. 3: Read values from temperature/humidity sensor DHT12, use 
 *         software UART library for ESP8266 WiFi module and send data 
 *         to ThingSpeak cloud.
 * ATmega328P (Arduino Uno), 16 MHz, Arduino IDE 1.8.13
 *
 * Copyright (c) 2020-Present Tomas Fryza
 * Dept. of Radio Electronics, Brno University of Technology, Czechia
 * This work is licensed under the terms of the MIT license.
 *
 **********************************************************************/

/* Includes ----------------------------------------------------------*/
// Wire library allows you to communicate with I2C/TWI devices
#include <Wire.h>
#include <SoftwareSerial.h>


/* Global variables --------------------------------------------------*/
// Air temperature in the form T0.T1, such as 21.3
unsigned char t0 = 0;
unsigned char t1 = 0;
// Relative humidity in the form H0.H1, such as 25.7
unsigned char h0 = 0;
unsigned char h1 = 0;

// Arduino pin  ESP pin  Arduino point of view
//     2          Tx     Receive data from ESP
//     3          Rx     Transmit data to ESP
SoftwareSerial espSerial(2, 3);

// ENTER YOUR VALUES
String ssid        = "xxx"; // SSID of your WiFi network
String password    = "xxx"; // Password of your WiFi network
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

    // Setup UART communication with Serial monitor in Arduino IDE
    Serial.begin(9600);

    // Setup UART communication with WiFi module
    espSerial.begin(115200);

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
    // Test WiFi module
    espData("AT", 1000);

    // Set mode to STA
    espData("AT+CWMODE=1", 2000);

    // Login to WiFi network and get IP address
    espData("AT+CWJAP=\""+ ssid +"\",\""+ password +"\"", 5000);
}


/**********************************************************************
 * Function: wifiSend()
 * Purpose:  Create a GET request and send data to the server.
 * Returns:  none
 **********************************************************************/
void wifiSend()
{
    // Start communication with ThingSpeak server
    espData("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",80", 1000);

    // Prepare request including temperature/humidity data
    // GET /update?api_key=xxx&field1=21.6&field2=19.4
    String cmd = "GET /update?api_key=";
    cmd = cmd + writeApiKey;
    cmd = cmd + "&field1=";
    cmd = cmd + String(t0);
    cmd = cmd + ".";
    cmd = cmd + String(t1);
    cmd = cmd + "&field2=";
    cmd = cmd + String(h0);
    cmd = cmd + ".";
    cmd = cmd + String(h1);

    // Send number of bytes first. Note that +2 is added for "\r\n"
    espData("AT+CIPSEND=" +String(cmd.length()+2), 100);

    // Send request including temperature/humidity data
    espData(cmd, 100);
}


/**********************************************************************
 * Function: espData()
 * Purpose:  Send AT command to EST8266 module and wait for response.
 * Returns:  response - Response of ESP8266
 **********************************************************************/
String espData(String command, const int timeout)
{
    // Just display info to Serial monitor
    Serial.print("AT Command ==> ");
    Serial.print(command);
    Serial.println("     ");

    // Use software serial and send AT command to ESP8266 module
    String response = "";
    espSerial.println(command);

    // Wait for timeout and read response from receiver pin
    long int time = millis();
    while ((time + timeout) > millis())
    {
        while (espSerial.available())
        {
            char c = espSerial.read();
            response += c;
        }
    }
    return response;
}

