#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Подключение дисплея
#define OLED_RESET     4
Adafruit_SSD1306 display(OLED_RESET);

// Подключение SD-карты
const int chipSelect = 53;

// Инициализация камеры
#define CAMERA_CS 5
#define CAMERA_RESET -1
#define CAMERA_SID 4
#define CAMERA_SCK 18
Camera cam = Camera(CAMERA_CS, CAMERA_RESET, CAMERA_SID, CAMERA_SCK);

// Загрузка эталонного кадра (с чистой камеры)
const char* referenceImagePath = "/reference_image.jpg";
uint8_t referenceImage[50000]; // Буфер для эталонного изображения

// Функция для определения степени загрязнения
int getPollutionLevel(const uint8_t* currentImage, const uint8_t* referenceImage, int width, int height, uint8_t* pollutionMask) {
  int pollutionPixels = 0;
  for (int i = 0; i < width * height; i++) {
    // Вычисление абсолютного различия между пикселями
    int diff = abs(currentImage[i] - referenceImage[i]);

    // Пороговое значение
    if (diff > 20) {
      pollutionPixels++;
      // Выделение пикселя загрязнения в маске
      pollutionMask[i] = 255; 
    }
  }

  // Вычисление уровня загрязнения
  int pollutionLevel = (pollutionPixels * 100) / (width * height);
  return pollutionLevel;
}

void setup() {
  Serial.begin(9600);
  
  // Инициализация дисплея
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  // Инициализация SD-карты
  if (!SD.begin(chipSelect)) {
    Serial.println(F("card failed, or not present"));
    return;
  }

  // Инициализация камеры
  cam.begin();

  // Загрузка эталонного изображения
  File referenceFile = SD.open(referenceImagePath, FILE_READ);
  if (!referenceFile) {
    Serial.println("Ошибка открытия эталонного изображения");
    return;
  }
  referenceFile.read(referenceImage, sizeof(referenceImage));
  referenceFile.close();
}

void loop() {
  // Захват кадра
  cam.readFrame();

  // Создание маски для выделения областей загрязнения
  uint8_t pollutionMask[cam.getWidth() * cam.getHeight()];
  memset(pollutionMask, 0, sizeof(pollutionMask));

  // Определение степени загрязнения
  int pollutionLevel = getPollutionLevel(cam.getImage(), referenceImage, cam.getWidth(), cam.getHeight(), pollutionMask);

  // Отображение результата на дисплей
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print("Уровень загрязнения: ");
  display.println(pollutionLevel);
  display.display();

  // Вывод результата в последовательный порт
  Serial.print("Уровень загрязнения: ");
  Serial.println(pollutionLevel);

  // Отображение текущего кадра с областями загрязнения
  display.clearDisplay();
  display.drawBitmap(0, 0, cam.getImage(), cam.getWidth(), cam.getHeight(), WHITE);
  for (int i = 0; i < cam.getWidth() * cam.getHeight(); i++) {
    if (pollutionMask[i] == 255) {
      display.drawPixel(i % cam.getWidth(), i / cam.getWidth(), BLACK);
    }
  }
  display.display();

  delay(500);
}
