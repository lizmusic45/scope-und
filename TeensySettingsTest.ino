char start_byte = '0';
char settings_bytes[18] = {'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0'};  //settings are sent in this order  [Vert1, Vert2, Vert3, Vert4, 
                                                 //Vert1Scale, Vert2Scale, Vert3Scale, Vert4Scale, Horiz, HorizScale, 
                                                 //TrigCh, TrigEdge, TrigScale, 
                                                 //TrigLevel[0], TrigLevel[1], TrigLevel[2], TrigLevel[3]]
bool printSettings = 0;

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
  
      SerialUSB1.readBytesUntil('e', settings_bytes, 18);
      start_byte = '0';
      printSettings = 1;
    }
  }

  if(printSettings == 1) {
    Serial.print("Ch1: ");
    Serial.print(settings_bytes[0]);
    Serial.print(" x 10^-");
    Serial.print(settings_bytes[4]);
    Serial.println(" V");
    Serial.print("Ch2: ");
    Serial.print(settings_bytes[1]);
    Serial.print(" x 10^-");
    Serial.print(settings_bytes[5]);
    Serial.println(" V");
    Serial.print("Ch3: ");
    Serial.print(settings_bytes[2]);
    Serial.print(" x 10^-");
    Serial.print(settings_bytes[6]);
    Serial.println(" V");
    Serial.print("Ch4: ");
    Serial.print(settings_bytes[3]);
    Serial.print(" x 10^-");
    Serial.print(settings_bytes[7]);
    Serial.println(" V");
    Serial.print("Horiz: ");
    Serial.print(settings_bytes[8]);
    Serial.print(" x 10^");
    Serial.print(settings_bytes[9]);
    Serial.println(" us");
    Serial.print("TrigCh: ");
    Serial.println(settings_bytes[10]);
    Serial.print("TrigEdge (1=Rise, 0=Fall): ");
    Serial.println(settings_bytes[11]);
    Serial.print("TrigLevel: ");
    Serial.print(settings_bytes[13]);
    Serial.print(settings_bytes[14]);
    Serial.print(settings_bytes[15]);
    Serial.print(settings_bytes[16]);
    Serial.print(" x 10^-");
    Serial.print(settings_bytes[12]);
    Serial.println(" V");
    Serial.println();
    printSettings = 0;
  }

}
