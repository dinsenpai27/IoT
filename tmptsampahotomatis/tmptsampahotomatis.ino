#include <Servo.h>

Servo servo;

void setup() {
  Serial.begin(9600);
  servo.attach(3);
  servo.write(180);   // posisi awal tertutup
}

void loop() {
  if (Serial.available()) {
    char cmd = Serial.read();

    if (cmd == 'O') {   // Open
      servo.write(0);  
      delay(3000);     // tahan 5 detik
      servo.write(180);
    }
  }
}
