char start_byte = '0';
char settings_bytes[19] = {'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0'};  //settings are sent in this order  [Vert1, Vert2, Vert3, Vert4, 
                                                 //Vert1Scale, Vert2Scale, Vert3Scale, Vert4Scale, Horiz, HorizScale, 
                                                 //TrigCh, TrigType, TrigScale, 
                                                 //TrigLevel[0], TrigLevel[1], TrigLevel[2], TrigLevel[3]]
bool printSettings = 0;

byte chDict[16] = {B00000000, B00000010, B00001000, B00001010, B00100000, B00100010, B00101000, B00101010, 
                   B10000000, B10000010, B10001000, B10001010, B10100000, B10100010, B10101000, B10101010};
                

void setup() {
  // put your setup code here, to run once:
  
  Serial.begin(9600);
  SerialUSB1.begin(9600);

  while(!Serial){}  //wait until the connection to the PC is established
   Serial.println("Start");

}

void loop() {
  // put your main code here, to run repeatedly:

  if (SerialUSB1.available() > 0) {
    start_byte= SerialUSB1.read();
  
    if (start_byte == 's'){
      SerialUSB1.write('s');
  
      SerialUSB1.readBytesUntil('e', settings_bytes, 19);
      start_byte = '0';
      printSettings = 1;
    }
  }

  if(printSettings == 1) {
    Serial.print("ChList: ");
    Serial.println(chDict[settings_bytes[0]-65]);
    Serial.print("Ch1: ");
    Serial.print(settings_bytes[1]);
    Serial.print(" x 10^-");
    Serial.print(settings_bytes[5]);
    Serial.println(" V");
    Serial.print("Ch2: ");
    Serial.print(settings_bytes[2]);
    Serial.print(" x 10^-");
    Serial.print(settings_bytes[6]);
    Serial.println(" V");
    Serial.print("Ch3: ");
    Serial.print(settings_bytes[3]);
    Serial.print(" x 10^-");
    Serial.print(settings_bytes[7]);
    Serial.println(" V");
    Serial.print("Ch4: ");
    Serial.print(settings_bytes[4]);
    Serial.print(" x 10^-");
    Serial.print(settings_bytes[8]);
    Serial.println(" V");
    Serial.print("Horiz: ");
    Serial.print(settings_bytes[9]);
    Serial.print(" x 10^");
    Serial.print(settings_bytes[10]);
    Serial.println(" us");
    Serial.print("TrigCh: ");
    Serial.println(settings_bytes[11]);
    Serial.print("TrigEdge (0=Free, 1=Rise, 2=Fall): ");
    Serial.println(settings_bytes[12]);
    Serial.print("TrigLevel: ");
    Serial.print(settings_bytes[14]);
    Serial.print(settings_bytes[15]);
    Serial.print(settings_bytes[16]);
    Serial.print(settings_bytes[17]);
    Serial.print(" x 10^-");
    Serial.print(settings_bytes[13]);
    Serial.println(" V");
    Serial.println();
    printSettings = 0;
  }

}
