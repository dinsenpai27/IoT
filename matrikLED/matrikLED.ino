#include <Servo.h> // Tambahkan pustaka Servo

// 2-dimensional array of row pin numbers:
int R[] = {2, 7, A5, 5, 13, A4, 12, A2}; 
// 2-dimensional array of column pin numbers:
int C[] = {6, 11, 10, 3, A3, 4, 8, 9};   

Servo myServo; // Objek servo

unsigned char biglove[8][8] =     // The big “heart”  
{ 
  0,0,0,0,0,0,0,0,
  0,0,1,0,0,1,0,0,
  0,0,1,0,0,1,0,0,
  0,0,1,0,0,1,0,0,
  0,0,0,0,0,0,0,0,
  0,1,0,0,0,0,1,0,
  0,0,1,1,1,1,0,0,
  0,0,0,0,0,0,0,0,
}; 
 
unsigned char smalllove[8][8] =   // The small “heart”
{ 
  0,0,0,0,0,0,0,0, 
  0,0,0,0,0,0,0,0, 
  1,1,1,0,0,1,1,1, 
  0,0,0,0,0,0,0,0, 
  0,0,0,0,0,0,0,0, 
  0,0,1,1,1,1,0,0, 
  0,0,0,0,0,0,0,0, 
  0,0,0,0,0,0,0,0,
}; 
 
void setup() 
{ 
  // Inisialisasi pin baris dan kolom sebagai output:
  for (int i = 0; i < 8; i++) 
  { 
    pinMode(R[i], OUTPUT); 
    pinMode(C[i], OUTPUT); 
  } 

  // Inisialisasi servo:
  myServo.attach(A0); // Servo terhubung ke pin A0
  myServo.write(90);  // Posisi awal servo (90 derajat)
} 
 
void loop() 
{ 
  for (int i = 0; i < 100; i++)        // Loop display 100 times
  { 
    Display(biglove);                  // Display the "Big Heart"
    myServo.write(45);                 // Pindahkan servo ke 45 derajat
                             // Tunggu sejenak
  } 

  for (int i = 0; i < 50; i++)         // Loop display 50 times
  {    
    Display(smalllove);                // Display the "Small Heart"
    myServo.write(135);                // Pindahkan servo ke 135 derajat
                             // Tunggu sejenak
  } 
} 
 
void Display(unsigned char dat[8][8])   
{ 
  for (int c = 0; c < 8; c++) 
  { 
    digitalWrite(C[c], LOW); // Gunakan kolom
    // Loop untuk baris
    for (int r = 0; r < 8; r++) 
    { 
      digitalWrite(R[r], dat[r][c]); 
    } 
    delay(1); 
    Clear();  // Hapus cahaya tampilan kosong
  } 
} 
 
void Clear()                         
{ 
  for (int i = 0; i < 8; i++) 
  { 
    digitalWrite(R[i], LOW); 
    digitalWrite(C[i], HIGH); 
  } 
}
