/*
 * Arduino code for slave arduino i2c :p
 * 
 * 
 * 
 * 
 */


// --------------------------------------------------------------------
// basic startup code:
//
// --------------------------------------------------------------------


// 1.8 inch tft
#include <SPI.h>
#include "Ucglib.h"  // Include Ucglib library to drive the display

    
// Create display and define the pins:

Ucglib_ST7735_18x128x160_SWSPI ucg(/*sclk=*/ 13, /*data=*/ 11, /*cd=*/ 9, /*cs=*/ 10, /*reset=*/ 8);
//Ucglib_ST7735_18x128x160_SWSPI ucg(/*sclk=*/ 4, /*data=*/ 5, /*cd=*/ 7 , /*cs=*/ 6, /*reset=*/ 8);
// The rest of the pins are pre-selected as the default hardware SPI for Arduino Uno (SCK=13 and SDA=11)




//i2c communication set up:
  #include <Wire.h>
  #include <EasyTransferI2C.h>
  //create object
  EasyTransferI2C ET; 



  

//variables:
int shift = 0;  // which screen are we displaying
int currentScreen = 0;
int screen = 0;
int counter = 0;
byte zone = 0;

int currentMinute = 0;

byte ProbeScreenSwitch =0;

byte Mois;


//buttons/buzzers
const int buttonF = 2; //switches forward
int buttonFVal = 0;
const int buttonB = 3; //goes back
int buttonBVal = 0;

const int buzzer = 5;
bool buzzOn = false;



// data structure for the I2C communication
struct RECEIVE_DATA_STRUCTURE{
  //put your variable definitions here for the data you want to receive
  //THIS MUST BE EXACTLY THE SAME ON THE OTHER ARDUINO

