#include <Wire.h>

#define SLAVE_ADDR 9
#define I2C_SDA 21
#define I2C_SCL 22

bool heater_power = true;
bool motor_power = true;
bool pump_power = true;

double temp;
double ph;
int rpm;

void setup() {
  // configure the pins
  Wire.begin (I2C_SDA, I2C_SCL);
  Serial.begin(9600);
}

void loop() {
  // request 6 bytes from slave
  Wire.requestFrom(SLAVE_ADDR, 6);
  while (Wire.available()) {
    temp = int(Wire.read()) * 256;
    temp += int(Wire.read());
    temp = temp / 100;
    
    rpm = int(Wire.read()) * 256;
    rpm += int(Wire.read());

    ph = int(Wire.read()) * 256;
    ph += int(Wire.read());
    ph = ph / 100;
  }
  
  // debugging print statements
  Serial.println(temp);
  Serial.println(rpm);
  Serial.println(ph);
  Serial.println();

  // send data to slave
  Wire.beginTransmission(SLAVE_ADDR);
  Wire.write(heater_power);
  Wire.write(motor_power);
  Wire.write(pump_power);
  Wire.endTransmission();  
  delay(500);
}
