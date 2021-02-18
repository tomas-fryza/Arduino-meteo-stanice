/*

*/
#include <Wire.h>

unsigned char temp1 = 0;
unsigned char temp2 = 0;
unsigned char humd1 = 0;
unsigned char humd2 = 0;

void setup()
{
  Wire.begin();       // Init I2C communication with Temp/Humd sensor
  Serial.begin(9600); // Init UART communication with speed of 9600 Bd
}


void loop()
{
  getHumdTemp();
//  getTemp();
//  getHumd();

  // Print sensor data to Serial Monitor
  Serial.print("Temperature: ");
  Serial.print(temp1); Serial.print("."); Serial.print(temp2); Serial.println(" deg");
  Serial.print("Humidity   : ");
  Serial.print(humd1); Serial.print("."); Serial.print(humd2); Serial.println(" %");
  Serial.println(" ");

  delay(2000);   // Wait for XXX secs
}


void getHumdTemp()
{
  Wire.beginTransmission(0x5c); // transmit to temp/humid. slave device
  Wire.write(byte(0x00));       // sets register pointer to register 0x00
  Wire.endTransmission();       // stop transmitting

  Wire.requestFrom(0x5c, 4);    // request 4 bytes from slave device 0x5c

  if (4 <= Wire.available()) {  // if 4 bytes were received
    humd1 = Wire.read();
    humd2 = Wire.read();
    temp1 = Wire.read();
    temp2 = Wire.read();
  }
}

void getTemp()
{
  Wire.beginTransmission(0x5c); // transmit to temp/humid. slave device
  Wire.write(byte(0x02));       // sets register pointer to register 0x02
  Wire.endTransmission();       // stop transmitting

  Wire.requestFrom(0x5c, 2);    // request 2 bytes from slave device 0x5c

  if (2 <= Wire.available()) {  // if 2 bytes were received
    temp1 = Wire.read();
    temp2 = Wire.read();
  }
}

void getHumd()
{
  Wire.beginTransmission(0x5c); // transmit to temp/humid. slave device
  Wire.write(byte(0x00));       // sets register pointer to register 0x00
  Wire.endTransmission();       // stop transmitting

  Wire.requestFrom(0x5c, 2);    // request 2 bytes from slave device 0x5c

  if (2 <= Wire.available()) {  // if 2 bytes were received
    humd1 = Wire.read();
    humd2 = Wire.read();
  }
}
