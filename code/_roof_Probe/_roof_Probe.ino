//
//Rooftop arduino code
//
//This arduino collects the rain, shading, pressure, humidity, and temperature
// with its sensors and then sends it to the main arduino using the
// nrf24l01 radio. 

// --------------------------------------------------------------------
// basic startup code:
//
// --------------------------------------------------------------------




  // this is the rain moisture sensor
  int rainVal = 0;
  int rainPin = A1;
  int rainPower = 4;



//bme
  #include <Wire.h>
  #include <SPI.h>
  #include <Adafruit_Sensor.h>
  #include <Adafruit_BME280.h>

  #define SEALEVELPRESSURE_HPA (1023.25)

   Adafruit_BME280 bme; // I2C


  

// lower power mode
  #include <LowPower.h> //library


//wireless network stuff
  #include <RF24Network.h>
  #include <RF24.h>
  #include <SPI.h>

  RF24 radio(9, 10);               // nRF24L01 (CE,CSN)
  RF24Network network(radio);      // Include the radio in the network
  const uint16_t this_node = 04;   // Address of this node in Octal format ( 04,031, etc)
  const uint16_t other_node = 00;      // Address of the other node in Octal format



// data structure for radio communication
// from first slave
struct payload_t {
  int Pressure;
   byte OutsideTemperature;
   byte OutsideHumidity;
   byte FrontRain;
   byte rVoltage;
   
   byte FrontSoil;
   byte fVoltage;
   
   byte BackSoil;
   byte bVoltage;

   byte SideMoisture;
   byte sVoltage;

};

//variables to send: with radio
byte temp;
byte humidity;
byte rain;
int Pressure;

  


//cycles
  int cycle = 2;
  
//voltage detector
  const long InternalReferenceVoltage = 1081;
  int VoltageTime = 17;  //when voltage should be scanned.
  int Voltage = 330;  //stores the voltage
  byte Volts;  //stores mapped voltage






// ---------------------------------
//void setup

// ---------------------------------


void setup() {
   Serial.begin(9600); //serial monitor

//radio network
  SPI.begin();
  radio.begin();
  network.begin(/*channel*/ 108, /*node address*/ this_node);

  
  radio.stopListening();  //sets in tx mode only

    
    // bme setting
    bme.begin(0x76);  




//rain sensor
  pinMode(rainPower, OUTPUT);
  digitalWrite(rainPower, LOW);
  

}



// -------------------------------------------------------------
// LOOP

// --------------------------------------------------------------



void loop() {
  // put your main code here, to run repeatedly:



//calls on the rain function below for the amount of rain. 
  rain = (((readRain()-970)*(-1))/9.5);

  if (rain < 30 || rain > 110) {
    rain = 0;
  }
  Serial.print("Rain sensor value: ");
  Serial.println(rain);
  


//voltage
VoltageTime++;
  if (VoltageTime > 18) //once a day, check voltage
  {
    Voltage = getBandgap(); //runs function below to get voltage. 
    Volts = map(Voltage, 0, 330, 0 , 100); //map, needs to be calibrated.
    VoltageTime = 0; //reset the clock.
  if (Volts < 1) {
      Volts = 1;
    }
  }

  Serial.print("Internal voltage: ");
  Serial.println(Volts);



//bme280 stuff
   
  Pressure = (bme.readPressure() / 100.0F);
  Serial.print("Pressure = ");
  Serial.println(Pressure);

  humidity = bme.readHumidity();
  Serial.print("Humidity = ");
  Serial.println(humidity);

  temp = bme.readTemperature();
  Serial.print("Temperature = ");
  Serial.println(temp);

  

// this function sends the data to the main radio
 Serial.print("sending");
 //updates the network
   network.update();
    payload_t payload = {Pressure, temp, humidity, rain, Volts, 0, 0, 0, 0, 0, 0};
     RF24NetworkHeader header(other_node);
    bool ok = network.write(header,&payload,sizeof(payload));

    //used for debug purposes: (is data received or not)
    if (ok)
      Serial.println("success");
    else
      Serial.println("fail");



  //brains off. 
  radio.powerDown();
  for(int q = 0; q < cycle; q++)
  {
    LowPower.powerDown(SLEEP_2S, ADC_OFF, BOD_OFF);
  }
  radio.powerUp();
}




//------------------------------------
//
// Global functions:
//
//------------------------------------



//reads the data from the rain sensor
int readRain()
{
  //briefly sends a current through rain sensor
    digitalWrite(rainPower, HIGH);
    delay(5);
    int val = analogRead(rainPin);//Reads the current from the sensor
    digitalWrite(rainPower, LOW);
    return val; //send current moisture value
}

int getBandgap() 
  {
  // REFS0 : Selects AVcc external reference
  // MUX3 MUX2 MUX1 : Selects 1.1V (VBG)  
   ADMUX = bit (REFS0) | bit (MUX3) | bit (MUX2) | bit (MUX1);
   ADCSRA |= bit( ADSC );  // start conversion
   while (ADCSRA & bit (ADSC))
     { }  // wait for conversion to complete
   int results = (((InternalReferenceVoltage * 1024) / ADC) + 5) / 10; 
   return results;
  }
  
