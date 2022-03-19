//code for the Teensy to read the MCP 3008 chip

//TRY THREADS!!!!!!!!!  probably don't need...?

#include <SPI.h>
#include <Chrono.h>

const int CS = 10;                                  //digital pin for chip select
const int buff = 5;                                 //number of samples for the buffer 
double trigV = 2;                                   //trigger to start readings
int trigCh = 0;                                     //trigger channel
const int Vref = 5;                                 //reference voltage sent to the MCP 3008 by the 5V output on the Teensy
const int clk = 3600000;                            //max clock speed for MCP 3008 chip
int horiz = 50000;                                  //horizontal division user input in us - min 500us to get 200ksps 
int horizDiv = 5;                                   //the number of horizontal divisions in serial plotter (half of the full set...?)
int sampleTime = ((horiz*horizDiv)/buff);           //time between samples in useconds
int sample0Time = 0;                                //initialize first sample time to zero
byte modeChannelSet[] = {B10000000, B10010000, B10100000, B10110000};   //set of channel number and mode settings
bool trigMet = 0;

typedef struct
{
  byte b1[4]; //first bits: X X X X X null B9 B8
  byte b2[4]; //last bits: B7 B6 B5 B4 B3 B2 B1
  int ch[4];  //channel number of the read
} sampleIn_type;

typedef struct
{
  double V[4];       //analog voltage value conversions for the 4 channels
  int t;             //time of the reading in microseconds
} sampleOut_type;

sampleIn_type sampleIn[buff];  //clear these to prevent glitches...?

sampleOut_type sampleOut[buff];  //clear these to prevent glitches...?

Chrono intervalTimer(Chrono::MICROS);


void setup() {
  // put your setup code here, to run once:
  pinMode(CS, OUTPUT);
  //Serial.begin(9600);
  Serial.println(sampleTime);
  digitalWrite(CS, HIGH); //disable chip select pin so no data is sent before ready
  SPI.begin();
  delay(1000);
}

void loop() {
  // put your main code here, to run repeatedly:

  SPI.beginTransaction(SPISettings(clk, MSBFIRST, SPI_MODE0));

  do {
    intervalTimer.restart();
    for (int k = 0; k < 4; k++) {
      readSPI(0, B00000001, modeChannelSet[k], k);
      if ((sampleIn[0].ch[k]==trigCh) && (sampleOut[0].V[trigCh]>trigV)){
        trigMet = 1;
        break;
      }
    }
  }while (trigMet == 0);
  trigMet = 0;           //this is the trigger code

  sample0Time = micros();
  //Serial.println(intervalTimer.elapsed());
  //intervalTimer.restart();

  for (int i = 1; i < buff; i++) {

    while (intervalTimer.hasPassed(sampleTime)==0) {} //wait for the sample interval time

    intervalTimer.restart();
    
    readSPI(i, B00000001, B10000000, 0);  //read ch0 in single mode
    readSPI(i, B00000001, B10010000, 1);  //read ch1 in single mode
    readSPI(i, B00000001, B10100000, 2);  //read ch2 in single mode
    readSPI(i, B00000001, B10110000, 3);  //read ch3 in single mode

    sampleOut[i].t = micros()-sample0Time;
    //Serial.println(intervalTimer.elapsed());
    //intervalTimer.restart();
  }

  SPI.endTransaction();

  for (int j = 0; j < buff; j++) {
    Serial.print(sampleOut[j].V[0]);
    Serial.print("\t");
    Serial.print(sampleOut[j].V[1]);
    Serial.print("\t");
    Serial.print(sampleOut[j].V[2]);
    Serial.print("\t");
    Serial.print(sampleOut[j].V[3]);
    Serial.print("\t");
    Serial.println(sampleOut[j].t);
    
    //Serial.println(intervalTimer.elapsed()); //4 us to print 4 values with 4 characters
    
    //delayMicroseconds(sampleTime); //need delay here to stop scroll? YES
  }

  //if (horiz <= 5000) {
  //delay(250);  //or 10,000/horiz - sometimes stalls at approx 49,000 data points in plotter - pullup resistor issue?  wire length issue?
  //}
  delay(5000);
  Serial.println();

}


void readSPI(int arrayNum, byte startBit, byte modeChannel, int readNum) {

  int x, y, z = 0;

  //get the digital number from the ADC
  digitalWrite(CS, LOW); //turn on the chip select (CS)
  
  SPI.transfer(startBit); //sending the start bit and receiving garbage
  sampleIn[arrayNum].b1[readNum] = SPI.transfer(modeChannel); //sending the mode and channel and recieving the two MSB
  sampleIn[arrayNum].b2[readNum] = SPI.transfer(0); //receiving the last 8 bits

  //get channel number from modeChannel byte
  sampleIn[arrayNum].ch[readNum] = (modeChannel & B00110000) / 16;
      
  //process the digital bytes back into the voltage data
  x = sampleIn[arrayNum].b1[readNum] & B00000011;  //last two bits of this message are the two MSB digital
  y = x * 256;  //convert to higher byte integer number
  z = sampleIn[arrayNum].b2[readNum] & B11111111;
  sampleOut[arrayNum].V[sampleIn[arrayNum].ch[readNum]] = (5.0/1024.0)*(y + z);  //add high and low bytes for digital number and convert digital back to analog
  digitalWrite(CS, HIGH); //turn off the chip select (CS)
}
