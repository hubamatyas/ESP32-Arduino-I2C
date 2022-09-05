#include <Wire.h>

#define SLAVE_ADDR 9

bool heater_power;
bool motor_power;
bool pump_power;

double temp;
double ph;
int rpm;

double target_temp;
double target_ph;
int target_rpm;

void setup() {
  Wire.begin(SLAVE_ADDR);

  // register event
  Wire.onRequest(requestEvent); 
  Wire.onReceive(receiveEvent);
  Serial.begin(9600);
}

void loop() {
  delay(100);
  //rpm++;
  //temp += 0.15;
  //ph += 0.1;
}

// encodes integers to fit 1 byte Wire.write()
void encode(double num, int* ptr) {
  *ptr = int(num/256);
  *(ptr+1) = int(num)%256;
}

// send data whenever it's requested by master
void requestEvent() {
  int* temp_encoded;
  int* rpm_encoded;
  int* ph_encoded;
  encode(temp*100, temp_encoded);
  encode(rpm, rpm_encoded);
  encode(ph*100, ph_encoded);
  
  Wire.write(*temp_encoded);
  Wire.write(*(temp_encoded+1));
  Wire.write(*rpm_encoded);
  Wire.write(*(rpm_encoded+1));
  Wire.write(*ph_encoded);
  Wire.write(*(ph_encoded+1));
}

// receive data from master
void receiveEvent(int howMany)
{
  while (Wire.available()) {
    target_temp = int(Wire.read()) * 256;
    target_temp += int(Wire.read());
    target_rpm = int(Wire.read()) * 256;
    target_rpm += int(Wire.read());
    target_ph = int(Wire.read()) * 256;
    target_ph += int(Wire.read());
  }
  
  target_temp = target_temp / 100;
  target_ph = target_ph / 100;

  // print statements for debugging
  Serial.println(target_temp);
  Serial.println(target_rpm);
  Serial.println(target_ph);
  Serial.println();
}
