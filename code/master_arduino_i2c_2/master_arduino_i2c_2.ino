
//12/21/2020: i think this is the main master arduino file.  

// CODE FOR THE MASTER I2C and Radio Arduino
// This arduino collects the data from the radio communication and sends it
//  to the Slave i2c Arduino where it can be stored and interpreted. 
// 

// --------------------------------------------------------------------
// basic startup code:
//
// --------------------------------------------------------------------


//temperature sensor setup
#include <dht.h>   // this is the DHT sensor library
  #define dhtTemp A0 // Analog Pin sensor is connected to
  dht DHT;    // "creates a DHT object"

  
// i2c communication set up
  #include <Wire.h>
  #include <EasyTransferI2C.h>
    //create object
    EasyTransferI2C ET; 

  
//wireless network stuff
  #include <RF24Network.h>
  #include <RF24.h>
  #include <SPI.h>


  RF24 radio(9, 10);               // nRF24L01 (CE,CSN)
  RF24Network network(radio);      // Include the radio in the network

  //every node in the network must be listed here. 
  const uint16_t this_node = 00;   // Address of this node in Octal format ( 04,031, etc)
  const uint16_t other_node = 01;      // Address of the other node in Octal format
  const uint16_t node02 = 02;
  const uint16_t node03 = 03;
  const uint16_t node04 = 04;


// This is for the D3231 clock
  #define DS3231_I2C_ADDRESS 0x68 //necessary for obtaining values from clock
  byte decToBcd(byte val) {   //converts decimals to binary coded decimal
    return( (val/10*16) + (val%10) );
  }
  byte bcdToDec(byte val){    // Convert binary coded decimal to normal decimal numbers
  return( (val/16*10) + (val%16) );
}




// data structure for i2c communication
struct SEND_DATA_STRUCTURE{
  //put your variable definitions here for the data you want to send
  //THIS MUST BE EXACTLY THE SAME ON THE OTHER ARDUINO
  byte IndoorTemperature;
  byte IndoorHumidity;
  byte Hour;
  byte Minute;
  byte DayOfWeek;
  byte Day;
  byte Month;
  int Year;

  int16_t Pressure;
  byte OutsideTemperature;
  byte OutsideHumidity;
  byte FrontRain;

  byte FrontSoil;
  byte fVoltage;

  byte BackSoil;
  byte bVoltage;

  byte SideSoil;
  byte sVoltage;
};

//var for above struct
SEND_DATA_STRUCTURE mydata;
//define slave i2c address
#define I2C_SLAVE_ADDRESS 9




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

payload_t payload;






// the timer to send data to the master i2c Arduino. 
const unsigned long interval = 6000; //ms  
unsigned long last_sent;             // When did we last send?

  



// ---------------------------------
//void setup

// ---------------------------------


void setup(){

//initializes Serial communication
  Serial.begin(9600);
 //starts the wire library
  Wire.begin();
  //start the library for i2c communication
  ET.begin(details(mydata), &Wire);


// starts the radio communication libraries. 
  SPI.begin();
  radio.begin();
  network.begin(108, this_node);

  


//this is a function that the consumer can use to set the time
// of the DS3231 real time clock. 
    // DS3231 seconds, minutes, hours, day, date, month, year
  //setDS3231time(30,37,19,1,10,3,19);
  //setDS3231time(30,57,16,2,21,1,20);

//stores a few base values into the i2c communication so nothing breaks.
  resetData();
}



// -------------------------------------------------------------
// LOOP

// --------------------------------------------------------------


