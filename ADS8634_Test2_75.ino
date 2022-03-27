#include <SPI.h>
#include <Chrono.h>

const int CS = 10;                                                      //digital pin for chip select
const int buff = 5;                                                     //number of samples for the buffer 
const int Vref = 2.5;                                                   //internal reference 2.5V
const int clk = 10000000;                                               //max clock speed for ADS 8634 chip 20Mhz
int VrefMult = 4;                                                       //default set for +-10V readings
int horiz = 500;
int horizDiv = 5;
int sampleTime = ((horiz*horizDiv)/buff);
int sample0Time = 0;
int sampleCount = 0;
bool settingsChange = 1;
byte chDict = B10100000;  //will be dictionary later
byte numChDict = 2;
bool trigFlag = 0;
int trigType = 2;
int trigChannel = 0;
double trigLevel = 2;

typedef struct
{
  byte b1[2]; //first bits: A3 A2 A1 A0 D11 D10 D9 D8  (AX is channel address, DX is digital data)
  byte b2[2]; //last bits: D7 D6 D5 D4 D3 D2 D1
  int ch[2];  //channel number of reading
} sampleIn_type;

typedef struct
{
  double V[2];       //analog voltage value
  int t;          //time of the reading in microseconds
} sampleOut_type;

sampleIn_type sampleIn[buff];  //clear these to prevent glitches...?

sampleOut_type sampleOut[buff];  //clear these to prevent glitches...?

Chrono intervalTimer(Chrono::MICROS);

void setup() {
  // put your setup code here, to run once:

  pinMode(CS, OUTPUT);
  digitalWrite(CS, HIGH); //disable chip select pin so no data is sent before ready

  //Serial.begin(9600);  //still need serial for when the data goes to CSV file...?
  SPI.begin();
  delay(1000);

  SPI.beginTransaction(SPISettings(clk, MSBFIRST, SPI_MODE0));
  //set initial settings
    //AL/PD pin is floating - is this okay? YES if programmed as a power-down input
    //register address: 06h, default: 00001000  (Aux-Config: 0000 |AL_PD=1 for power-down| |IntVref=1 to use internal ref| |TempSensor=0 not using temp sensor| 0)
  writeSPI(B00001100, B00001100);  //register is 7-bit address, MSB is R/W: Write = 0
  //set to auto-scan
    //register 05h, default:  00000000 - keeps current settings
  writeSPI(B00001010, B00000000);
  //set auto-scan channels
    //register 0Ch, default:  00000000 - scans only channel 0
  writeSPI(B00011000, B00000000);

  SPI.endTransaction();

}

void loop() {
  // put your main code here, to run repeatedly:

  SPI.beginTransaction(SPISettings(clk, MSBFIRST, SPI_MODE0));

  if(settingsChange == 1) {
   
    writeSPI(B00011000, chDict);  //change the auto-scan channels from the settings
    numChDict = 2;
    
  }

  if (trigFlag == 0) {
    trigFlag = triggerFunc(trigLevel, trigChannel, trigType);
    Serial.print("trigStuck");
  }
  
  if (trigFlag == 1) {

    if(sampleCount == 0) {
      intervalTimer.restart();
      sample0Time = micros();
      sampleCount = sampleCount + 1;
    }
  
    if(intervalTimer.hasPassed(sampleTime)) {
    
        intervalTimer.restart();
      
        for (int k = 0; k < numChDict; k++) {
      
          readSPI(sampleCount, k);
      
        }
        
        sampleOut[sampleCount].t = micros()- sample0Time;

        sampleCount = sampleCount + 1;
        
    }
  }
    
  SPI.endTransaction();

  if (sampleCount == buff) {

    for (int j = 0; j < buff; j++) {
      //Serial.print(sampleIn[j].b1[0]);
      //Serial.print("\t");
      //Serial.print(sampleIn[j].b2[0]);
      //Serial.print("\t");
      //Serial.println(sampleIn[j].ch[0]);

      //Serial.print(sampleIn[j].b1[1]);
      //Serial.print("\t");
      //Serial.print(sampleIn[j].b2[1]);
      //Serial.print("\t");
      //Serial.println(sampleIn[j].ch[1]);
    
      Serial.print(sampleOut[j].V[0]);
      Serial.print(",");
      Serial.print(sampleOut[j].V[1]);
      Serial.print(",");
      Serial.println(sampleOut[j].t);
    
    }

    Serial.println();
    sampleCount = 0;
    delay(1000);
  }

  settingsChange = 0;
}

void writeSPI(byte reg, byte setting) {

  digitalWrite(CS, LOW); //turn on the chip select (CS)
  SPI.transfer(reg);
  SPI.transfer(setting);
  digitalWrite(CS, HIGH);
 
}


void readSPI(int arrayNum, int readNum) {

  int x, y = 0;

  digitalWrite(CS, LOW); //turn on the chip select (CS)

  //get the 16-bit digital number from the ADC
  sampleIn[arrayNum].b1[readNum] = SPI.transfer(B00000000); //receiving channel number and first 4 MSB
  sampleIn[arrayNum].b2[readNum] = SPI.transfer(B00000000); //receiving the last 8 bits
    
  //process the digital bytes back into the channel number
  sampleIn[arrayNum].ch[readNum] = (sampleIn[arrayNum].b1[readNum] & B01100000) / 32; //channel number shifted down from 2^6 to 2^2

  //process the digital bytes back into the analog voltage
  x = (sampleIn[arrayNum].b1[readNum] & B00001111) * 256;  //integer value of shifted up MSBs from 2^3 to 2^11
  y = sampleIn[arrayNum].b2[readNum] & B11111111;  //integer value of LSBs
  sampleOut[arrayNum].V[sampleIn[arrayNum].ch[readNum]] = (((2.0*Vref*VrefMult)/4095.0)*(x + y))-(Vref*VrefMult);  //convert digital back to analog

  digitalWrite(CS, HIGH); //turn off the chip select (CS)
}

bool triggerFunc(double trigL, int trigCh, int chNums) { //this is the trigger code
  if(trigType == 0) { // free run
    return 1;
  }
  readSPI(0, chNums);
  double a = sampleOut[0].V[trigCh]; // move reading to register
  readSPI(0, chNums);
  if(trigType == 1) { //rising trigger
    if(sampleOut[0].V[trigCh]<a && a>trigL) {
      return 1;
    }
    else {
      return 0;
    }
  }
  else if(trigType == 2) { //falling trigger
    if(sampleOut[0].V[trigCh]>a && a<trigL) {
      return 1;
    }
    else {
      return 0;
    }
  }
  else if(trigType==3 && sampleOut[0].V[trigCh]>trigL){// > trigger
      return 1;
    }
  else if(trigType==4 && sampleOut[0].V[trigCh]<trigL){// < trigger
      return 1;
  }
  else {
    return 0;
  }
}
