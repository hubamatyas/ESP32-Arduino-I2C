# ESP32-Thingsboard-IoT-Dashboard

This repo contains
- Source code to set up a wired, I2C, connection between an ESP32 and an Arduino. Then using I2C the two devices can trasfer integers and floats between each other. See `esp32_master` and `arduino_slave`.
- Source code to set up a wireless, MQTT, connection betwen an ESP32 and Thingsboard (a cloud enabled dashboard).

### I2C
I2C connection helps transfer integers and floats between ESP32 and Ardunio. `Wire.Write` function can only transfer integers with the maximum value of 255 (i.e., the size of one byte), therefore integers and floating-point numbers are encoded using binary encoding where every two bytes represented a number. Floats were first converted to integers by multiplying them by 100 (multiply accordingly if you have higher precision floats) and were only then encoded.

### MQTT
Thingsboard connects to the ESP32 via the MQTT protocol to achieve fast and reliable two-way communication. This allows the ESP32 to subscribe to topics provided by Thingsboard and publish data to them. Via Remote Procedure Calls (RPC) the ESP32 can send data to and receive data from Thingsboard (and vice versa). In `esp32_thingsboard` there are three examples metrics that Thingsboard displays.
- Temperature (°C)
- RPM
- Ph value

### Thingsboard
Each metric has its own control widgets, knob controls, and their time series charts. When a user changes the value using the knob, Thingsboard sends commands to the ESP32, which then forwards the values to the Arudino. The Ardunio then sets the specified values in its subsystems and sends back telemetry data (via the ESP32) to Thingsboard.

![Thingsboard dashboard](https://github.com/hubamatyas/ESP32-Thingsboard-IoT-Dashboard/blob/main/dashboard.png)

### Architecture
![System architecture](https://github.com/hubamatyas/ESP32-Thingsboard-IoT-Dashboard/blob/main/architecture.png)

