//code for the Teensy to read the MCP 3008 chip
/* V2 allow for processing code while waiting for next sample
 *  add rising and falling trigger functions
 */

//TRY THREADS!!!!!!!!!  probably don't need...?

#include <SPI.h>
#include <Chrono.h>

const int CS = 10;                                  //digital pin for chip select
const int buff = 10;                               //number of samples for the buffer 
double trig = 2;                                    //trigger to start readings
const int Vref = 5;                                 //reference voltage sent to the MCP 3008 by the 5V output on the Teensy
const int clk = 3600000;                            //max clock speed for MCP 3008 chip
int horiz = 5000;                                   //horizontal division user input in us - min 500us to get 200ksps 
int horizDiv = 5;                                   //the number of horizontal divisions in serial plotter (half of the full set...?)
int sampleTime = ((horiz*horizDiv)/buff);           //time between samples in useconds
int sample0Time = 0;                                //program time of the initial sample in useconds 
bool trigFlag=0;                                      //trigger flag
int samplecount = 1;                                //samplecount
int trigType = 1;                                       //trigger type, 0 is freerun, 1 is rising, 2 is falling
int trigChannel = 0;                                    //trigger channel, 0-3 respectively

typedef struct
{
  //byte b1; //nothing: XXXXXXXX
  byte b2; //first bits: X X X X X null B9 B8
  byte b3; //last bits: B7 B6 B5 B4 B3 B2 B1
} sampleIn_type;

typedef struct
{
  int ch;         //channel of the reading
  double V;       //analog voltage value
  int t;          //time of the reading in microseconds
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
/*sample ADC on trigger channel as fast as possible. When trigger conditions are
 * met get samples.
 */
  SPI.beginTransaction(SPISettings(clk, MSBFIRST, SPI_MODE0));
  triggerFunc(); // check trigger
    //Serial.println(intervalTimer.elapsed());
    //intervalTimer.restart();
  if(intervalTimer.hasPassed(sampleTime) && trigFlag==1) { //get sample after interval time
    if(samplecount == 1) { // set sample0Time on first sample
      sample0Time = micros();
    }
    if(samplecount <= buff) {
      intervalTimer.restart();
      readSPI(samplecount, B00000001, B10000000);
      sampleOut[samplecount].t = micros()-sample0Time; //don't really need cuz we know the interval
      samplecount++; // increment buffer count
      //Serial.println(intervalTimer.elapsed());
      //intervalTimer.restart();
    }
  }
  SPI.endTransaction();
  if(samplecount == buff+1) { //buffer has all samples, add one due to increment above
    for (int j = 0; j < buff; j++) {
      //Serial.print(sampleOut[j].t);
      //Serial.print("\t");
      
      Serial.print(sampleOut[j].V);
      Serial.print("\t");
      Serial.print(sampleOut[j].V+.5);
      Serial.print("\t");
      Serial.print(sampleOut[j].V-.5);
      Serial.print("\t");
      Serial.println(-sampleOut[j].V);
    }
    samplecount = 1; // reset buffer counter
    trigFlag = 0; // reset trigger flag
    Serial.println();
  }
  //if (horiz <= 5000) {
    //delay(500);  //or 10,000/horiz - sometimes stalls at approx 49,000 data points in plotter - pullup resistor issue?  wire length issue?
  //}

}


void readSPI(int arrayNum, byte startBit, byte modeChannel) {

  int x, y, z = 0;

  //get the digital number from the ADC
  digitalWrite(CS, LOW); //turn on the chip select (CS)

  SPI.transfer(startBit); //sending the start bit and receiving garbage
  sampleIn[arrayNum].b2 = SPI.transfer(modeChannel); //sending the mode and channel and recieving the two MSB
  sampleIn[arrayNum].b3 = SPI.transfer(0); //receiving the last 8 bits
    
  //process the digital bytes back into the voltage data
  x = sampleIn[arrayNum].b2 & B00000011;  //last two bits of this message are the two MSB digital
  y = x * 256;  //convert to higher byte integer number
  z = sampleIn[arrayNum].b3 & B11111111;
  sampleOut[arrayNum].V = (5.0/1024.0)*(y + z);  //add high and low bytes for digital number and convert digital back to analog
  digitalWrite(CS, HIGH); //turn off the chip select (CS)
}
void triggerFunc() { //this is the trigger code
  if(trigType == 0) { // free run
    trigFlag = 1;
    return;
  }
  readSPI(0, B00000001, B10000000);
  double a = sampleOut[0].V; // move reading to register
  readSPI(0, B00000001, B10000000);
  if(trigType == 1) { //rising trigger
    if(sampleOut[0].V<a && a>trig) {
      trigFlag = 1; //set trigger flag
    }
  }
  if(trigType == 2) { //falling trigger
    if(sampleOut[0].V>a && a<trig) {
      trigFlag = 1; //set trigger flag
    }
  }
}
