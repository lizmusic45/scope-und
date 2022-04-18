char start_byte = '0';
char settings_bytes[20] = {'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0'};  //settings are sent in this order  [Vert1, Vert2, Vert3, Vert4, 
                                                 //Vert1Scale, Vert2Scale, Vert3Scale, Vert4Scale, Horiz, HorizScale, 
                                                 //TrigCh, TrigType, TrigScale, 
                                                 //TrigLevel[0], TrigLevel[1], TrigLevel[2], TrigLevel[3]]
String settings_strings;

bool printSerialPlotter = 0;
bool printAcquisition = 0;
bool readSerialAgain = 0;
char sendLetter = '0';

int acquisitionSetting = 0;
char acquisitionBuff[1] = {'0'};

byte chDict[16] = {B00000000, B00000010, B00001000, B00001010, B00100000, B00100010, B00101000, B00101010, 
                   B10000000, B10000010, B10001000, B10001010, B10100000, B10100010, B10101000, B10101010};

int numChDict[16] = {1, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};

int sampleTime = 0;

double trigLevel = 0;

int f = 440;          //frequency
double w = 2*3.14*f;  //angular frequency

double spV1 = 0;
double spV2 = 0;
double spV3 = 0;
double spV4 = 0;

float z1 = 0;
float z2 = 0;
float z3 = 0;
float z4 = 0;



void setup() {
  // put your setup code here, to run once:
  
  Serial.begin(9600);
  SerialUSB1.begin(9600);

  while(!Serial){}  //wait until the connection to the PC is established
  Serial.println("Ch1:,Ch2:,Ch3,Ch4");
  delay(200);

}

void loop() {
  // put your main code here, to run repeatedly:

  if (SerialUSB1.available() > 0) {
    start_byte= SerialUSB1.read();
  
    if (start_byte == 's'){
      SerialUSB1.write('s');
  
      SerialUSB1.readBytesUntil('e', settings_bytes, 20);
      start_byte = '0';
      printSerialPlotter = 1;
      
      settings_strings=String(settings_bytes);

      trigLevel = triggerValue(settings_bytes[13],settings_bytes[14],settings_bytes[15],
                               settings_bytes[16],settings_bytes[17]);

      sampleTime = ((int(settings_bytes[9])-48)*pow(10,int(settings_bytes[10])-48)*5)/500;

      acquisitionSetting = settings_bytes[18]-48;
      if (acquisitionSetting == 1) {
        readSerialAgain = 1;
      }
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

  if(printSerialPlotter == 1) {
    for (int n = 0; n < 500; n++) {
      float y = w*n*sampleTime*pow(10, -6);
      
      if (settings_bytes[1] != '0') {z1 = sin(y);}          //frequency of f and a magnitude of 1
      else {z1 = 0;}
      if (settings_bytes[2] != '0') {z2 = 2+cos(y*10);}     //frequency of 10f and a DC shift of 2V
      else {z2 = 0;}
      if (settings_bytes[3] != '0') {z3 = 6*sin(y*2);}      //frequency of 2f and magnitude of 6
      else {z3 = 0;}
      if (settings_bytes[4] != '0') {z4 = (1/3.0)*cos(y*5);}  //frequency of 5f and magnitude of 1/3
      else {z4 = 0;}

      spV1 = (z1*4)/((int(settings_bytes[1])-48)*pow(10,-(int(settings_bytes[5])-48)));
      if ((settings_bytes[1] == '0') || (spV1 <= -20)) {Serial.print(0);}
      else if (spV1 >= 19) {Serial.print(4000);}
      else {Serial.print(map(spV1,-20, 20, 0, 4095));}
      Serial.print(',');
        
      spV2 = (z2*4)/((int(settings_bytes[2])-48)*pow(10,-(int(settings_bytes[6])-48)));
      if ((settings_bytes[2] == '0') || (spV2 <= -20)) {Serial.print(0);}
      else if (spV2 >= 19) {Serial.print(4000);}
      else {Serial.print(map(spV2,-20, 20, 0, 4095));}
      Serial.print(',');
      
      spV3 = (z3*4)/((int(settings_bytes[3])-48)*pow(10,-(int(settings_bytes[7])-48)));
      if ((settings_bytes[3] == '0') || (spV3 <= -20)) {Serial.print(0);}
      else if (spV3 >= 19) {Serial.print(4000);}
      else {Serial.print(map(spV3,-20, 20, 0, 4095));}
      Serial.print(',');
        
      spV4 = (z4*4)/((int(settings_bytes[4])-48)*pow(10,-(int(settings_bytes[8])-48)));
      if ((settings_bytes[4] == '0') || (spV4 <= -20)) {Serial.print(0);}
      else if (spV4 >= 19) {Serial.print(4000);}
      else {Serial.print(map(spV4,-20, 20, 0, 4095));}
      Serial.print(',');

      Serial.print(4096/2);
      Serial.print(',');
      Serial.print(4000);
      Serial.print(',');
      Serial.print(0);
      Serial.print('\n');
      Serial.print('\n');
      
    }
    printSerialPlotter = 0;
  }

  if (printAcquisition == 1) {
    
    for(int i = 1; i <= 5; i++) {
      
      for(int n = i*100-100; n < i*100; n++){
        
        float y = w*n*sampleTime*pow(10, -6);
        float z1 = sin(y);                              //frequency of f and a magnitude of 1
        float z2 = 2+cos(y*10);                         //frequency of 10f and a DC shift of 2V
        float z3 = 6*sin(y*2);                          //frequency of 2f and a magnitude of 6
        float z4 = (1/3.0)*cos(y*5);                      //frequency of 5f and a magnitude of 1/3

      
        if ((chDict[settings_bytes[0]-65] & B10000000)==128) {
          SerialUSB1.print(z1);
        }
        else {
          SerialUSB1.print(0);
        }
        SerialUSB1.print(',');
  
        if ((chDict[settings_bytes[0]-65] & B00100000)==32) {
          SerialUSB1.print(z2);
        }
        else {
          SerialUSB1.print(0);
        }
        SerialUSB1.print(',');
  
        if ((chDict[settings_bytes[0]-65] & B00001000)==8) {
          SerialUSB1.print(z3);
        }
        else {
          SerialUSB1.print(0);
        }
        SerialUSB1.print(',');
  
        if ((chDict[settings_bytes[0]-65] & B00000010)==2) {
          SerialUSB1.print(z4);
        }
        else {
          SerialUSB1.print(0);
        }
        SerialUSB1.print(',');
        
        SerialUSB1.print(n*sampleTime);
        SerialUSB1.print('\n');
      }
      
      sendLetter = 100+i;
      SerialUSB1.print(sendLetter);
      
    }

    printAcquisition = 0;
    
  }

}

// function

//input  char 0 pos, 1 for neg for plus 4 char numbers 0-9

//will return a float value

float triggerValue(char a, char b, char c, char d, char e){

  float g;

  g = 10.0*(b-48) + c-48 + 0.10*(d-48) + .01*(e-48);

  if(a==49){g = g * -1;

  }

  return g; //return result to function

}
