#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// WiFi
const char* ssid = "NAMA_WIFI_KAMU";
const char* password = "PASSWORD_WIFI_KAMU";

// MQTT
const char* mqtt_server = "192.168.1.100"; // Ganti dengan IP broker MQTT kamu
const int mqtt_port = 1883;
const char* mqtt_topic = "tanaman/data";

// GPIO
const int RELAY_PIN = 25;
const int DHT_PIN = 27;
const int SOIL_PIN = 32;

#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);

WiFiClient espClient;
PubSubClient client(espClient);

// ------------------- Fungsi Fuzzy --------------------
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

// ------------------ Setup WiFi + MQTT -------------------
void setup_wifi() {
  delay(10);
  Serial.print("Menghubungkan ke WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi terkoneksi");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Menghubungkan MQTT...");
    if (client.connect("ESP32Client")) {
      Serial.println("terhubung");
    } else {
      Serial.print("gagal, rc=");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

// ------------------- Setup & Loop --------------------
void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  float suhu = dht.readTemperature();
  float kelembapan = dht.readHumidity();

  int soilValue = analogRead(SOIL_PIN);
  float soilMoisture = map(soilValue, 4095, 0, 0, 100);

  float durasiPenyiraman = inferensi(suhu, soilMoisture);
  durasiPenyiraman = constrain(durasiPenyiraman, 0, 5);

  Serial.print("Suhu: "); Serial.print(suhu);
  Serial.print(" °C, Kelembapan Tanah: "); Serial.print(soilMoisture);
  Serial.print(" %, Durasi: "); Serial.print(durasiPenyiraman);
  Serial.println(" detik");

  if (durasiPenyiraman > 0) {
    digitalWrite(RELAY_PIN, HIGH);
    delay(durasiPenyiraman * 1000);
    digitalWrite(RELAY_PIN, LOW);
  }

  // Kirim ke MQTT dalam format JSON
  String payload = "{";
  payload += "\"suhu\": " + String(suhu, 1) + ",";
  payload += "\"kelembapan_tanah\": " + String(soilMoisture, 1) + ",";
  payload += "\"durasi\": " + String(durasiPenyiraman, 1);
  payload += "}";

  client.publish(mqtt_topic, payload.c_str());

  delay(10000); // Delay 10 detik
}
