#include <WiFi.h>

// Ganti sesuai jaringanmu
const char* ssid = "Anahilham";
const char* password = "Ranglow250619";

void setup() {
  Serial.begin(115200);

  // Hubungkan ke WiFi induk
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nTerhubung ke WiFi Induk");

  // Jadikan juga Access Point
  WiFi.softAP("WiFi_Extender", "12345678");
  Serial.println("ESP jadi repeater dengan SSID: WiFi_Extender");
}

void loop() {
  // tidak perlu apa-apa, hanya meneruskan
}