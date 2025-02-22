const int RELAY_PIN = 3;  // Pin relay untuk mengontrol pompa
const int DHT_PIN = 2;    // Pin DHT11 untuk sensor suhu dan kelembapan
const int SOIL_PIN = A0;  // Pin sensor kelembapan tanah

// Fungsi untuk membaca nilai suhu dan kelembapan dari sensor DHT11
#include <DHT.h>
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);

// Fungsi fuzzyfikasi suhu
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
// Fungsi fuzzyfikasi kelembapan tanah
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

// Fungsi inferensi dan defuzzifikasi
float inferensi(float suhu, float kelembapan) {
  float md[12]; // Min derajat keanggotaan
  float dr[12] = {5, 5, 0, 5, 5, 0, 5, 5, 0, 5, 5, 0}; // Durasi per aturan
 
  // Hitung fuzzy suhu dan kelembapan
  float fs[4] = {fsDingin(suhu), fsAgakDingin(suhu), fsNormal(suhu), fsPanas(suhu)};
  float fk[3] = {fkKering(kelembapan), fkLembab(kelembapan), fkBasah(kelembapan)};

  // Aturan fuzzy (12 aturan)
  md[0] = min(fs[0], fk[0]); // Dingin, Kering
  md[1] = min(fs[0], fk[1]); // Dingin, Lembab
  md[2] = min(fs[0], fk[2]); // Dingin, Basah
  md[3] = min(fs[1], fk[0]); // Agak Dingin, Kering
  md[4] = min(fs[1], fk[1]); // Agak Dingin, Lembab
  md[5] = min(fs[1], fk[2]); // Agak Dingin, Basah
  md[6] = min(fs[2], fk[0]); // Normal, Kering
  md[7] = min(fs[2], fk[1]); // Normal, Lembab
  md[8] = min(fs[2], fk[2]); // Normal, Basah
  md[9] = min(fs[3], fk[0]); // Panas, Kering
  md[10] = min(fs[3], fk[1]); // Panas, Lembab
  md[11] = min(fs[3], fk[2]); // Panas, Basah
 
  // Defuzzifikasi
  float totalWeighted = 0;
  float totalInference = 0;

  for (int i = 0; i < 12; i++) {
    totalWeighted += md[i] * dr[i]; // Hasil inferensi * durasi
    totalInference += md[i];       // Total inferensi
  }

  if (totalInference == 0) {
    return 0; // Hindari pembagian oleh nol
  }

  return totalWeighted / totalInference; // Durasi penyiraman
}

void setup() {
  Serial.begin(9600); 
  dht.begin();
  pinMode(RELAY_PIN, OUTPUT);  // Set relay pin sebagai output
}

void loop() {
  // Baca suhu dan kelembapan dari sensor DHT
  float suhu = dht.readTemperature();
  float kelembapan = dht.readHumidity();
  
  // Baca kelembapan tanah
  int soilValue = analogRead(SOIL_PIN);
  float soilMoisture = map(soilValue, 1023, 0 , 0, 100);  // Konversi ke persen (0-100%)
  
  // Hitung durasi penyiraman berdasarkan inferensi
  float durasiPenyiraman = inferensi(suhu, soilMoisture);
  
  // Batasi durasi penyiraman antara 0 hingga 5 detik
  if (durasiPenyiraman > 5) {
    durasiPenyiraman = 5;
  } else if (durasiPenyiraman < 0) {
    durasiPenyiraman = 0;
  }
 ;
  // Menampilkan data pada serial monitor
  Serial.print("Suhu: ");
  Serial.print(suhu);
  Serial.print("°C, Kelembapan Tanah: ");
  Serial.print(soilMoisture);
  Serial.print("%, Durasi Penyiraman: ");
  Serial.print(durasiPenyiraman);
  Serial.println(" detik");
  
  // Mengontrol relay berdasarkan durasi penyiraman 
  if (durasiPenyiraman > 0) {
    digitalWrite(RELAY_PIN, HIGH);  // Nyalakan relay (penyiraman hidup)
    delay(durasiPenyiraman * 1000);  // Tunggu durasi dalam milidetik
    digitalWrite(RELAY_PIN, LOW);   // Matikan relay (penyiraman mati)
  } else {
    digitalWrite(RELAY_PIN, LOW);   // Matikan relay jika tidak ada penyiraman
  }

  // Tunggu 10 detik sebelum pembacaan berikutnya
  delay(10000);
}
