//serial transfer test

int data = [[1, 2, 3, 4, 100], [5, 6, 7, 8, 200]];  //data format [ch0, ch1, ch2, ch3, time] - time is in us
int dataSize = 2;
double vertHoriz = [1, 1];  //vertical and horizontal scalar entered by user into serial monitor - initialized to 1

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);  //don't need this for the Teensy

}

void loop() {
  // put your main code here, to run repeatedly:

  for (int i = 0; i < dataSize; i++) {
   
    Serial.print(data[i][0]*vertHoriz[0]);
    Serial.print(', ');
    Serial.print(data[i][1]*vertHoriz[0]);
    Serial.print(', ');
    Serial.print(data[i][2]*vertHoriz[0]);
    Serial.print(', ');
    Serial.print(data[i][3]*vertHoriz[0]);
    Serial.print(', ');
    Serial.print(data[i][4]*vertHoriz[1]);
    Serial.print('\n');
    
  }

  delay(2000);
  

  while(Serial.available()) {
    
    vertHoriz[0] = Serial.readStringUntil(',');
    Serial.read();  //blank read on the comma
    vertHoriz[1] = Serial.readStringUntil('\n');  //set the serial monitor to "newline" 
    
    Serial.println(vertHoriz[0]);  //prints your entered values back on the monitor so you can see them
    Serial.println(vertHoriz[1]);
    
  }

  delay(2000);
  
}
