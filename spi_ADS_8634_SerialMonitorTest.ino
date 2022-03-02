//code for the Teensy to read the ADS 8634 chip and observe values on the serial monitor

//this will print out the two bytes coming in from Dout and the channel number on one line and then
//the converted voltage value and time-stamp on the second line
//this will be done 5 times for the buffer size 
//then there will be a 1 second pause

//you can feel free to change these variables for various testing:  buff, trig, horiz, channelNumber

#include <SPI.h>
#include <Chrono.h>

const int CS = 10;                                                      //digital pin for chip select
const int buff = 5;                                                     //number of samples for the buffer 
double trig = 0.1;                                                      //trigger to start readings in volts
const int Vref = 2.5;                                                   //internal reference 2.5V
const int clk = 20000000;                                               //max clock speed for ADS 8634 chip 20Mhz
int horiz = 500;                                                        //horizontal division user input in us - min 500us to get 200ksps 
int horizDiv = 5;                                                       //the number of horizontal divisions in serial plotter (half of the full set...?)
int sampleTime = ((horiz*horizDiv)/buff);                               //time between samples in useconds
int sample0Time = 0;                                                    //set the initial sample read to zero 
byte channelSelection = {B10000000, B00100000, B00001000, B00000010};   //Ch0 x Ch1 x Ch2 x Ch3 x
int channelNumber = 0;                                                  //index for channel number selection

typedef struct
{
  byte b1; //first bits: A3 A2 A1 A0 D11 D10 D9 D8  (AX is channel address, DX is digital data)
  byte b2; //last bits: D7 D6 D5 D4 D3 D2 D1
  int ch;  //channel number of reading
} sampleIn_type;

typedef struct
{
  double V;       //analog voltage value
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

  //send chip settings writeSPI(reg, setting)  reg bits:  R6 R5 R4 R3 R2 R1 R0 R/W  Write = 0

  digitalWrite(CS, LOW);  //check how long digitalWrite takes - long enough for td(CS-DO) = 40 ns?
    //use power-up/power-down feature...?  maybe useful after data set on computer...?  
    //AL/PD pin is floating - is this okay? YES if programmed as a power-down input
    //register address: 06h, default: 00001000  (Aux-Config: 0000 |AL_PD=1 for power-down| |IntVref=1 to use internal ref| |TempSensor=0 not using temp sensor| 0)
  writeSPI(B00001100, B00001100);  //register is 7-bit address, MSB is R/W: Write = 0
  digitalWrite(CS, HIGH);

  digitalWrite(CS, LOW);
    //auto scan  - what should default be...?  always scanning all four is easiest...?
    //register: 0Ch, default: 00000000 - results in channel 0 being selected  (Auto-Md Ch-Sel: Ch0 X Ch1 X Ch2 X Ch3 X)
  writeSPI(B00011000, channelSelection[channelNumber]);
  digitalWrite(CS, HIGH);

  SPI.endTransaction();
  
  
}

void loop() {
  // put your main code here, to run repeatedly:

  SPI.beginTransaction(SPISettings(clk, MSBFIRST, SPI_MODE0));

  do {
    intervalTimer.restart();
    readSPI(0);
  }while (sampleOut[0].V < trig);           //this is the trigger code

  sample0Time = micros();
  //Serial.println(intervalTimer.elapsed());
  //intervalTimer.restart();

  for (int i = 1; i < buff; i++) {

    while (intervalTimer.hasPassed(sampleTime)==0) {} //wait for the sample interval time

    intervalTimer.restart();
    
    readSPI(i);

    sampleOut[i].t = micros()-sample0Time;
    //Serial.println(intervalTimer.elapsed());
    //intervalTimer.restart();
  }

  SPI.endTransaction();

  for (int j = 0; j < buff; j++) {
    Serial.print(sampleIn[j].b1);
    Serial.print("\t");
    Serial.print(sampleIn[j].b2);
    Serial.print("\t");
    Serial.println(sampleIn[j].ch);
    
    Serial.print(sampleOut[j].V*2);   //multiply the reading by two b/c of the reduction 
    Serial.print("\t");
    Serial.println(sampleOut[j].t);
    
    //Serial.println(intervalTimer.elapsed()); //4 us to print 4 values with 4 characters
    
    //delayMicroseconds(sampleTime); //need delay here to stop scroll? YES
  }

  //if (horiz <= 5000) {
  //delay(250);  //or 10,000/horiz - sometimes stalls at approx 49,000 data points in plotter - pullup resistor issue?  wire length issue?
  //}
  delay(1000);
  Serial.println();

}


void writeSPI(byte reg, byte setting) {

  digitalWrite(CS, LOW); //turn on the chip select (CS)
  SPI.transfer(reg);
  SPI.transfer(setting);
  digitalWrite(CS, HIGH);
 
}


void readSPI(int arrayNum) {

  int x, y = 0;

  digitalWrite(CS, LOW); //turn on the chip select (CS)

  //get the 16-bit digital number from the ADC
  sampleIn[arrayNum].b1 = SPI.transfer(0); //receiving channel number and first 4 MSB
  sampleIn[arrayNum].b2 = SPI.transfer(0); //receiving the last 8 bits
    
  //process the digital bytes back into the channel number
  sampleIn[arrayNum].ch = (sampleIn[arrayNum].b1 & B11110000) / 32; //channel number shifted down from 2^7 to 2^2

  //process the digital bytes back into the analog voltage
  x = (sampleIn[arrayNum].b1 & B00001111) * 256;  //integer value of shifted up MSBs from 2^3 to 2^11
  y = sampleIn[arrayNum].b2 & B11111111;  //integer value of LSBs
  sampleOut[arrayNum].V = (Vref/4095.0)*(x + y);  //convert digital back to analog

  digitalWrite(CS, HIGH); //turn off the chip select (CS)
}
