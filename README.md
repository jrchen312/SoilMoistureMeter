# Soil Moisture Meter

### Purpose
The Soil Moisture Meter is a product that is intended to obtain the soil moisture of a homeowner's lawn, and then predict how frequently the lawn should be watered. The soil moisture is measured with a network of soil moisture sensors, and the data is sent by radios to a "home hub". The homehub processes the data, before displaying it on it's LCD screen. A Bluetooth app can also connect to the Arduino, where further data can be stored and analyzed. 

### Design
The Soil Moisture Meter's home hub is shown below. It has a LCD screen, as well as buttons for user interaction. There are other sensors and devices on board, such as a temperature sensor and Bluetooth module. It is currently controlled by two Arduino Nanos using i2c communication. 
![homeHub](https://github.com/jrchen312/SoilMoistureMeter/blob/main/images/HomeHub.jpeg)

The sensors are shown below. These sensors periodically collect soil moisture data before using a nrf24l01 radio to send it to the home hub (range of at least 20 meters). They are simplistic and are battery powered, with a battery life of over 3 years (they have an idle battery consumption of just a few microAmps). 
![soilSensor](https://github.com/jrchen312/SoilMoistureMeter/blob/main/images/Sensor.jpeg)

The PCBs for the sensors are shown below. These sensors use ATMega328p chips, and model the Arduino boards, except any excess power consuming components have been stripped. The board has a soil moisture sensor and a nrf24l01 radio, and is powered with two AA batteries. The ATMega chip is easily programmed with SPI by using the pin headers labeled "10, 11, 12, 13". 

![soilPCB](https://github.com/jrchen312/SoilMoistureMeter/blob/main/images/SensorPCB.png)



Finally, the device is able to communicate with a Bluetooth app, which is able to display all of the data from the comfort of the user's smartphone/tablet. The app also has the capability of storing any extra data, displaying it in a chart.

![homePage](https://github.com/jrchen312/SoilMoistureMeter/blob/main/images/appHome.png)

![secPage](https://github.com/jrchen312/SoilMoistureMeter/blob/main/images/appData.png)
