#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

// === Konfigurasi DHT11 ===
#define DHTPIN 25
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// === WiFi & MQTT ===
const char* ssid = "Yasashi";
const char* password = "0987654321";
const char* mqtt_server = "192.168.23.248"; // Ganti dengan IP broker MQTT (laptop kamu)

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
  client.setServer(mqtt_server, 1883); // perbaikan di sini
}

void loop() {
  if (!client.connected()) {
    connectmqtt();
  }
  client.loop();

  float suhu = dht.readTemperature();
  float kelembapan = dht.readHumidity();

  if (isnan(suhu) || isnan(kelembapan)) {
    Serial.println("Gagal membaca dari sensor DHT!");
    return;
  }

  Serial.print("Suhu: ");
  Serial.print(suhu);
  Serial.print(" °C, Kelembapan: ");
  Serial.print(kelembapan);
  Serial.println(" %");

  // Konversi nilai ke string lalu kirim ke MQTT
  String suhuStr = String(suhu);
  String kelembapanStr = String(kelembapan);

  client.publish("esp32/suhu", suhuStr.c_str());
  client.publish("esp32/kelembapan", kelembapanStr.c_str());

  delay(5000);
}
