#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

// === Konfigurasi DHT11 ===
#define DHTPIN 25
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// === Sensor kelembapan tanah ===
#define SOIL_MOISTURE_PIN 33  // Gunakan pin GPIO 33 untuk sensor tanah

// === WiFi & MQTT ===
const char* ssid = "Yasashi";
const char* password = "0987654321";
const char* mqtt_server = "192.168.23.248"; // Ganti dengan IP broker MQTT

WiFiClient espClient;
PubSubClient client(espClient);

// === Fungsi koneksi ke WiFi ===
void setup_wifi() {
  delay(100);
  Serial.print("Menghubungkan ke WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
   
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi terhubung!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

// === Fungsi koneksi ke MQTT Broker ===
void connectmqtt() {
  while (!client.connected()) {
    Serial.print("Menghubungkan ke MQTT... ");
    if (client.connect("esp32-client")) {
      Serial.println("Berhasil!");
    } else {
      Serial.print("Gagal, rc=");
      Serial.print(client.state());
      Serial.println(" coba lagi dalam 5 detik");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, 1883);

  // Konfigurasi pin input analog
  pinMode(SOIL_MOISTURE_PIN, INPUT);
}

void loop() {
  if (!client.connected()) {
    connectmqtt();
  }
  client.loop();

  // === Baca suhu dari DHT11 ===
  float suhu = dht.readTemperature();

  // === Baca kelembapan tanah (nilai analog dari 0–4095) ===
  int kelembapanTanahRaw = analogRead(SOIL_MOISTURE_PIN);
  float kelembapanTanahPersen = map(kelembapanTanahRaw, 0, 4095, 100, 0); // Kering = nilai tinggi

  if (isnan(suhu)) {
    Serial.println("Gagal membaca suhu dari DHT!");
    return;
  }

  // Tampilkan ke Serial Monitor
  Serial.print("Suhu: ");
  Serial.print(suhu);
  Serial.print(" °C, Kelembapan Tanah: ");
  Serial.print(kelembapanTanahPersen);
  Serial.println(" %");

   // Konversi ke string untuk MQTT
  String suhuStr = String(suhu);
  String tanahStr = String(kelembapanTanahPersen);

  // Publish ke MQTT
  client.publish("esp32/suhu", suhuStr.c_str());
  client.publish("esp32/kelembapanTanah", tanahStr.c_str());

  delay(5000);
}
