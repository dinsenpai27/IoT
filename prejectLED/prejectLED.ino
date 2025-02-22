#define ledRed 2
#define ledGreen 4
#define button 6
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  pinMode(button, INPUT);
  digitalWrite(button, HIGH);

  pinMode(ledRed, OUTPUT);
  pinMode(ledGreen, OUTPUT);

}
void speedMode(int option){
  if(option==1){
  digitalWrite(ledRed, HIGH);

  delay(1000);
  digitalWrite(ledRed, LOW);
  digitalWrite(ledGreen, HIGH);
  delay(1000);
  digitalWrite(ledGreen, LOW);
  }else {
  digitalWrite(ledRed, HIGH);
  delay(100);
  digitalWrite(ledRed, LOW);
  digitalWrite(ledGreen, HIGH);
  delay(100);
  digitalWrite(ledGreen, LOW);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  int buttonPressed = digitalRead(button);
  if(buttonPressed == 0){
    speedMode(1);
  } else {
    speedMode(0);
  }

  }
  
  

