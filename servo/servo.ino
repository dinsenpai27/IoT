#include <Servo.h>

Servo servo1; // Membuat objek servo

void setup() {
  servo1.attach(3);
// Menghubungkan servo ke pin digital 3
}

void loop() {
servo1.write(0);
delay(1000);
  servo1.write(180);
  delay(1000);
  // Menggerakkan servo ke posisi kanan bawah (90 derajat)
     // Tunggu 1 detik
}
