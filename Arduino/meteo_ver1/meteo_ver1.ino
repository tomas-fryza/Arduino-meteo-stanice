/***********************************************************************
 * 
 * Arduino Meteo station.
 * Ver. 1: Reading values from temperature/humidity sensor DHT12.
 * ATmega328P (Arduino Uno), 16 MHz, Arduino IDE 1.8.13
 *
 * Copyright (c) 2018-Present Tomas Fryza
 * Dept. of Radio Electronics, Brno University of Technology, Czechia
 * This work is licensed under the terms of the MIT license.
 *
 **********************************************************************/

/* Includes ----------------------------------------------------------*/
// Wire library allows you to communicate with I2C/TWI devices
// (see https://www.arduino.cc/en/reference/wire)
#include <Wire.h>


/* Global variables --------------------------------------------------*/
// Air temperature in the form T0.T1, such as 21.3
unsigned char t0 = 0;
unsigned char t1 = 0;
// Relative humidity in the form H0.H1, such as 25.7
unsigned char h0 = 0;
unsigned char h1 = 0;


/* Functions ---------------------------------------------------------*/
/**********************************************************************
 * Function: setup()
 * Purpose:  Setup function where the program execution begins. Set 
 *           serial communications: I2C/TWI, UART.
 * Returns:  none
 **********************************************************************/
void setup()
{
    // Setup I2C/TWI communication with the Temp/Humid sensor
    Wire.begin();

    // Setup UART communication for Serial Monitor
    Serial.begin(9600);
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

    // Send temperature/relative humidity data to UART
    Serial.print("Temperature: ");
    Serial.print(t0); Serial.print("."); Serial.print(t1); Serial.println(" deg");
    Serial.print("Humidity   : ");
    Serial.print(h0); Serial.print("."); Serial.print(h1); Serial.println("% RH");
    Serial.println(" ");  // Just a new empty line

    // Wait 3 seconds (3000 milisecs) and then continue
    delay(3000);
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