  //these are all bytes to conserve on space and to add for more expansion. 
  // max size of file transfer is 26 bytes

  
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

//give a name to the group of data
  RECEIVE_DATA_STRUCTURE mydata;
//define slave i2c address
  #define I2C_SLAVE_ADDRESS 9



  
/*
//theoretical data from master :p
  byte IndoorTemperature = 68;
  byte IndoorHumidity = 45;
  byte Hour = 18;
  byte Minute = 38;
  byte DayOfWeek = 7;
  byte Day = 28;
  byte Month = 10;
  int Year = 2019;

  int16_t Pressure;
  byte OutsideTemperature = 38;
  byte OutsideHumidity = 43;
  byte FrontRain;

  byte FrontSoil = 56;
  byte fVoltage;

  byte BackSoil = 95;
  byte bVoltage;

  byte SideSoil = 76;
  byte sVoltage;*/
  

// bluetooth
  // variable for bluetooth connection
   int state; 




/*
// SD card
  #include <SPI.h> //SD card library
  
  #define SD_CS_PIN 4 //sd card pin
  #include "SdFat.h" //pwr saving library for SD
  SdFat SD;  //sd object
  File myFile; //myFile name

 // txt file line break
 // this code helps read the data from the SD card files:
      char inputString [10];
      char inputChar;
      int stringIndex = 0; //String stringIndexing int;

*/

      

  

void setup() {


//begins arduino on serial speed of 115200 to allow for efficient
  // communication over bluetooth
  Serial.begin(115200);

  //begin the wire i2c communication
  Wire.begin(I2C_SLAVE_ADDRESS);
  
  //start the library
  ET.begin(details(mydata), &Wire);
  
  //define handler function on receiving data
  Wire.onReceive(receive);
  

/*
// sd card initialization
  if (!SD.begin(SD_CS_PIN)) {  //SD card communication
    Serial.println("failed");
    return;
  }
*/



  
   ucg.begin(UCG_FONT_MODE_SOLID);

   ucg.clearScreen();  // Clear the screen
   //ucg.setRotate180();
   //ucg.setPrintDir(0);



pinMode(buttonF, INPUT);
  pinMode(buttonB, INPUT);
  pinMode(buzzer, OUTPUT);
  
 screen = 0;
  shift = 1;


  //defines a random seed for the random generator
  randomSeed(analogRead(0));

}




void loop() {
  // put your main code here, to run repeatedly:


if(ET.receiveData()){

  if (mydata.FrontSoil >100) {
    mydata.FrontSoil == 0;
  }


  if (mydata.BackSoil > 100) {
    mydata.BackSoil == 0;
  }
  
  }







if (shift != 0) {
  screen = currentScreen + shift;

    //if shift is -1, go to the home page. 
  if (shift == -1) {
    screen = 1;
  }

  
  shift = 0; //stop shifting. 


  // 
  if (screen == 1 || screen == 2) {
    ucg.clearScreen();
  }

    //displays the information for the home screen
  if (screen == 1) {
    currentScreen = 1;
    screen =1;
    DispBox();
    DispHome();
    currentMinute = mydata.Minute;
    DispHomeInfo();

  // displays information for first soil moisture probe zone
  } else if (screen == 2) {
    currentScreen = 2;

    zone = 1;
    DispSmallBox();
    DispSoil(soilCond(mydata.FrontSoil), mydata.FrontSoil);
    
  } else if (screen == 3) {
    currentScreen = 3;
    zone =2;
    dispsoil(soilCond(mydata.BackSoil), mydata.BackSoil);

  } else if (screen == 4) {
    currentScreen = 0;
    screen = 0;
    zone = 3;
    dispsoil(soilCond(mydata.SideSoil), mydata.SideSoil);  
    ProbeScreenSwitch = 0;

/*
    //remove later
    mydata.SideSoil = random (10,99);
    mydata.BackSoil = random (10, 99);
    mydata.FrontSoil = random(10,99); */
  } 
} 
  



  
buttonFVal = digitalRead(buttonF);
  if (buttonFVal ==HIGH) {
    shift = 1;
    analogWrite(buzzer, 10);
    buzzOn = true;
  }

buttonBVal = digitalRead(buttonB);
  if (buttonBVal == HIGH) {
    shift = -1;
    analogWrite(buzzer, 10);
    buzzOn = true;
  }

delay(100);

if (buzzOn == true) {
  analogWrite(buzzer, 0);
  buzzOn = false;
}


/*
 * This code below is inteded to update the home screen every minute
 */
 
if ((mydata.Minute != currentMinute) && (screen ==1) && (shift != -1))
{
  DispHomeInfo();
  currentMinute = mydata.Minute;
}


/*
counter++;
if (counter > 50) {
  Minute ++;
  counter =0;

  IndoorHumidity = random(10,99);
  IndoorTemperature = random(10,99);
  OutsideTemperature = random(10, 99);
  OutsideHumidity = random(10,99);

} */

  
// checks if bluetooth is connected
  if (Serial.available()) {    
    //reads any value from BT
    state = (Serial.read());   

  //if the ascii value sent from bluetooth is equivalent to "2", then:
    if (state == 50) { 

    //begins sending values to the bluetooth, starting with the 
    //indoor information (temperature and humidity)

      // the divider helps the app seperate the different data values
      Serial.print(mydata.IndoorTemperature); 
      Serial.print("|");
      
      Serial.print(mydata.IndoorHumidity);
      Serial.print("|");

    //outdoor stuff
      Serial.print(mydata.OutsideTemperature); 
      Serial.print("|");

      Serial.print(mydata.OutsideHumidity);
      Serial.print("|");

    //this is a moisture percentage (0-100)
      Serial.print(mydata.FrontSoil);  
      Serial.print("|");

      Serial.print (mydata.BackSoil);
      Serial.print("|");

      Serial.print(mydata.SideSoil);
      Serial.print("|");


// sends "too wet", "too dry", and "perfect" on the app
// '1' is too wet, '2' is perfect, '3' is dry, '4' displays a dash (soil meter probe not in soil).
      Serial.print(soilCond(mydata.FrontSoil));
      Serial.print("|");

      Serial.print(soilCond(mydata.BackSoil));
      Serial.print("|");

      Serial.print(soilCond(mydata.SideSoil));
      Serial.print("|");


      Serial.print(waterLawn());  //How frequently water lawn? 1-14 days.
      Serial.print("|");
      Serial.print(waterLawn());  //How frequently water lawn? 1-14 days.
      Serial.print("|");
      Serial.print(waterLawn());  //How frequently water lawn? 1-14 days.
      Serial.print("|");


      Serial.print("76|75|74|"); //sunday
      Serial.print("66|67|68|"); //monday
      Serial.print("83|84|85|"); //tuesday
      Serial.print("80|81|82|"); //wednesday
      Serial.print("72|73|74|"); //thurs
      Serial.print("86|87|88|"); //frid
      Serial.print("92|93|91|"); //sat
      Serial.print((mydata.FrontRain)/10);

      Serial.print("|");

      Serial.println(soilCond(80));
      
      state = 0;
      
      
  /*
    int wktotalavg;  // This is an average of soil moisture for all stations over the past week.
      int wktotal = 0;  // This is a total of soil moisture for all stations over the past week.


    //This function will access the data that is stored in
    // the SD card of the prototype and then print it out 
    // for the Bluetooth HC-06 module. 
       myFile = SD.open("Sun.txt");
         if (myFile) {
           while (myFile.available()) {  // read from the file
              inputChar = myFile.read();

              if (inputChar != '\n')
                {
                  inputString[stringIndex] = inputChar;

                  stringIndex++;
                }
               else
               {
                Serial.print(inputString);
                Serial.print("|");
                wktotal = wktotal + inputString;
                stringIndex = 0;
               }
           
             }
         myFile.close();    //close the file
         }

      myFile = SD.open("Mon.txt");
         if (myFile) {
           while (myFile.available()) {  // read from the file
              inputChar = myFile.read();

              if (inputChar != '\n')
                {
                  inputString[stringIndex] = inputChar;

                  stringIndex++;
                }
               else
               {
                Serial.print(inputString);
                Serial.print("|");
                wktotal = wktotal + inputString;
                stringIndex = 0;
               }
           
             }
         myFile.close();    //close the file
         }
      myFile = SD.open("Tues.txt");
         if (myFile) {
           while (myFile.available()) {  // read from the file
              inputChar = myFile.read();

              if (inputChar != '\n')
                {
                  inputString[stringIndex] = inputChar;

                  stringIndex++;
                }
               else
               {
                Serial.print(inputString);
                Serial.print("|");
                wktotal = wktotal + inputString;
                stringIndex = 0;
               }
           
             }
         myFile.close();    //close the file
         }
         
      myFile = SD.open("Wed.txt");
         if (myFile) {
           while (myFile.available()) {  // read from the file
              inputChar = myFile.read();

              if (inputChar != '\n')
                {
                  inputString[stringIndex] = inputChar;

                  stringIndex++;
                }
               else
               {
                Serial.print(inputString);
                Serial.print("|");
                wktotal = wktotal + inputString;
                stringIndex = 0;
               }
           
             }
         myFile.close();    //close the file
         }
      myFile = SD.open("Thurs.txt");
         if (myFile) {
           while (myFile.available()) {  // read from the file
              inputChar = myFile.read();

              if (inputChar != '\n')
                {
                  inputString[stringIndex] = inputChar;

                  stringIndex++;
                }
               else
               {
                Serial.print(inputString);
                Serial.print("|");
                wktotal = wktotal + inputString;
                stringIndex = 0;
               }
           
             }
         myFile.close();    //close the file
         }
      myFile = SD.open("Fri.txt");
         if (myFile) {
           while (myFile.available()) {  // read from the file
              inputChar = myFile.read();

              if (inputChar != '\n')
                {
                  inputString[stringIndex] = inputChar;

                  stringIndex++;
                }
               else
               {
                Serial.print(inputString);
                Serial.print("|");
                wktotal = wktotal + inputString;
                stringIndex = 0;
               }
           
             }
         myFile.close();    //close the file
         }
         
         
      myFile = SD.open("Sat.txt");
         if (myFile) {
           while (myFile.available()) {  // read from the file
              inputChar = myFile.read();

              if (inputChar != '\n')
                {
                  inputString[stringIndex] = inputChar;

                  stringIndex++;
                }
               else
               {
                Serial.print(inputString);
                Serial.print("|");
                wktotal = wktotal + inputString;
                stringIndex = 0;
               }
           
             }
         myFile.close();    //close the file
         }

      wktotalavg = wktotal / 21;
        
      Serial.print((mydata.FrontRain)/10);

      Serial.print("|");

      Serial.println(soilCond(wktotalavg));
      
      state = 0;

      */

      
    }
  }
}  //END OF LOOP

/* fonts used:
 *  ucg.setFont(ucg_font_7x14B_mr);        bold header
 *  ucg.setFont(ucg_font_inr24_mr);        very thick letters
 *  ucg.setFont(ucg_font_6x13_mr);         for any small fonts
 */




// displays the basic information for the home screen
 void DispHome()
 {
  // display the text
  ucg.setFont(ucg_font_6x13_mr);
  ucg.setColor(0, 255, 255, 255);  // Set color (0,R,G,B)
  ucg.setColor(1, 0, 0, 0);  // Set color of text background (1,R,G,B)
  ucg.setPrintPos(9, 73);
  ucg.print(F("Indoor"));
  ucg.setPrintPos(9, 123);
  ucg.print(F("Outdoor"));

//humidity symbol LOL
  ucg.setColor(0, 102, 178, 255);
  ucg.drawDisc(78, 95, 4, UCG_DRAW_LOWER_RIGHT); //
  ucg.drawDisc(78, 95, 4, UCG_DRAW_LOWER_LEFT); 
  ucg.drawTriangle(74,95, 82,95, 78,86);
  
  ucg.drawDisc(78, 145, 4, UCG_DRAW_LOWER_RIGHT); //
  ucg.drawDisc(78, 145, 4, UCG_DRAW_LOWER_LEFT); 
  ucg.drawTriangle(74,145, 82,145, 78,136);

//thermometer symbol

  
  ucg.setColor(0, 255, 255, 255); //white
  ucg.drawCircle(78, 75, 4, UCG_DRAW_LOWER_RIGHT); //bottom circle
  ucg.drawCircle(78, 75, 4, UCG_DRAW_LOWER_LEFT);
  ucg.drawCircle(76, 75, 3, UCG_DRAW_UPPER_LEFT); //top of bottom circle
  ucg.drawCircle(80, 75, 3, UCG_DRAW_UPPER_RIGHT);
  ucg.drawLine(76,72, 76,67); //stems of themometer
  ucg.drawLine(80,72, 80,67);
  ucg.drawCircle(78, 67, 2, UCG_DRAW_UPPER_LEFT); //top of thermometer
  ucg.drawCircle(78, 67, 2, UCG_DRAW_UPPER_RIGHT);

  ucg.setColor(255, 0 , 0);
  ucg.drawDisc(78, 75, 2, UCG_DRAW_ALL);
  ucg.drawBox(77, 69, 3, 4);
  

  ucg.setColor(0, 255, 255, 255); //white
  ucg.drawCircle(78, 125, 4, UCG_DRAW_LOWER_RIGHT); //bottom circle
  ucg.drawCircle(78, 125, 4, UCG_DRAW_LOWER_LEFT);
  ucg.drawCircle(76, 125, 3, UCG_DRAW_UPPER_LEFT); //top of bottom circle
  ucg.drawCircle(80, 125, 3, UCG_DRAW_UPPER_RIGHT);
  ucg.drawLine(76,122, 76,117); //stems of themometer
  ucg.drawLine(80,122, 80,117);
  ucg.drawCircle(78, 117, 2, UCG_DRAW_UPPER_LEFT); //top of thermometer
  ucg.drawCircle(78, 117, 2, UCG_DRAW_UPPER_RIGHT);

  ucg.setColor(255, 0 , 0);
  ucg.drawDisc(78, 125, 2, UCG_DRAW_ALL);
  ucg.drawBox(77, 119, 3, 4);
 }


//displays the essential information of home screen
void DispHomeInfo() {
  //let's display the time :D
  ucg.setFont(ucg_font_inr24_mr);
  ucg.setColor(0, 255, 255, 255);
  ucg.setPrintPos(13,30);
  ucg.print(F("      "));
  if (mydata.Hour > 9) 
  {
   ucg.setPrintPos(13, 30);
  } else {
    ucg.setPrintPos(24, 30);
  }
  ucg.print(mydata.Hour);
  ucg.print(F(":"));
  if (mydata.Minute < 10) 
  {
    ucg.print(F("0"));
  }
  ucg.print(mydata.Minute);


  //let's display the date!
  ucg.setFont(ucg_font_7x14B_mr);
  ucg.setColor(0, 255, 204, 204);
  ucg.setPrintPos(36, 53);
  ucg.print(F("      ")); //this might need some more later. 
  ucg.setPrintPos(36, 53);
  if (mydata.Month < 10) 
  {
    ucg.print(F("0"));
  }
  ucg.print(mydata.Month);
  ucg.print(F("/"));
  if (mydata.Day < 10) 
  {
    ucg.print(F("0"));
  }
  ucg.print(mydata.Day);
  ucg.print(F("/"));
  ucg.print(mydata.Year);


int randNum = random(2);
if (mydata.OutsideTemperature > 0)
{
  randNum = randNum + mydata.OutsideTemperature;
} else if (mydata.OutsideTemperature == 0) 
{
  randNum = mydata.IndoorTemperature - 3;
}


  //display indoor data
  ucg.setColor(0, 255, 255, 255);
  ucg.setPrintPos( 92, 78);
  ucg.print(randNum);  //used to be mydata.IndoorTemperature;
  ucg.print(F(" F"));
  ucg.setPrintPos(92, 98);
  ucg.print(mydata.IndoorHumidity);
  ucg.print(F(" %"));

  //display outdoor data
  ucg.setColor(0, 255, 255, 255);
  ucg.setPrintPos( 92, 128);
  ucg.print(mydata.OutsideTemperature);
  ucg.print(F(" F"));
  ucg.setPrintPos(92, 148);
  ucg.print(mydata.OutsideHumidity);
  ucg.print(F(" %"));
}

  


// displays basic information for the soil moisture probe
void DispSoil(int Condition, byte Percent) 
{
  // display the text
  ucg.setFont(ucg_font_7x14B_mr);
  ucg.setColor(0, 0, 255, 255);  // Set color (0,R,G,B)
  ucg.setColor(1, 0, 0, 0);  // Set color of text background (1,R,G,B)
  ucg.setPrintPos(42, 11);
  ucg.print(F("Zone: "));
  ucg.setPrintPos(80, 11);
  ucg.print(zone); 

  ucg.setFont(ucg_font_inr24_mr);
  ucg.setColor(0, 255, 255, 255);
  ucg.setPrintPos(26,62);

  if (Percent == 0) {
    ucg.setPrintPos(34,62);
    ucg.print("--");
  } else {
  ucg.print(Percent);
  ucg.print(F("%"));
  ucg.setPrintPos(11, 102);
    ucg.setFont(ucg_font_6x13_mr);
  ucg.print(F("Water this zone"));
  }
  
  ucg.setFont(ucg_font_6x13_mr);
  ucg.setColor(0, 255, 255, 255);  // Set color (0,R,G,B)
  ucg.setPrintPos(11, 29);
  ucg.print(F("Soil Moisture:"));
  

  
  ucg.setFont(ucg_font_6x13_mr);
  if (Condition == 1) {
    ucg.setPrintPos(11, 81);
    ucg.print(F("Quality: "));
    ucg.setColor(0, 255, 0, 0);
    ucg.print(F("TOO WET"));
    
    ucg.setPrintPos(11, 116);
    ucg.setColor(0, 204, 229, 255);
    ucg.print(F("2 minute(s) "));
    ucg.setColor(0, 255, 30, 30);
    ucg.setPrintPos(82, 116);
    ucg.print(F("less"));

    ProbeScreenSwitch = 0;
    
  } else if (Condition == 0) {
    ucg.setPrintPos(11, 81);
    ucg.print(F("Quality: "));
    ucg.setColor(0, 255, 255, 255);
    ucg.print(F("--     "));

    ProbeScreenSwitch = 1;

    ucg.setPrintPos(11, 102);
    ucg.print(F("Insert the probe"));
    ucg.setPrintPos(11, 116);
    ucg.print(F("into your lawn."));
    
  } else if (Condition == 2) {
    ucg.setPrintPos(11, 81);
    ucg.print(F("Quality: "));
    ucg.setColor(0, 0, 255, 0);
    ucg.print(F("PERFECT"));
    
    
    ucg.setPrintPos(11, 116);
    ucg.setColor(0, 204, 229, 255);
    ucg.print(F("the same!"));

    ProbeScreenSwitch = 0;
    
  } else if (Condition == 3) {
    ucg.setPrintPos(11, 81);
    ucg.print(F("Quality: "));
    ucg.setColor(0, 255, 0, 0);
    ucg.print(F("TOO DRY"));
    
    ucg.setPrintPos(11, 116);
    ucg.setColor(0, 204, 229, 255);
    ucg.print(F("2 minute(s) "));
    ucg.setColor(0, 255, 30, 30);
    ucg.setPrintPos(82, 116);
    ucg.print(F("more"));

    ProbeScreenSwitch = 0;
    
  }

  ucg.setFont(ucg_font_6x13_mr);
  ucg.setColor(0, 255, 255, 255);  // Set color (0,R,G,B)
  ucg.setPrintPos(11, 140);
  ucg.print(F("Water your lawn"));
  ucg.setPrintPos(11, 154);
  ucg.setColor(0, 255, 30, 30);
  ucg.print("3");
  ucg.setColor(0, 255, 255, 255);
  ucg.print(F(" times a week"));
     
  //battery   32,62
  ucg.setColor(0, 255, 255, 255); //white
  ucg.drawLine(100,60, 110,60); //base
  ucg.drawLine(100,60, 100,43);  //stem
  ucg.drawLine(110,60, 110,43);  //stem
  
  ucg.drawLine(100,43, 102,43);
  ucg.drawLine(110,43, 108,43);

  ucg.drawLine(102,43, 102,41);
  ucg.drawLine(108,43, 108,41);
  ucg.drawLine(108,41, 102,41);
  
  ucg.setColor(0, 0, 255, 0); //green
  ucg.drawBox(102, 45, 7, 4);
  ucg.drawBox(102, 50, 7, 4);
  ucg.drawBox(102, 55, 7, 4);
}


void dispsoil(int Condition, byte Percent) {

  ucg.setFont(ucg_font_7x14B_mr);
  ucg.setPrintPos(80, 11);
  ucg.print(F("  "));

  ucg.setFont(ucg_font_inr24_mr);
  ucg.setPrintPos(26,62);
  ucg.print(F("   "));


  
  ucg.setFont(ucg_font_6x13_mr);

  if ((ProbeScreenSwitch = 1) && (Percent != 0)) {   //used to be  //if (ProbeScreenSwitch = 1) {
    ucg.setPrintPos(11, 104);
     ucg.print(F("                 "));

     ucg.setPrintPos(11, 102);
  ucg.print(F("Water this zone"));
  }
  ucg.setPrintPos(11, 116);
  ucg.print(F("                 "));


  
  
  ucg.setFont(ucg_font_7x14B_mr);
  ucg.setColor(0, 0, 255, 255);  // Set color (0,R,G,B)
  ucg.setColor(1, 0, 0, 0);  // Set color of text background (1,R,G,B)
  ucg.setPrintPos(80, 11);
  ucg.print(zone);
  
  ucg.setFont(ucg_font_inr24_mr);
  ucg.setColor(0, 255, 255, 255);
  ucg.setPrintPos(26,62);
  
  if (Percent == 0) {
    ucg.setPrintPos(34,62);
    ucg.print("--");
  } else {
    ucg.print(Percent);
     ucg.print(F("%"));
  }




  ucg.setFont(ucg_font_6x13_mr);
  if (Condition == 1) {
    ucg.setPrintPos(11, 81);
    ucg.print(F("Quality: "));
    ucg.setColor(0, 255, 0, 0);
    ucg.print(F("TOO WET"));
    
    ucg.setPrintPos(11, 116);
    ucg.setColor(0, 204, 229, 255);
    ucg.print(F("2 minute(s) "));
    ucg.setColor(0, 255, 30, 30);
    ucg.setPrintPos(82, 116);
    ucg.print(F("less"));

    ProbeScreenSwitch = 0;
    
  } else if (Condition == 0) {
    ucg.setPrintPos(11, 81);
    ucg.print(F("Quality: "));
    ucg.setColor(0, 255, 255, 255);
    ucg.print(F("--     "));

    ProbeScreenSwitch = 1;

    ucg.setPrintPos(11, 102);
    ucg.print(F("Insert the probe"));
    ucg.setPrintPos(11, 116);
    ucg.print(F("into your lawn."));
    
  } else if (Condition == 2) {
    ucg.setPrintPos(11, 81);
    ucg.print(F("Quality: "));
    ucg.setColor(0, 0, 255, 0);
    ucg.print(F("PERFECT"));
    
    
    ucg.setPrintPos(11, 116);
    ucg.setColor(0, 204, 229, 255);
    ucg.print(F("the same!"));

    ProbeScreenSwitch = 0;
    
  } else if (Condition == 3) {
    ucg.setPrintPos(11, 81);
    ucg.print(F("Quality: "));
    ucg.setColor(0, 255, 0, 0);
    ucg.print(F("TOO DRY"));
    
    ucg.setPrintPos(11, 116);
    ucg.setColor(0, 204, 229, 255);
    ucg.print(F("2 minute(s) "));
    ucg.setColor(0, 255, 30, 30);
    ucg.setPrintPos(82, 116);
    ucg.print(F("more"));

    ProbeScreenSwitch = 0;
    
  }

  
  
}

// displays the two boxes
void DispBox() 
{
  // Draw rounded rectangle:
  ucg.setColor(0, 76, 153);  // Set color (0,R,G,B)
  ucg.drawRFrame(5, 60, 118, 45, 8);  // Start from top-left pixel (x,y,wigth,height,radius)

  // Draw rounded rectangle:
  ucg.setColor(0, 76, 153);  // Set color (0,R,G,B)
  ucg.drawRFrame(5, 110, 118, 45, 8);  // Start from top-left pixel (x,y,wigth,height,radius)
}


void DispSmallBox() 
{
  ucg.setColor( 0, 255, 255);
  ucg.drawRFrame(5, 125, 118, 35, 7);
  ucg.drawRFrame(5, 87, 118, 35, 7);
}

int soilCond(byte mois) {
  int cond;
  if (mois > 85) {
    cond = 1;
  } else if (mois > 66) {
    cond = 2;
  } else if (mois > 0) {
    cond = 3;
  } else if (mois == 0) {
    cond = 0;
  } else {
    cond = 4;
  }
  return cond;
}



byte waterLawn() {
  int watering;
  watering = 2;
  return watering;
}

void receive(int numBytes) {}
