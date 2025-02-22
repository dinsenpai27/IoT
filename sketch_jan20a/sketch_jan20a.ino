#include <DHT.h>

// Konfigurasi DHT Sensor
#define DHTPIN 2       // Pin digital untuk DHT
#define DHTTYPE DHT11  // Pilihan: DHT11, DHT22, atau DHT21
DHT dht(DHTPIN, DHTTYPE);

// Konfigurasi Soil Moisture Sensor
#define MOISTURE_PIN A0  // Pin analog untuk kelembapan tanah
#define RELAY_PIN 3      // Pin digital untuk relay (diubah ke pin 3)

// Fungsi untuk menentukan status pompa berdasarkan suhu dan kelembapan tanah
void kontrolPompa(float suhu, int kelembapanTanah) {
  // Tentukan status pompa berdasarkan suhu dan kelembapan tanah
  if (kelembapanTanah <= 20) { // Jika kelembapan tanah rendah (lebih kering)
    if (suhu >= 15 && suhu < 35) {  // Suhu yang lebih tinggi memicu penyiraman
      digitalWrite(RELAY_PIN, HIGH);  // Pompa aktif (tanah kering)
    } else {
      digitalWrite(RELAY_PIN, LOW);   // Pompa mati (tanah lembab)
    }
  } else {
    digitalWrite(RELAY_PIN, LOW);   // Pompa mati (tanah lembab)
  }
}

void setup() {
  // Inisialisasi Serial Monitor
  Serial.begin(9600);
  Serial.println("Soil Moisture Sensor and Relay Control");

  // Mulai sensor DHT
  dht.begin();

  // Atur pin relay sebagai output
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // Matikan relay di awal (relay aktif rendah)
}

void loop() {
  // Membaca suhu udara dari DHT
  float temperature = dht.readTemperature();

  // Membaca nilai kelembapan tanah
  int soilValue = analogRead(MOISTURE_PIN);
  int soilMoisturePercent = map(soilValue, 0, 1023, 0, 100);

  // Menyiapkan format data untuk backend
  if (!isnan(temperature)) {
    // Kirimkan suhu, kelembapan tanah, dan status pompa
    Serial.print("Suhu: ");
    Serial.print(temperature);  // Suhu dalam format float
    Serial.print(" °C, ");
  } else {
    Serial.print("Error dalam pembacaan suhu, ");
  }

  // Tampilkan data kelembapan tanah
  Serial.print("Kelembapan Tanah: ");
  Serial.print(soilMoisturePercent);  // Kelembapan tanah dalam persen
  Serial.print("%, ");

  // Tentukan status pompa dan kirimkan status pompa
  if (soilMoisturePercent <= 20) {
    Serial.println("Status Pompa: Aktif");  // Status pompa aktif
  } else {
    Serial.println("Status Pompa: Mati");  // Status pompa mati
  }

  // Kontrol pompa berdasarkan suhu dan kelembapan tanah
  kontrolPompa(temperature, soilMoisturePercent);

  // Tunggu 2 detik sebelum pembacaan berikutnya
  delay(2000);
}
