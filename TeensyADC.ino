#include <SPI.h>
#include <Chrono.h>

char start_byte = '0';
char settings_bytes[20] = {'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0'};  //settings are sent in this order  [Vert1, Vert2, Vert3, Vert4, 
                                                 //Chs, Vert1, Vert2, Vert3, Vert4, 
                                                 //Vert1Scale, Vert2Scale, Vert3Scale, Vert4Scale,
                                                 //Horiz, HorizScale, 
                                                 //TrigCh, TrigType, TrigSign,
                                                 //TrigLevel[0], TrigLevel[1], TrigLevel[2], TrigLevel[3]],
                                                 //acquire, endmessage
String settings_strings;
byte chDict[16] = {B00000000, B00000010, B00001000, B00001010, B00100000, B00100010, B00101000, B00101010, 
                   B10000000, B10000010, B10001000, B10001010, B10100000, B10100010, B10101000, B10101010};
int numChDict[16] = {1, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};
char oldChannel = '0';
int numChannels = 0;

bool channelChange = 0;
bool printAcquisition = 0;
bool printSerialPlotter = 0;
bool readSerialAgain = 0;

int acquisitionSetting = 0;
char acquisitionBuff[1] = {'0'};
char sendLetter = '0';

const int CS = 10;                                        //digital pin for chip select
const int buff = 500;                                     //number of samples for the buffer 
const double Vref = 2.5;                                     //internal reference 2.5V
const int clk = 20000000;                                 //max clock speed for ADS 8634 chip 20Mhz
int VrefMult = 4;                                         //default set for +-10V readings

int horizDiv = 5;                                         //horiz divisions in the Serial Plotter
int sampleTime = 0;                                       //time between samples based on horiz user entry
int sample0Time = 0;
int sampleCount = 0;

bool trigFlag = 0;
double trigLevel = 0;
int trigChannel = 0;
int trigType = 0;
double trigSample1 = 0;
double trigSample2 = 0;

double spV1 = 0;
double spV2 = 0;
double spV3 = 0;
double spV4 = 0;

typedef struct
{
  byte b1[4]; //first bits: A3 A2 A1 A0 D11 D10 D9 D8  (AX is channel address, DX is digital data)
  byte b2[4]; //last bits: D7 D6 D5 D4 D3 D2 D1
  int ch[4];  //channel number of reading
} sampleIn_type;

typedef struct
{
  double V[4];       //analog voltage value
  int t;             //time of the reading in microseconds
} sampleOut_type;

sampleIn_type sampleIn[buff];  

sampleOut_type sampleOut[buff];  

Chrono intervalTimer(Chrono::MICROS);

void setup() {
  // put your setup code here, to run once:

  pinMode(CS, OUTPUT);
  digitalWrite(CS, HIGH); //disable chip select pin so no data is sent before ready

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

  Serial.begin(9600);  //still need serial for when the data goes to CSV file...?
  SerialUSB1.begin(9600);
  
  while(!Serial){}  //wait until the connection to the PC is established
  
  Serial.println("Ch1:,Ch2:,Ch3,Ch4");
  delay(200);

}

