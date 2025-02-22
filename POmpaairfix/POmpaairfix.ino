#include <DHT.h>

// Pin untuk sensor DHT
#define DHT_PIN 2
#define DHT_TYPE DHT11  // Bisa juga menggunakan DHT11
#define RELAY_PIN 3     // Pin untuk relay
#define MOISTURE_PIN A0  // Pin untuk sensor kelembapan tanah (analog pin A0)

DHT dht(DHT_PIN, DHT_TYPE);

// Nilai keanggotaan fuzzy untuk suhu
float suhuKeanggotaan[4];  // Dingin, Agak Dingin, Normal, Panas

// Nilai keanggotaan fuzzy untuk kelembapan tanah
float kelembapanTanahKeanggotaan[3];  // Kering, Lembab, Basah

// Durasi untuk aturan (0 detik atau 5 detik)
float durasi[3] = {0, 5, 0};  // Durasi (0, 5, 0)

// Fuzzyfikasi untuk suhu
void fuzzyfikasiSuhu(float suhu) {
  // Dingin: 15-23
  suhuKeanggotaan[0] = max(0.0, min(1.0, (23 - suhu) / 8.0));
  
  // Agak Dingin: 19-27
  suhuKeanggotaan[1] = max(0.0, min(1.0, min((suhu - 19) / 8.0, (27 - suhu) / 8.0)));
  
  // Normal: 23-31
  suhuKeanggotaan[2] = max(0.0, min(1.0, min((suhu - 23) / 8.0, (31 - suhu) / 8.0)));
  
  // Panas: 27-35
  suhuKeanggotaan[3] = max(0.0, min(1.0, (suhu - 27) / 8.0));
}

// Fuzzyfikasi untuk kelembapan tanah
void fuzzyfikasiKelembapanTanah(float kelembapan) {
  // Kering: 0-40
  kelembapanTanahKeanggotaan[0] = max(0.0, min(1.0, (40 - kelembapan) / 40.0));
  
  // Lembab: 27-75
  kelembapanTanahKeanggotaan[1] = max(0.0, min(1.0, min((kelembapan - 27) / 48.0, (75 - kelembapan) / 48.0)));
  
  // Basah: 60-100
  kelembapanTanahKeanggotaan[2] = max(0.0, min(1.0, (kelembapan - 60) / 40.0));
}

