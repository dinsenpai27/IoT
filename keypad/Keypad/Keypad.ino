#include <Keypad.h>
#include <math.h>

// Definisi keypad
const byte ROWS = 4;
const byte COLS = 4;
byte rowPins[ROWS] = {5, 4, 3, 2};
byte colPins[COLS] = {6, 7, 8, 9};

char Keys[ROWS][COLS] = {
  {'1','2','3','4'},
  {'5','6','7','8'},
  {'9','0','+','-'},
  {'=','E','F','G'}  // F=^, G=√
};

Keypad customKeypad = Keypad( makeKeymap(Keys), rowPins, colPins, ROWS, COLS );

// Definisi pin untuk tombol kurung buka dan tutup
const int tombolKurungBuka = 10;
const int tombolKurungTutup = 11;

// Variabel kalkulator
String input = "";
float angka1 = 0;
float angka2 = 0;
char op = 0;
bool sedangInputAngka1 = true;
bool akarMode = false;

// Variabel untuk debounce tombol
bool prevKurungBuka = HIGH;
bool prevKurungTutup = HIGH;

void setup() {
  Serial.begin(9600);
  Serial.println("Kalkulator Siap!");
  
  pinMode(tombolKurungBuka, INPUT_PULLUP);  // Tombol kurung buka
  pinMode(tombolKurungTutup, INPUT_PULLUP); // Tombol kurung tutup
}

void loop() {
  // Membaca input dari keypad
  char customKey = customKeypad.getKey();

  // Tombol ( dan )
  bool tombolBuka = digitalRead(tombolKurungBuka);
  if (tombolBuka == LOW && prevKurungBuka == HIGH) {
    input += '(';
    Serial.println("( ditambahkan");
    delay(200);  // debounce delay
  }
  prevKurungBuka = tombolBuka;

  bool tombolTutup = digitalRead(tombolKurungTutup);
  if (tombolTutup == LOW && prevKurungTutup == HIGH) {
    input += ')';
    Serial.println(") ditambahkan");
    delay(200);  // debounce delay
  }
  prevKurungTutup = tombolTutup;

  // Logika keypad untuk angka dan operator
  if (customKey) {
    Serial.print("Tombol ditekan: ");
    Serial.println(customKey);

    if (customKey >= '0' && customKey <= '9') {
      input += customKey;
      Serial.print("Input: ");
      Serial.println(input);
    }
    else if (customKey == '+' || customKey == '-' || customKey == '*' || customKey == '/' || customKey == 'F') {
      if (input != "") {
        angka1 = input.toFloat();
        input = "";
      }
      op = customKey;
      sedangInputAngka1 = false;
      akarMode = false;

      Serial.print("Operator: ");
      if (op == 'F') Serial.println("^");
      else Serial.println(op);
    }
    else if (customKey == 'G') {
      // Aktifkan mode akar
      akarMode = true;
      input = ""; // kosongkan input agar angka bisa diinput setelah tekan G
      Serial.println("Mode Akar Kuadrat Aktif");
    }
    else if (customKey == '=') {
      float hasil = 0;

      if (akarMode) {
        // Jika mode akar aktif
        if (input != "") {
          angka1 = input.toFloat();
        }
        if (angka1 >= 0) {
          hasil = sqrt(angka1);
          Serial.print("√");
          Serial.print(angka1);
          Serial.print(" = ");
          Serial.println(hasil);
        } else {
          Serial.println("Error: Akar negatif!");
        }
      }
      else if (op != 0 && input != "") {
        angka2 = input.toFloat();

        switch (op) {
          case '+': hasil = angka1 + angka2; break;
          case '-': hasil = angka1 - angka2; break;
          case '*': hasil = angka1 * angka2; break;
          case '/':
            if (angka2 != 0) hasil = angka1 / angka2;
            else {
              Serial.println("Error: Bagi 0!");
              resetKalkulator();
              return;
            }
            break;
          case 'F': hasil = pow(angka1, angka2); break;
        }

        Serial.print("Hasil: ");
        Serial.println(hasil);
      }
      else {
        Serial.println("Belum ada input yang cukup!");
      }

      resetKalkulator();
    }
    else if (customKey == 'E') {
      resetKalkulator();
      Serial.println("Reset Kalkulator");
    }
  }
}

void resetKalkulator() {
  input = "";
  angka1 = 0;
  angka2 = 0;
  op = 0;
  sedangInputAngka1 = true;
  akarMode = false;
}
