#include <SPI.h>

const int CS = 10;                                                      //digital pin for chip select
const int buff = 5;                                                     //number of samples for the buffer 
const int Vref = 2.5;                                                   //internal reference 2.5V
const int clk = 20000000;                                               //max clock speed for ADS 8634 chip 20Mhz
int VrefMult = 4;                                                       //default set for +-10V readings

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


void setup() {
  // put your setup code here, to run once:

  pinMode(CS, OUTPUT);
  digitalWrite(CS, HIGH); //disable chip select pin so no data is sent before ready

  //Serial.begin(9600);  //still need serial for when the data goes to CSV file...?
  SPI.begin();
  delay(1000);

  SPI.beginTransaction(SPISettings(clk, MSBFIRST, SPI_MODE0));
  
    //AL/PD pin is floating - is this okay? YES if programmed as a power-down input
    //register address: 06h, default: 00001000  (Aux-Config: 0000 |AL_PD=1 for power-down| |IntVref=1 to use internal ref| |TempSensor=0 not using temp sensor| 0)
  writeSPI(B00001100, B00001100);  //register is 7-bit address, MSB is R/W: Write = 0

  SPI.endTransaction();

}

void loop() {
  // put your main code here, to run repeatedly:

  SPI.beginTransaction(SPISettings(clk, MSBFIRST, SPI_MODE0));

    //set to auto-scan
    //register 05h, default:  00000000 - keeps current settings
  writeSPI(B00001010, B00000000);

  for (int i = 1; i < buff; i++) {

    readSPI(i);
    
  }

  SPI.endTransaction();


  for (int j = 0; j < buff; j++) {
    Serial.print(sampleIn[j].b1);
    Serial.print("\t");
    Serial.print(sampleIn[j].b2);
    Serial.print("\t");
    Serial.println(sampleIn[j].ch);
    
    Serial.print(sampleOut[j].V);
    Serial.print("\t");
    Serial.println(sampleOut[j].t);
    
  }

  Serial.println();
  delay(1000);

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
  sampleIn[arrayNum].b1 = SPI.transfer(B00000000); //receiving channel number and first 4 MSB
  sampleIn[arrayNum].b2 = SPI.transfer(B00000000); //receiving the last 8 bits
    
  //process the digital bytes back into the channel number
  sampleIn[arrayNum].ch = (sampleIn[arrayNum].b1 & B01100000) / 32; //channel number shifted down from 2^6 to 2^2

  //process the digital bytes back into the analog voltage
  x = (sampleIn[arrayNum].b1 & B00001111) * 256;  //integer value of shifted up MSBs from 2^3 to 2^11
  y = sampleIn[arrayNum].b2 & B11111111;  //integer value of LSBs
  sampleOut[arrayNum].V = (((2.0*Vref*VrefMult)/4095.0)*(x + y))-(Vref*VrefMult);  //convert digital back to analog

  digitalWrite(CS, HIGH); //turn off the chip select (CS)
}