void loop() {
  // put your main code here, to run repeatedly:

  if (SerialUSB1.available() > 0) {
    start_byte= SerialUSB1.read();

    oldChannel = channelOld(settings_bytes[0]);
    
    if (start_byte == 's'){
      SerialUSB1.write('s');

      //while (SerialUSB.available() <= 0) {}  //wait for Python to send settings
      SerialUSB1.readBytesUntil('e', settings_bytes, 20);
      start_byte = '0';
      printSerialPlotter = 1;
      
      settings_strings=String(settings_bytes);

      if (oldChannel != settings_bytes[0]) {
        channelChange = 1;
      }
      numChannels = numChDict[settings_bytes[0]-65];
      
      trigLevel = triggerValue(settings_bytes[13],settings_bytes[14],settings_bytes[15],
                               settings_bytes[16],settings_bytes[17]);
      trigChannel = settings_bytes[11]-48;
      trigType = settings_bytes[12]-48;
      trigFlag = 0;
                               
      sampleTime = ((int(settings_bytes[9])-48)*pow(10,int(settings_bytes[10])-48)*horizDiv)/buff; //update from default values

      if (sampleCount > 0) {
        sampleCleanup(sampleCount);
        //Serial.print("SampleCount");
      }
      sampleCount = 0;

      acquisitionSetting = settings_bytes[18]-48;
      if (acquisitionSetting == 1) {
        readSerialAgain = 1;
      }
      //Serial.print(settings_bytes[18]-48);
    }

    if (readSerialAgain == 1) {
      //Serial.print("acquisitionSettingLoop");
      while (SerialUSB1.available() <= 0) {} //wait for Python to send the 'a' character
      
      SerialUSB1.readBytesUntil('a', acquisitionBuff, 1);
      SerialUSB1.write('a');
      printAcquisition = 1;
      printSerialPlotter = 0;
      readSerialAgain = 0;
        
      //Serial.println(printAcquisition);
    }
    
  }

  
  SPI.beginTransaction(SPISettings(clk, MSBFIRST, SPI_MODE0));
    
  if (channelChange == 1) {
    writeSPI(B00011000, chDict[settings_bytes[0]-65]);  //change the auto-scan channels from the settings
    for (int k = 0; k < 4; k++) {  //do a full set of dummy reads to make sure channels are changed for new sequence
      SPI.transfer(B00000000);
      SPI.transfer(B00000000); 
    }
    channelChange = 0;
    //Serial.println("ChannelChange");
    //Serial.println(settings_bytes);
  }

  if (printSerialPlotter || printAcquisition) {
    
    if (trigFlag == 0) {
      trigSample1 = triggerReadSPI(0, numChannels, trigChannel);  
      trigSample2 = triggerReadSPI(0, numChannels, trigChannel);
      for (int j = 0; j < numChannels; j++) {
            readSPI(0, j);
          }
      trigFlag = triggerFunc(trigLevel, trigSample1, trigSample2, trigType);
      trigSample1 = 0;
      trigSample2 = 0;
      //Serial.print("trigStuck");
    }
  
    if (trigFlag == 1) {
      
      if((settings_bytes[10]-48 == 2) && (settings_bytes[9]-48 == 5) && (numChDict[settings_bytes[0]-65] == 4)) { //maximum sample frequency of 20ksps
        sample0Time = micros();
        
        for (int i = 1; i < buff; i++) {
          
          for (int j = 0; j < 4; j++) {
            readSPI(i, j);
          }

          sampleOut[i].t = micros()-sample0Time;
        }

        sampleCount = buff;
      }
      
      else if (settings_bytes[10]-48 == 2) {
        intervalTimer.restart();
        sample0Time = micros();

        for (int m = 1; m < buff; m++) {
          while(intervalTimer.hasPassed(sampleTime) != 1) {}
          intervalTimer.restart();

          for (int n = 0; n < numChannels; n++) {
            readSPI(m, n);
          }
          sampleOut[m].t = micros()-sample0Time;
        }

        sampleCount = buff;
      }
      
      else{
        if(sampleCount == 0) {
          intervalTimer.restart();
          sample0Time = micros();
          sampleCount = sampleCount + 1;
        }
    
        if(intervalTimer.hasPassed(sampleTime)) {
      
          intervalTimer.restart();
        
          for (int k = 0; k < numChannels; k++) {
        
            readSPI(sampleCount, k);
        
          }
          
          sampleOut[sampleCount].t = micros()- sample0Time;
  
          sampleCount = sampleCount + 1;
          //Serial.println(sampleCount);
        }
      }
    }
    
    SPI.endTransaction();

    if (sampleCount == buff) {

      for (int i = 0; i < buff; i++) {

        for (int j = 0; j < numChannels; j++) {
          bytesDecode(i, j);
        }
        
      }

      if (printSerialPlotter == 1) {

        for (int k = 0; k < buff; k++) {
          spV1 = (sampleOut[k].V[0]*4)/((int(settings_bytes[1])-48)*pow(10,-(int(settings_bytes[5])-48)));
          if ((settings_bytes[1] == '0') || (spV1 <= -20)) {Serial.print(0);}
          else if (spV1 >= 19) {Serial.print(4000);}
          else {Serial.print(map(spV1,-20, 20, 0, 4095));}
          Serial.print(',');
          sampleOut[k].V[0] = 0;

          spV2 = (sampleOut[k].V[1]*4)/((int(settings_bytes[2])-48)*pow(10,-(int(settings_bytes[6])-48)));
          if ((settings_bytes[2] == '0') || (spV2 <= -20)) {Serial.print(0);}
          else if (spV2 >= 19) {Serial.print(4000);}
          else {Serial.print(map(spV2,-20, 20, 0, 4095));}
          Serial.print(',');
          sampleOut[k].V[1] = 0;
      
          spV3 = (sampleOut[k].V[2]*4)/((int(settings_bytes[3])-48)*pow(10,-(int(settings_bytes[7])-48)));
          if ((settings_bytes[3] == '0') || (spV3 <= -20)) {Serial.print(0);}
          else if (spV3 >= 19) {Serial.print(4000);}
          else {Serial.print(map(spV3,-20, 20, 0, 4095));}
          Serial.print(',');
          sampleOut[k].V[2] = 0;

          spV4 = (sampleOut[k].V[3]*4)/((int(settings_bytes[4])-48)*pow(10,-(int(settings_bytes[8])-48)));
          if ((settings_bytes[4] == '0') || (spV4 <= -20)) {Serial.print(0);}
          else if (spV4 >= 19) {Serial.print(4000);}
          else {Serial.print(map(spV4,-20, 20, 0, 4095));}
          Serial.print(',');
          sampleOut[k].V[3] = 0;
            
          Serial.print(4096/2);
          Serial.print(',');
          Serial.print(4000);
          Serial.print(',');
          Serial.print(0);
          Serial.print('\n');
          
        }
        
      }


      if (printAcquisition == 1) {
    
        for (int i = 1; i <= buff/100; i++) {
   
          for (int j = i*100-100 ; j < i*100; j++) {
            SerialUSB1.print(sampleOut[j].V[0]);
            SerialUSB1.print(',');
            sampleOut[j].V[0] = 0;
            SerialUSB1.print(sampleOut[j].V[1]);
            SerialUSB1.print(',');
            sampleOut[j].V[1] = 0;
            SerialUSB1.print(sampleOut[j].V[2]);
            SerialUSB1.print(',');
            sampleOut[j].V[2] = 0;
            SerialUSB1.print(sampleOut[j].V[3]);
            SerialUSB1.print(',');
            sampleOut[j].V[3] = 0;
            SerialUSB1.print(sampleOut[j].t);
            sampleOut[j].t = 0;
            SerialUSB1.print('\n');
          }
          
          sendLetter = 100+i;
          SerialUSB1.print(sendLetter);
        }

        printAcquisition = 0;
        
      }

      trigFlag = 0;
      sampleCount = 0;
      delay(500);
      
    }
    
  }
  
}