// Evaluasi dan defuzzifikasi
float evaluasiDanDefuzzyfikasi() {
  // Keanggotaan untuk durasi penyiraman
  float keanggotaanDurasi[3] = {0.0, 0.0, 0.0};

  // Aturan 1: Dingin dan Kering => Nyala (on) untuk 5 detik
  keanggotaanDurasi[1] = min(suhuKeanggotaan[0], kelembapanTanahKeanggotaan[0]);

  // Aturan 2: Dingin dan Lembab => Nyala (on) untuk 5 detik
  keanggotaanDurasi[1] = max(keanggotaanDurasi[1], min(suhuKeanggotaan[0], kelembapanTanahKeanggotaan[1]));

  // Aturan 3: Dingin dan Basah => Mati (off) untuk 0 detik
  keanggotaanDurasi[0] = max(keanggotaanDurasi[0], min(suhuKeanggotaan[0], kelembapanTanahKeanggotaan[2]));

  // Aturan 4: Agak Dingin dan Kering => Nyala (on) untuk 5 detik
  keanggotaanDurasi[1] = max(keanggotaanDurasi[1], min(suhuKeanggotaan[1], kelembapanTanahKeanggotaan[0]));

  // Aturan 5: Agak Dingin dan Lembab => Nyala (on) untuk 5 detik
  keanggotaanDurasi[1] = max(keanggotaanDurasi[1], min(suhuKeanggotaan[1], kelembapanTanahKeanggotaan[1]));

  // Aturan 6: Agak Dingin dan Basah => Mati (off) untuk 0 detik
  keanggotaanDurasi[0] = max(keanggotaanDurasi[0], min(suhuKeanggotaan[1], kelembapanTanahKeanggotaan[2]));

  // Aturan 7: Normal dan Kering => Nyala (on) untuk 5 detik
  keanggotaanDurasi[1] = max(keanggotaanDurasi[1], min(suhuKeanggotaan[2], kelembapanTanahKeanggotaan[0]));

  // Aturan 8: Normal dan Lembab => Nyala (on) untuk 5 detik
  keanggotaanDurasi[1] = max(keanggotaanDurasi[1], min(suhuKeanggotaan[2], kelembapanTanahKeanggotaan[1]));

  // Aturan 9: Normal dan Basah => Mati (off) untuk 0 detik
  keanggotaanDurasi[0] = max(keanggotaanDurasi[0], min(suhuKeanggotaan[2], kelembapanTanahKeanggotaan[2]));

  // Aturan 10: Panas dan Kering => Nyala (on) untuk 5 detik
  keanggotaanDurasi[1] = max(keanggotaanDurasi[1], min(suhuKeanggotaan[3], kelembapanTanahKeanggotaan[0]));

  // Aturan 11: Panas dan Lembab => Nyala (on) untuk 5 detik
  keanggotaanDurasi[1] = max(keanggotaanDurasi[1], min(suhuKeanggotaan[3], kelembapanTanahKeanggotaan[1]));

  // Aturan 12: Panas dan Basah => Mati (off) untuk 0 detik
  keanggotaanDurasi[0] = max(keanggotaanDurasi[0], min(suhuKeanggotaan[3], kelembapanTanahKeanggotaan[2]));

  // Defuzzifikasi: Rata-rata tertimbang (centroid)
  float num = 0.0;  // Pembilang
  float den = 0.0;  // Penyebut

  for (int i = 0; i < 3; i++) {
    num += keanggotaanDurasi[i] * durasi[i];
    den += keanggotaanDurasi[i];
  }

  if (den != 0) {
    return num / den;  // Menghitung durasi penyiraman akhir
  } else {
    return 0.0;  // Jika tidak ada keanggotaan, kembali 0
  }
}

void setup() {
  // Inisialisasi Serial Monitor
  Serial.begin(9600);
  dht.begin();

  // Set relay pin sebagai output
  pinMode(RELAY_PIN, OUTPUT);
}

void loop() {
  // Membaca suhu dan kelembapan dari sensor DHT
  float suhu = dht.readTemperature();  // Suhu dalam Celsius

  // Membaca kelembapan tanah dari A0 (pin analog)
  int moistureValue = analogRead(MOISTURE_PIN);
  float kelembapanTanah = map(moistureValue, 1023, 0, 0, 100);  // Pemetaan ke skala 0-100
  
  // Cek jika pembacaan gagal
  if (isnan(suhu)) {
    Serial.println("Gagal membaca sensor suhu!");
    return;
  }

  // Melakukan fuzzyfikasi untuk suhu dan kelembapan tanah
  fuzzyfikasiSuhu(suhu);
  fuzzyfikasiKelembapanTanah(kelembapanTanah);

  // Evaluasi aturan dan defuzzifikasi untuk mendapatkan durasi penyiraman
  float durasiPenyiraman = evaluasiDanDefuzzyfikasi();

  // Menampilkan hasil di Serial Monitor
  Serial.print("Suhu: ");
  Serial.print(suhu);
  Serial.print(" °C, ");
  Serial.print("Kelembapan Tanah: ");
  Serial.print(kelembapanTanah);
  Serial.print(" %, ");
  Serial.print("Durasi Penyiraman: ");
  Serial.print(durasiPenyiraman);
  Serial.println(" detik");

  // Mengontrol relay berdasarkan durasi penyiraman yang dihitung
  if (durasiPenyiraman > 0) {
    digitalWrite(RELAY_PIN, HIGH);  // Nyalakan relay (penyiraman hidup)
    delay(durasiPenyiraman * 1000);  // Tunggu durasi dalam milidetik
    digitalWrite(RELAY_PIN, LOW);   // Matikan relay (penyiraman mati)
  } else {
    digitalWrite(RELAY_PIN, LOW);   // Matikan relay jika tidak ada penyiraman
  }

  // Tunggu 2 detik sebelum pembacaan berikutnya
  delay(10000);
}