void loop(void){

//checks the radio for any incoming data
  network.update();
  //===== Receiving =====//
  while ( network.available() ) {     // Is there any incoming data?
    RF24NetworkHeader header;
    payload_t payload;
    Serial.println("receiving...");
   network.read(header,&payload,sizeof(payload));


    Serial.print("pressure = ");
  Serial.println(payload.Pressure);
  

  Serial.print("temp = ");
  Serial.print(payload.OutsideTemperature);
  Serial.print(" , humidity = ");
  Serial.println(payload.OutsideHumidity);



  Serial.print("front soil: ");
  Serial.print(payload.FrontSoil);
  Serial.print(", ");
  Serial.println(payload.FrontRain);

  Serial.print("backyard arduino data: ");
  Serial.print("soil = ");
  Serial.println(payload.BackSoil);

  Serial.print("Side yard data: ");
  Serial.println(payload.SideMoisture);


// If there isn't a zero value for pressure, that means
// that the data is coming from the Front yard sensor system.
  if (payload.Pressure > 0) { 
    mydata.Pressure = payload.Pressure;
    int ofarenheit = (((payload.OutsideTemperature)*1.8)+32);
    mydata.OutsideTemperature = ofarenheit;
    mydata.OutsideHumidity = payload.OutsideHumidity;
    mydata.FrontRain = payload.FrontRain;
  }

  else if (payload.fVoltage > 0 || payload.FrontSoil > 0) {
    mydata.FrontSoil = payload.FrontSoil;
    mydata.fVoltage = payload.fVoltage;
  }
// If the values for the front and side sensors read as zero,
// that means the data is coming from the back yard sensor. 
  else if (payload.bVoltage > 0 || payload.BackSoil > 0) {
    mydata.BackSoil = payload.BackSoil;
    mydata.bVoltage = payload.bVoltage;
  }

// If the values for the front and back sensors read zero,
// that means the data is coming from the side yard sensor.
  else if (payload.sVoltage > 0 || payload.SideMoisture > 0) {
    mydata.SideSoil = payload.SideMoisture;
    mydata.sVoltage = payload.sVoltage;
  }
  else {}
  
 }



  
// If it's time to send a message, send it!

unsigned long now = millis();              
  if ( now - last_sent >= interval  )
  {
    last_sent = now;
  //this is how you access the variables. [name of the group].[variable name]
  DHT.read11(dhtTemp); 


  int ifarenheit = (((DHT.temperature)*1.8)+32);
  mydata.IndoorTemperature = ifarenheit;
  
  mydata.IndoorHumidity = DHT.humidity;
  mydata.Hour = Hour();
  mydata.Minute = Minute();
  mydata.DayOfWeek = DayOfWeek();
  mydata.Day = Day();
  mydata.Month = Month();
  mydata.Year = Year();

  
  //send the data (the other 8 variables should be defined above)
  ET.sendData(I2C_SLAVE_ADDRESS);

  
  }

  
}

//------------------------------------
//
// Global functions:
//
//------------------------------------


//this is a function that will communicate with i2c to the arduino.
// The function is called only once during setup. 
void setDS3231time(byte second, byte minute, byte hour, byte dayOfWeek, byte
dayOfMonth, byte month, byte year){
  // sets time and date data to DS3231
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set next input to start at the seconds register
  Wire.write(decToBcd(second)); // set seconds
  Wire.write(decToBcd(minute)); // set minutes
  Wire.write(decToBcd(hour)); // set hours
  Wire.write(decToBcd(dayOfWeek)); // set day of week (1=Sunday, 7=Saturday)
  Wire.write(decToBcd(dayOfMonth)); // set date (1 to 31)
  Wire.write(decToBcd(month)); // set month
  Wire.write(decToBcd(year)); // set year (0 to 99)
  Wire.endTransmission();
}


// calling on this function allows the DS3231 to
//  send data back to the Arduino.
void readDS3231time(byte *second,
byte *minute,
byte *hour,
byte *dayOfWeek,
byte *dayOfMonth,
byte *month,
byte *year){
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
  // request seven bytes of data from DS3231 starting from register 00h
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
  *dayOfWeek = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *year = bcdToDec(Wire.read());
}

//this function collects the hour from the ds3231
byte Hour() 
{
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
  int Hours = (hour);
  return Hours;
}

// this function collects the day of the week from the ds3231
byte DayOfWeek() 
{
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
  int DayOfWeeks = (dayOfWeek);
  return DayOfWeeks;
}

// this function collects the minute from the ds3231
byte Minute()
{
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
  int Minutes = (minute);
  return Minutes;
}


byte Month() {
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
  int Months = (month);
  return Months;
}

byte Day() {
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
  int Days = (dayOfMonth);
  return Days;
  
}

int Year() {
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
  int Years = (year);
  return Years;
}

//this function resets the data that is in the Arduino
// useful for initial setup. 
void resetData()
{
  mydata.Pressure = 840;
  
  mydata.OutsideTemperature = 0;
  mydata.OutsideHumidity = 0;
  mydata.FrontSoil = 0;
  mydata.FrontRain = 0;
  
  mydata.BackSoil = 0;
  
  mydata.SideSoil = 0;
  mydata.fVoltage = 100;
  mydata.bVoltage = 100;
  mydata.sVoltage = 100;
}
