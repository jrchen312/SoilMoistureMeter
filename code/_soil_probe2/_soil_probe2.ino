/*
 * Soil Moisture Probe 2 (battery powered)
 * 
 * This section codes for one of the soil moisture probes of
 * the network system. 
 * 
 * It attempts to save as much power as possible while collecting
 * data regularly.
 * 
 */


//______________________________________________________________
//
// Global variables and libraries:
//______________________________________________________________
//


//HL-69 soil moisture sensor
  int soilVal = 0;  //soil moisture analog input
  const int soilPin = A0;  //analog pin 
  const int soilPower = 4;  //VCC digital pin


//low power library
  #include <LowPower.h>


//nrf24l01 radio libraries
  #include <RF24Network.h>
  #include <RF24.h>
  #include <SPI.h>



//nrf24l01 radio
  RF24 radio(9, 10);               // nRF24L01 (CE,CSN)
  RF24Network network(radio);      // Include the radio in the network
  const uint16_t this_node = 02;   // Address of this node in Octal format ( 04,031, etc)
  const uint16_t other_node = 00;      // Address of the other node in Octal format


//structure for data tx
  struct payload_t 
  {
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
  int soil;

//cycles
  int cycle = 15;

  
//voltage detector
  const long InternalReferenceVoltage = 1081;
  int VoltageTime = 17;  //when voltage should be scanned.
  int Voltage = 330;  //stores the voltage
  byte Volts;  //stores mapped voltage

/*
// Find internal 1.1 reference voltage on AREF pin
void setup ()
{
  analogReference (INTERNAL);
  analogRead (A0);  // force voltage reference to be turned on
}
void loop () { }
 */




//______________________________________________________________
//
// Set up:
//______________________________________________________________
//


  
  
void setup() {
//radio network
  SPI.begin();
  radio.begin();
  network.begin(/*channel*/ 108, /*node address*/ this_node);

  radio.setPALevel(RF24_PA_LOW); //might want to decrease power requirements
  //radio.setDataRate(RF24_250KBPS); //this COULD break something. refer to https://github.com/nRF24/RF24/issues/137 if something does. 
  radio.stopListening();  //sets in tx mode only


//Soil sensor pins
  pinMode(soilPower, OUTPUT);
  digitalWrite(soilPower, LOW);
  


} //end of setup





//______________________________________________________________
//
// loop:
//______________________________________________________________
//

void loop() {

//pump the network
  network.update();


// soil sensor data
  soil = (((readSoil()-970)*(-1))/9.5);

  if ((soil < 1) || (soil > 110))
  {
    soil = 0;
  }



  

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




//sending data
 

  payload_t payload = {0, 0, 0, 0, 0, 0, 0, soil, Volts, 0, 0};
     RF24NetworkHeader header(other_node);
    bool ok = network.write(header,&payload,sizeof(payload));
    if (ok)
    {
      Serial.println("success");
    }
    else
    {
      Serial.println("fail");
      //machine learning is ok here. 
      /*cycle = cycle -1;
      if (cycle < 65) 
      {
        cycle = cycle + 6;
      }
      */
    }



//brains off. 
  radio.powerDown();
  for(int q = 0; q < cycle; q++)
  {
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  }
  radio.powerUp();
  

}    // end of loop




//______________________________________________________________
//
// functions:
//______________________________________________________________
//



int readSoil()
{
    digitalWrite(soilPower, HIGH);
    delay(5);
    int val = analogRead(soilPin);//Reads the current from the sensor
    digitalWrite(soilPower, LOW);
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
