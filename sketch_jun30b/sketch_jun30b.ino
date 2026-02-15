#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// ===== Konfigurasi WiFi & MQTT =====
const char* ssid = "Yasashi";                 // Ganti dengan SSID WiFi kamu
const char* password = "heriyasashi";         // Ganti dengan password WiFi kamu
const char* mqtt_server = "192.168.144.248";  // IP address dari broker Mosquitto
const char* mqtt_user = "esp32user";          // Ganti dengan username MQTT kamu
const char* mqtt_pass = "123456789";           // Ganti dengan password MQTT kamu

// ===== Pin ESP32 =====
#define RELAY_PIN 25
#define DHT_PIN 32
#define SOIL_PIN 33
#define DHT_TYPE DHT11

DHT dht(DHT_PIN, DHT_TYPE);
WiFiClient espClient;
PubSubClient client(espClient);

// ===== Fuzzyfikasi Suhu =====
float fsDingin(float t) {
  if (t <= 15) return 1;
  if (t > 15 && t <= 23) return (23 - t) / (23 - 15);
  return 0;
}
float fsAgakDingin(float t) {
  if (t > 15 && t <= 23) return (t - 15) / (23 - 15);
  if (t > 23 && t <= 27) return (27 - t) / (27 - 23);
  return 0;
}
float fsNormal(float t) {
  if (t < 23) return 0;
  if (t <= 27) return (t - 23) / (27 - 23);
  if (t <= 31) return (31 - t) / (31 - 27);
  return 0;
}
float fsPanas(float t) {
  if (t > 27 && t <= 35) return (t - 27) / (35 - 27);
  if (t > 35) return 1;
  return 0;
}

// ===== Fuzzyfikasi Kelembapan Tanah =====
float fkKering(float k) {
  if (k <= 40) return 1;
  if (k > 40 && k <= 60) return (60 - k) / (60 - 40);
  return 0;
}
float fkLembab(float k) {
  if (k < 27) return 0;
  if (k <= 50) return (k - 27) / (50 - 27);
  if (k <= 75) return (75 - k) / (75 - 50);
  return 0;
}
float fkBasah(float k) {
  if (k > 60 && k <= 100) return (k - 60) / (100 - 60);
  return 0;
}

// ===== Inferensi dan Defuzzifikasi =====
float inferensi(float suhu, float kelembapan) {
  float md[12];
  float dr[12] = {5, 5, 0, 5, 5, 0, 5, 5, 0, 5, 5, 0};

  float fs[4] = {fsDingin(suhu), fsAgakDingin(suhu), fsNormal(suhu), fsPanas(suhu)};
  float fk[3] = {fkKering(kelembapan), fkLembab(kelembapan), fkBasah(kelembapan)};

  md[0] = min(fs[0], fk[0]);
  md[1] = min(fs[0], fk[1]);
  md[2] = min(fs[0], fk[2]);
  md[3] = min(fs[1], fk[0]);
  md[4] = min(fs[1], fk[1]);
  md[5] = min(fs[1], fk[2]);
  md[6] = min(fs[2], fk[0]);
  md[7] = min(fs[2], fk[1]);
  md[8] = min(fs[2], fk[2]);
  md[9] = min(fs[3], fk[0]);
  md[10] = min(fs[3], fk[1]);
  md[11] = min(fs[3], fk[2]);

  float totalWeighted = 0;
  float totalInference = 0;

  for (int i = 0; i < 12; i++) {
    totalWeighted += md[i] * dr[i];
    totalInference += md[i];
  }

  if (totalInference == 0) return 0;
  return totalWeighted / totalInference;
}

// ===== Koneksi WiFi =====
void setup_wifi() {
  delay(100);
  Serial.println("Menghubungkan ke WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Terhubung!");
}

// ===== Reconnect MQTT =====
void reconnect() {
  while (!client.connected()) {
    Serial.print("Menghubungkan MQTT...");
    if (client.connect("ESP32Client", mqtt_user, mqtt_pass)) {
      Serial.println("Berhasil!");
      client.subscribe("tanaman/perintah");  // Optional: jika ingin langganan
    } else {
      Serial.print("Gagal. rc=");
      Serial.print(client.state());
      Serial.println(" Retry...");
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(RELAY_PIN, OUTPUT);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  float suhu = dht.readTemperature();  // suhu dari DHT11
  int soilValue = analogRead(SOIL_PIN);
  float kelembapanTanah = map(soilValue, 4095, 0, 0, 100);  // ubah ke %

  float durasiPenyiraman = inferensi(suhu, kelembapanTanah);
  durasiPenyiraman = constrain(durasiPenyiraman, 0, 5);

  if (durasiPenyiraman > 0) {
    digitalWrite(RELAY_PIN, HIGH);
    delay(durasiPenyiraman * 1000);
    digitalWrite(RELAY_PIN, LOW);
  } else {
    digitalWrite(RELAY_PIN, LOW);
  }

  String payload = String("{\"suhu\":") + suhu +
                   ",\"kelembapan_tanah\":" + kelembapanTanah +
                   ",\"durasi\":" + durasiPenyiraman + "}";

  client.publish("tanaman/status", payload.c_str());

  Serial.println(payload);

  delay(5000); // Delay 5 detik sebelum loop berikutnya
}
 