void writeSPI(byte reg, byte setting) {

  digitalWrite(CS, LOW); //turn on the chip select (CS)
  SPI.transfer(reg);
  SPI.transfer(setting);
  digitalWrite(CS, HIGH);
 
}


void readSPI(int arrayNum, int readNum) {

  digitalWrite(CS, LOW); //turn on the chip select (CS)

  //get the 16-bit digital number from the ADC
  sampleIn[arrayNum].b1[readNum] = SPI.transfer(B00000000); //receiving channel number and first 4 MSB
  sampleIn[arrayNum].b2[readNum] = SPI.transfer(B00000000); //receiving the last 8 bits

  digitalWrite(CS, HIGH); //turn off the chip select (CS)
}

void bytesDecode(int arrayNum, int readNum) {
  int x, y = 0;

  //process the digital bytes back into the channel number
  sampleIn[arrayNum].ch[readNum] = (sampleIn[arrayNum].b1[readNum] & B01100000) / 32; //channel number shifted down from 2^6 to 2^2

  //process the digital bytes back into the analog voltage
  x = (sampleIn[arrayNum].b1[readNum] & B00001111) * 256;  //integer value of shifted up MSBs from 2^3 to 2^11
  y = sampleIn[arrayNum].b2[readNum] & B11111111;  //integer value of LSBs
  sampleOut[arrayNum].V[sampleIn[arrayNum].ch[readNum]] = 2*((((2.0*Vref*VrefMult)/4095.0)*(x + y))-(Vref*VrefMult));  //convert digital back to analog
  
}

double triggerReadSPI(int arrayNum, int readNum, int trigCh) {
  for (int i = 0; i < readNum; i++) {
    digitalWrite(CS, LOW); //turn on the chip select (CS)

    //get the 16-bit digital number from the ADC
    sampleIn[arrayNum].b1[readNum] = SPI.transfer(B00000000); //receiving channel number and first 4 MSB
    sampleIn[arrayNum].b2[readNum] = SPI.transfer(B00000000); //receiving the last 8 bits

    digitalWrite(CS, HIGH); //turn off the chip select (CS)
  
    int w, x, y, z = 0;

    //process the digital bytes back into the channel number
    w = sampleIn[arrayNum].ch[readNum] = (sampleIn[arrayNum].b1[readNum] & B01100000) / 32; //channel number shifted down from 2^6 to 2^2

    //process the digital bytes back into the analog voltage
    x = (sampleIn[arrayNum].b1[readNum] & B00001111) * 256;  //integer value of shifted up MSBs from 2^3 to 2^11
    y = sampleIn[arrayNum].b2[readNum] & B11111111;  //integer value of LSBs
    z = sampleOut[arrayNum].V[sampleIn[arrayNum].ch[readNum]] = 2*((((2.0*Vref*VrefMult)/4095.0)*(x + y))-(Vref*VrefMult));  //convert digital back to analog

    if (w == trigCh) {
      return z;
    }
  }
}

bool triggerFunc(double L, double S1, double S2, int T) { //this is the trigger code
  if(T == 0) { // free run
    return 1;
  } 
  if(T == 1) { //rising trigger
    if(S1 < S2 && S2 > L) {
      return 1;
    }
    else {
      return 0;
    }
  }
  else if(T == 2) { //falling trigger
    if(S1 > S2 && S2 < L) {
      return 1;
    }
    else {
      return 0;
    }
  }
  else if(T == 3 && S2 > L){// > trigger
      return 1;
    }
  else if(T == 4 && S2 < L){// < trigger
      return 1;
  }
  else {
    return 0;
  }
}

float triggerValue(char a, char b, char c, char d, char e){

  float g;

  g = 10.0*(b-48) + c-48 + 0.10*(d-48) + .01*(e-48);

  if(a==49){g = g * -1;

  }

  return g; //return result to function

}

void sampleCleanup(int c) {
  
  for (int i = 0; i < c; i++) {
    sampleOut[i].V[0] = 0;
    sampleOut[i].V[1] = 0;
    sampleOut[i].V[2] = 0;
    sampleOut[i].V[3] = 0;
    sampleOut[i].t = 0;
  }
  
}

char channelOld(char c) {
  return c;
}
