// WIKA A-10 0-0.25 bar 4-20mA Basınç Sensörü - Direnç 247.2Ω ve Filtreli
// Nextion: j0.val (0-100), z0.val (0-180)

const int sensorPin = A0;           
const float referenceVoltage = 5.0; 
const float resistorValue = 247.2;  // Hassas ölçülmüş direnç değeri (Ω)

// Sensörün ölçüm aralığı
const float minPressureBar = 0.0;   
const float maxPressureBar = 0.25;  

// 4-20mA sinyal aralığı
const float minCurrent = 4.0;       
const float maxCurrent = 20.0;      

const float BAR_TO_PSI = 14.5038;
const float SCALE_FACTOR = 100.0;

// --- Dijital Filtre Ayarları ---
const int numReadings = 20;  // Kaç okuma ortalamaya alınacak (10-20 arası iyidir)
float readings[numReadings]; // Okumaları saklamak için dizi
int readIndex = 0;           // Dizi indeksi
float total = 0;             // Toplam değer
float average = 0;           // Ortalama değer

void setup() {
  Serial.begin(9600);
  
  // Filtre dizisini sıfırla
  for (int i = 0; i < numReadings; i++) {
    readings[i] = 0;
  }
}

void loop() {
  // A0 pininden ham ADC değerini oku
  int adcValue = analogRead(sensorPin);
  
  // --- Hareketli Ortalama Filtresi ---
  total = total - readings[readIndex];        // En eski değeri toplamdan çıkar
  readings[readIndex] = adcValue;             // Yeni değeri diziye ekle
  total = total + readings[readIndex];        // Yeni değeri toplama ekle
  readIndex = (readIndex + 1) % numReadings;  // İndeksi ilerlet (döngüsel)
  
  average = total / numReadings;              // Ortalamayı hesapla
  
  // --- Filtrelenmiş değer ile hesaplamalar ---
  float voltage = (average / 1024.0) * referenceVoltage * 1000; // mV
  
  float current = voltage / resistorValue; // mA
  
  // 4-20mA aralığına göre basınç hesapla
  float pressureBar = (current - minCurrent) / (maxCurrent - minCurrent) * (maxPressureBar - minPressureBar) + minPressureBar;
  
  // 4mA'den düşük değerleri 0 olarak sınırla (sensör boşta veya bağlantı yoksa)
  if (current < minCurrent) {
    pressureBar = 0;
  }
  
  // Basıncı sensör aralığına sınırla (0.0 - 0.25 bar)
  if (pressureBar < minPressureBar) pressureBar = minPressureBar;
  if (pressureBar > maxPressureBar) pressureBar = maxPressureBar;
  
  float pressurePsi = pressureBar * BAR_TO_PSI;
  float pressureCbar = pressureBar * SCALE_FACTOR;
  float pressureCpsi = pressurePsi * SCALE_FACTOR;
  
  // --- Nextion için 0-100 ve 0-180 ölçekleme ---
  int jValue = (int)((pressureBar / maxPressureBar) * 100.0);  // j0: 0-100
  int zValue = (int)((pressureBar / maxPressureBar) * 180.0);  // z0: 0-180
  
  // Sınırları garanti altına al
  if (jValue < 0) jValue = 0;
  if (jValue > 100) jValue = 100;
  if (zValue < 0) zValue = 0;
  if (zValue > 180) zValue = 180;
  
  // --- Seri Ekrana Yazdır (isteğe bağlı, yorumdan çıkarabilirsiniz) ---
  //Serial.print("ADC: ");
  //Serial.print(adcValue);
  //Serial.print(" | Filtre: ");
  //Serial.print(average, 0);
  //Serial.print(" | Gerilim: ");
  //Serial.print(voltage, 1);
  //Serial.print(" mV");
  //Serial.print(" | Akım: ");
  //Serial.print(current, 3);
  //Serial.print(" mA");
  //Serial.print(" | Basınç: ");
  //Serial.print(pressureCbar, 2);
  //Serial.print(" cbar");
  //Serial.print(" | ");
  //Serial.print(pressureCpsi, 2);
  //Serial.println(" cPSI");

  // --- Nextion Komutları ---
  String command1 = "t0.txt=\"" + String(average, 0) + "\"";
  Serial.print(command1);
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff); 

  String command2 = "t1.txt=\"" + String(voltage, 1) + "\"";
  Serial.print(command2);
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);

  String command3 = "t2.txt=\"" + String(current, 3) + "\"";
  Serial.print(command3);
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);

  String command4 = "t3.txt=\"" + String(pressureCbar, 2) + "\"";
  Serial.print(command4);
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);

  String command5 = "t4.txt=\"" + String(pressureCpsi, 2) + "\"";
  Serial.print(command5);
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);

  // j0 bileşenine 0-100 arası değer gönder
  String command6 = "j0.val=" + String(jValue);
  Serial.print(command6);
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);

  // z0 bileşenine 0-180 arası değer gönder
  String command7 = "z0.val=" + String(zValue);
  Serial.print(command7);
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  
  delay(500); // 500ms bekle (filtre zaten yumuşatıyor)
}
