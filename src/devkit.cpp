#include <SPI.h>
#include <SD.h>

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("\n\n=== SD CARD PIN SCANNER ===");

  // Вариант 1: Стандартный VSPI (как у нас было)
  Serial.println("1. SCK=18, MISO=19, MOSI=23, CS=5");
  SPI.begin(18, 19, 23, 5);
  if (SD.begin(5, SPI, 4000000)) { Serial.println("✅ OK! Пины VSPI работают"); return; }
  SD.end(); SPI.end(); delay(500);

  // Вариант 2: Альтернативный HSPI
  Serial.println("2. SCK=14, MISO=12, MOSI=13, CS=15");
  SPI.begin(14, 12, 13, 15);
  if (SD.begin(15, SPI, 4000000)) { Serial.println("✅ OK! Пины HSPI работают (CS=15)"); return; }
  SD.end(); SPI.end(); delay(500);

  // Вариант 3: Еще один популярный набор
  Serial.println("3. SCK=18, MISO=19, MOSI=23, CS=4");
  SPI.begin(18, 19, 23, 4);
  if (SD.begin(4, SPI, 4000000)) { Serial.println("✅ OK! CS=4 работает"); return; }
  SD.end(); SPI.end(); delay(500);

  // Вариант 4: Кастомный, часто используется на модулях
  Serial.println("4. SCK=18, MISO=19, MOSI=23, CS=15");
  SPI.begin(18, 19, 23, 15);
  if (SD.begin(15, SPI, 4000000)) { Serial.println("✅ OK! CS=15 работает"); return; }
  SD.end(); SPI.end(); delay(500);

  // Вариант 5: Медленная скорость на стандартных пинах
  Serial.println("5. SCK=18, MISO=19, MOSI=23, CS=5 (SPEED 1MHz)");
  SPI.begin(18, 19, 23, 5);
  if (SD.begin(5, SPI, 1000000)) { Serial.println("✅ OK! Скорость 1МГц помогла"); return; }
  SD.end(); SPI.end(); delay(500);

  Serial.println("❌ НИ ОДИН ВАРИАНТ НЕ СРАБОТАЛ.");
  Serial.println("Возможные причины:");
  Serial.println("- Модуль SD сгорел");
  Serial.println("- Провода отвалились");
  Serial.println("- Нужен внешний источник питания 3.3В (модуль не тянет от платы)");
  Serial.println("- SD-карта не в формате FAT32");
}

void loop() { delay(1000); }
