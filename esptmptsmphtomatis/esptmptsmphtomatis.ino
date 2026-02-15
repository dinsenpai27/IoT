#include <ESP32Servo.h>

Servo servo;

#define SERVO_PIN 13   // Ganti sesuai pin yang kamu pakai

void setup() {
  Serial.begin(115200);   // ESP32 pakai baudrate lebih tinggi
  servo.attach(SERVO_PIN);
  
  servo.write(180);      // Posisi awal: tertutup
  Serial.println("ESP32 Ready");
}

void loop() {
  if (Serial.available()) {
    char cmd = Serial.read();

    if (cmd == 'O') {          // Open
      Serial.println("OPEN");
      servo.write(0);         // Buka pintu
      delay(3000);            // Tahan 3 detik
      servo.write(180);       // Tutup kembali
      Serial.println("CLOSE");
    }
  }
}
 