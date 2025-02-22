#include <Servo.h>

Servo myServo;  // Deklarasi Servo
int angle = 90; // Sudut awal

void setup() {
    Serial.begin(9600);  // Inisialisasi komunikasi serial
    myServo.attach(2);   // Servo terhubung ke pin 9
    myServo.write(angle); // Set posisi awal servo
}

void loop() {
    if (Serial.available() > 0) {
        angle = Serial.parseInt();  // Baca data sudut dari serial
        if (angle >= 0 && angle <= 180) {
            myServo.write(angle);   // Gerakkan servo sesuai sudut
        }
    }
}
