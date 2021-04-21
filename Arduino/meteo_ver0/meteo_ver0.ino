/***********************************************************************
 * 
 * Arduino Meteo station.
 * Ver. 0: I2C scan and detect all connected devices.
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
uint8_t addr = 0;      // Default I2C slave address
uint8_t noDevices = 0; // Number of detected slave devices


/* Functions ---------------------------------------------------------*/
/**********************************************************************
 * Function: setup()
 * Purpose:  Setup function where the program execution begins. Set 
 *           serial communications: I2C/TWI, UART.
 * Returns:  none
 **********************************************************************/
void setup()
{
    // Setup I2C/TWI communication
    Wire.begin();

    // Setup UART communication for Serial Monitor
    Serial.begin(9600);
    Serial.println("Scan I2C-bus for slave devices:");
    Serial.println("");
    Serial.println("      .0 .1 .2 .3 .4 .5 .6 .7 .8 .9 .a .b .c .d .e .f");
    Serial.print("0x0.:");
}

/**********************************************************************
 * Function: loop()
 * Purpose:  Infinite loop. This function is executed over and over 
 *           again. Step-by-step verification of all I2C addresses of 
 *           slave devices. Serial Monitor is used to view scanning 
 *           process/results.
 * Returns:  none
 **********************************************************************/
void loop()
{
    // Number of bytes returned from the slave device
    uint8_t noBytes = 0;

    // Request 1 byte from slave device/address
    noBytes = Wire.requestFrom(addr, 1);

    // A response was received from the slave device
    if (noBytes >= 1)
    {
        noDevices = noDevices + 1;
        Serial.print(" ");
        Serial.print(addr, HEX);
    }
    else
    {
        Serial.print(" --");
    }

    // Wait 50 milisecs and then continue
    delay(50);

    // Increment to next address
    addr = addr + 1;
    // If all addresses has been tested, loop here forever
    if (addr >= 128)
    {
        Serial.println("");
        Serial.println("");
        Serial.print("Number of detected devices: ");
        Serial.println(noDevices);
        while(1);
    }
    if ((addr % 16) == 0)
    {
        Serial.println("");
        Serial.print("0x");
        Serial.print(addr / 16);
        Serial.print(".:");
    }
}

// Several known devices:
//----------------------------------
// 0x57: Serial EEPROM
// 0x5C: humid/temp sensor
// 0x68: RTC
// 0x76: temp/pressure sensor BMP280
