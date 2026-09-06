
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <Adafruit_ILI9341.h>
#include <Arduino.h>
#include <Adafruit_FT6206.h>
#include <SD.h>
#include <Wire.h>

#define TFT_DC 2
#define TFT_CS 15
#define SD_CS 5

#define SPI_SCK 12
#define SPI_MISO 13
#define SPI_MOSI 11

#define I2C_SDA 10
#define I2C_SCL 8

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

Adafruit_FT6206 ctp = Adafruit_FT6206();

#define BG_COLOR ILI9341_BLACK
#define HEADER_COLOR ILI9341_BLUE
#define TEXT_COLOR ILI9341_WHITE
#define BUTTON_COLOR ILI9341_DARKGREY
#define BORDER_COLOR ILI9341_CYAN
#define GREEN_COLOR ILI9341_GREEN
#define RED_COLOR ILI9341_RED
#define YELLOW_COLOR ILI9341_YELLOW

enum Screen {
  HOME,
  FUEL_ODO,
  FUEL_LITERS,
  FUEL_PRICE,
  FUEL_RESULT,
  HISTORY,
  STATS
};

Screen currentScreen = HOME;

String odoInput = "";
String litersInput = "";
String priceInput = "";

float currentOdo = 0;
float currentLiters = 0;
float currentPrice = 0;

float previousOdo = 0;
float distance = 0;
float fuelEconomy = 0;
float totalCost = 0;

float totalFuel = 0;
float totalSpending = 0;
float totalDistance = 0;
float averageEconomy = 0;

int fuelEntries = 0;
float lastOdo = 0;

void handleTouch(int x, int y);
void handleNumberInput(int x, int y, String &input);

void drawHomeScreen();
void drawFuelOdoScreen();
void drawFuelLitersScreen();
void drawFuelPriceScreen();
void drawFuelResultScreen();
void drawHistoryScreen();
void drawStatsScreen();

void drawHeader(const char *title);

void drawButton(
  int x,
  int y,
  int w,
  int h,
  const char *label
);

void drawSmallButton(
  int x,
  int y,
  int w,
  int h,
  const char *label
);

void drawNumberPad();

void drawKey(
  int x,
  int y,
  int w,
  int h,
  const char *label
);

void calculateFuel();
float getLastOdometer();
void saveFuelRecord();
void calculateStatistics();

void setup() {
  Serial.begin(115200);

  delay(500);

  Serial.println();
  Serial.println("FUEL & MAINTENANCE CYBERDECK");

  Wire.setPins(I2C_SDA, I2C_SCL);

  Serial.println("I2C pins configured");

  SPI.begin(
    SPI_SCK,
    SPI_MISO,
    SPI_MOSI
  );

  pinMode(TFT_CS, OUTPUT);
  pinMode(SD_CS, OUTPUT);

  digitalWrite(TFT_CS, HIGH);
  digitalWrite(SD_CS, HIGH);

  Serial.println("SPI configured");

  Serial.println("Starting ILI9341...");

  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(BG_COLOR);

  Serial.println("Display OK");

  Serial.println("Starting FT6206...");

  if (!ctp.begin(40)) {
    Serial.println("Touchscreen failed!");

    while (1) {
      delay(100);
    }
  }

  Serial.println("Touchscreen OK");

  Serial.println("Starting SD card...");

  if (!SD.begin(SD_CS, SPI)) {
    Serial.println("SD Card failed!");
  } else {
    Serial.println("SD Card OK");

    uint8_t cardType = SD.cardType();

    if (cardType == CARD_NONE) {
      Serial.println("No SD card detected!");
    } else {
      Serial.println("SD card detected!");

      Serial.print("SD Card Size: ");

      uint64_t cardSize =
        SD.cardSize() / (1024 * 1024);

      Serial.print(cardSize);
      Serial.println(" MB");
    }
  }

  drawHomeScreen();

  Serial.println("System ready.");
}

void loop() {
  if (ctp.touched()) {
    TS_Point p = ctp.getPoint();

    int x = map(
      p.y,
      0,
      320,
      0,
      320
    );

    int y = map(
      p.x,
      0,
      240,
      240,
      0
    );

    Serial.print("Touch: ");
    Serial.print(x);
    Serial.print(", ");
    Serial.println(y);

    handleTouch(x, y);

    delay(250);
  }
}

void drawHomeScreen() {
  currentScreen = HOME;

  tft.fillScreen(BG_COLOR);

  tft.fillRect(
    0,
    0,
    320,
    40,
    HEADER_COLOR
  );

  tft.setTextColor(TEXT_COLOR);
  tft.setTextSize(2);
  tft.setCursor(10, 12);
  tft.println("FUEL & MAINT");

  tft.setTextSize(1);
  tft.setCursor(250, 15);
  tft.println("ONLINE");

  drawButton(
    20,
    60,
    130,
    55,
    "FUEL LOG"
  );

  drawButton(
    170,
    60,
    130,
    55,
    "MAINT"
  );

  drawButton(
    20,
    130,
    130,
    55,
    "STATS"
  );

  drawButton(
    170,
    130,
    130,
    55,
    "HISTORY"
  );

  tft.setTextColor(GREEN_COLOR);
  tft.setTextSize(1);

  tft.setCursor(20, 215);
  tft.println("SYSTEM READY");

  tft.setCursor(220, 215);
  tft.println("SD: READY");
}

void drawFuelOdoScreen() {
  currentScreen = FUEL_ODO;

  tft.fillScreen(BG_COLOR);

  drawHeader("FUEL LOG");

  tft.setTextColor(TEXT_COLOR);
  tft.setTextSize(2);

  tft.setCursor(20, 55);
  tft.println("ENTER ODOMETER");

  tft.setTextSize(1);

  tft.setCursor(20, 82);
  tft.println("CURRENT VEHICLE KM");

  tft.drawRect(
    20,
    100,
    280,
    40,
    BORDER_COLOR
  );

  tft.setTextSize(2);
  tft.setCursor(30, 112);
  tft.println(odoInput);

  drawNumberPad();

  drawSmallButton(
    20,
    215,
    85,
    20,
    "CLEAR"
  );

  drawSmallButton(
    215,
    215,
    85,
    20,
    "NEXT"
  );
}

void drawFuelLitersScreen() {
  currentScreen = FUEL_LITERS;

  tft.fillScreen(BG_COLOR);

  drawHeader("FUEL LOG");

  tft.setTextColor(TEXT_COLOR);
  tft.setTextSize(2);

  tft.setCursor(20, 55);
  tft.println("ENTER LITERS");

  tft.setTextSize(1);

  tft.setCursor(20, 82);
  tft.println("FUEL AMOUNT");

  tft.drawRect(
    20,
    100,
    280,
    40,
    BORDER_COLOR
  );

  tft.setTextSize(2);
  tft.setCursor(30, 112);
  tft.println(litersInput);

  drawNumberPad();

  drawSmallButton(
    20,
    215,
    85,
    20,
    "CLEAR"
  );

  drawSmallButton(
    215,
    215,
    85,
    20,
    "NEXT"
  );
}

void drawFuelPriceScreen() {
  currentScreen = FUEL_PRICE;

  tft.fillScreen(BG_COLOR);

  drawHeader("FUEL LOG");

  tft.setTextColor(TEXT_COLOR);
  tft.setTextSize(2);

  tft.setCursor(20, 55);
  tft.println("PRICE / LITER");

  tft.setTextSize(1);

  tft.setCursor(20, 82);
  tft.println("PHP PER LITER");

  tft.drawRect(
    20,
    100,
    280,
    40,
    BORDER_COLOR
  );

  tft.setTextSize(2);
  tft.setCursor(30, 112);
  tft.println(priceInput);

  drawNumberPad();

  drawSmallButton(
    20,
    215,
    85,
    20,
    "CLEAR"
  );

  drawSmallButton(
    215,
    215,
    85,
    20,
    "CALC"
  );
}

void drawNumberPad() {
  int startX = 20;
  int startY = 150;

  int buttonW = 48;
  int buttonH = 28;

  drawKey(
    startX,
    startY,
    buttonW,
    buttonH,
    "1"
  );

  drawKey(
    startX + 57,
    startY,
    buttonW,
    buttonH,
    "2"
  );

  drawKey(
    startX + 114,
    startY,
    buttonW,
    buttonH,
    "3"
  );

  drawKey(
    startX + 171,
    startY,
    buttonW,
    buttonH,
    "4"
  );

  drawKey(
    startX + 228,
    startY,
    buttonW,
    buttonH,
    "5"
  );

  drawKey(
    startX,
    startY + 34,
    buttonW,
    buttonH,
    "6"
  );

  drawKey(
    startX + 57,
    startY + 34,
    buttonW,
    buttonH,
    "7"
  );

  drawKey(
    startX + 114,
    startY + 34,
    buttonW,
    buttonH,
    "8"
  );

  drawKey(
    startX + 171,
    startY + 34,
    buttonW,
    buttonH,
    "9"
  );

  drawKey(
    startX + 228,
    startY + 34,
    buttonW,
    buttonH,
    "0"
  );

  drawKey(
    startX + 114,
    startY + 68,
    buttonW,
    buttonH,
    "."
  );
}

void drawKey(
  int x,
  int y,
  int w,
  int h,
  const char *label
) {
  tft.fillRect(
    x,
    y,
    w,
    h,
    BUTTON_COLOR
  );

  tft.drawRect(
    x,
    y,
    w,
    h,
    BORDER_COLOR
  );

  tft.setTextColor(TEXT_COLOR);
  tft.setTextSize(1);

  int textWidth =
    strlen(label) * 6;

  int textX =
    x + (w - textWidth) / 2;

  int textY =
    y + 10;

  tft.setCursor(
    textX,
    textY
  );

  tft.println(label);
}

void drawFuelResultScreen() {
  currentScreen = FUEL_RESULT;

  tft.fillScreen(BG_COLOR);

  drawHeader("FUEL RESULT");

  tft.setTextColor(TEXT_COLOR);
  tft.setTextSize(1);

  tft.setCursor(20, 55);

  tft.print("ODO: ");
  tft.print(currentOdo, 0);
  tft.println(" KM");

  tft.setCursor(20, 75);

  tft.print("DISTANCE: ");
  tft.print(distance, 0);
  tft.println(" KM");

  tft.setCursor(20, 95);

  tft.print("FUEL: ");
  tft.print(currentLiters, 1);
  tft.println(" L");

  tft.setCursor(20, 115);

  tft.print("COST: PHP ");
  tft.println(totalCost, 2);

  tft.setTextColor(GREEN_COLOR);
  tft.setTextSize(2);

  tft.setCursor(20, 145);

  tft.print("ECONOMY: ");
  tft.print(fuelEconomy, 2);
  tft.println(" KM/L");

  tft.setTextSize(1);

  tft.setCursor(20, 180);

  tft.println("SAVED TO /FUEL.CSV");

  drawSmallButton(
    110,
    215,
    100,
    20,
    "HOME"
  );
}

void drawHistoryScreen() {
  currentScreen = HISTORY;

  tft.fillScreen(BG_COLOR);

  drawHeader("FUEL HISTORY");

  tft.setTextColor(YELLOW_COLOR);
  tft.setTextSize(1);

  tft.setCursor(10, 50);
  tft.println("ODO");

  tft.setCursor(65, 50);
  tft.println("L");

  tft.setCursor(105, 50);
  tft.println("PHP");

  tft.setCursor(180, 50);
  tft.println("KM/L");

  tft.drawLine(
    10,
    62,
    310,
    62,
    BORDER_COLOR
  );

  if (!SD.exists("/FUEL.CSV")) {
    tft.setTextColor(RED_COLOR);

    tft.setCursor(60, 100);

    tft.println("NO FUEL RECORDS");

    drawSmallButton(
      110,
      215,
      100,
      20,
      "HOME"
    );

    return;
  }

  File file =
    SD.open("/FUEL.CSV");

  if (!file) {
    tft.setTextColor(RED_COLOR);

    tft.setCursor(50, 100);

    tft.println("ERROR READING SD");

    drawSmallButton(
      110,
      215,
      100,
      20,
      "HOME"
    );

    return;
  }

  if (file.available()) {
    file.readStringUntil('\n');
  }

  int y = 72;
  int recordCount = 0;

  while (
    file.available() &&
    recordCount < 6
  ) {
    String line =
      file.readStringUntil('\n');

    line.trim();

    if (line.length() == 0) {
      continue;
    }

    int comma1 =
      line.indexOf(',');

    int comma2 =
      line.indexOf(
        ',',
        comma1 + 1
      );

    int comma3 =
      line.indexOf(
        ',',
        comma2 + 1
      );

    int comma4 =
      line.indexOf(
        ',',
        comma3 + 1
      );

    int comma5 =
      line.indexOf(
        ',',
        comma4 + 1
      );

    if (
      comma1 == -1 ||
      comma2 == -1 ||
      comma3 == -1 ||
      comma4 == -1 ||
      comma5 == -1
    ) {
      continue;
    }

    String odo =
      line.substring(
        0,
        comma1
      );

    String liters =
      line.substring(
        comma1 + 1,
        comma2
      );

    String price =
      line.substring(
        comma2 + 1,
        comma3
      );

    String economy =
      line.substring(
        comma4 + 1,
        comma5
      );

    tft.setTextColor(TEXT_COLOR);
    tft.setTextSize(1);

    tft.setCursor(10, y);
    tft.println(odo);

    tft.setCursor(65, y);
    tft.println(liters);

    tft.setCursor(105, y);
    tft.println(price);

    tft.setCursor(180, y);
    tft.println(economy);

    tft.drawLine(
      10,
      y + 12,
      310,
      y + 12,
      ILI9341_DARKGREY
    );

    y += 23;

    recordCount++;
  }

  file.close();

  if (recordCount == 0) {
    tft.setTextColor(RED_COLOR);

    tft.setCursor(60, 100);

    tft.println("NO VALID RECORDS");
  }

  drawSmallButton(
    110,
    215,
    100,
    20,
    "HOME"
  );
}

void drawStatsScreen() {
  currentScreen = STATS;

  calculateStatistics();

  tft.fillScreen(BG_COLOR);

  drawHeader("FUEL STATS");

  tft.setTextColor(TEXT_COLOR);
  tft.setTextSize(1);

  tft.setCursor(20, 55);
  tft.print("TOTAL FUEL:");

  tft.setCursor(180, 55);
  tft.print(totalFuel, 2);
  tft.println(" L");

  tft.setCursor(20, 80);
  tft.print("TOTAL SPENDING:");

  tft.setCursor(180, 80);
  tft.print("PHP ");
  tft.println(totalSpending, 2);

  tft.setCursor(20, 105);
  tft.print("TOTAL DISTANCE:");

  tft.setCursor(180, 105);
  tft.print(totalDistance, 0);
  tft.println(" KM");

  tft.setCursor(20, 130);
  tft.print("AVG ECONOMY:");

  tft.setCursor(180, 130);
  tft.print(averageEconomy, 2);
  tft.println(" KM/L");

  tft.setCursor(20, 155);
  tft.print("FUEL ENTRIES:");

  tft.setCursor(180, 155);
  tft.println(fuelEntries);

  tft.setCursor(20, 180);
  tft.print("LAST ODO:");

  tft.setCursor(180, 180);
  tft.print(lastOdo, 0);
  tft.println(" KM");

  drawSmallButton(
    110,
    215,
    100,
    20,
    "HOME"
  );
}

void drawHeader(const char *title) {
  tft.fillRect(
    0,
    0,
    320,
    40,
    HEADER_COLOR
  );

  tft.setTextColor(TEXT_COLOR);
  tft.setTextSize(2);

  tft.setCursor(10, 12);

  tft.println(title);
}

void drawButton(
  int x,
  int y,
  int w,
  int h,
  const char *label
) {
  tft.fillRoundRect(
    x,
    y,
    w,
    h,
    8,
    BUTTON_COLOR
  );

  tft.drawRoundRect(
    x,
    y,
    w,
    h,
    8,
    BORDER_COLOR
  );

  tft.setTextColor(TEXT_COLOR);
  tft.setTextSize(2);

  int textWidth =
    strlen(label) * 12;

  int textX =
    x + (w - textWidth) / 2;

  int textY =
    y + (h / 2) - 8;

  tft.setCursor(
    textX,
    textY
  );

  tft.println(label);
}

void drawSmallButton(
  int x,
  int y,
  int w,
  int h,
  const char *label
) {
  tft.fillRect(
    x,
    y,
    w,
    h,
    BUTTON_COLOR
  );

  tft.drawRect(
    x,
    y,
    w,
    h,
    BORDER_COLOR
  );

  tft.setTextColor(TEXT_COLOR);
  tft.setTextSize(1);

  int textX =
    x + (w - strlen(label) * 6) / 2;

  int textY =
    y + 6;

  tft.setCursor(
    textX,
    textY
  );

  tft.println(label);
}

void handleTouch(int x, int y) {

  if (currentScreen == HOME) {

    if (
      x >= 20 &&
      x <= 150 &&
      y >= 60 &&
      y <= 115
    ) {
      odoInput = "";

      drawFuelOdoScreen();

      return;
    }

    if (
      x >= 20 &&
      x <= 150 &&
      y >= 130 &&
      y <= 185
    ) {
      drawStatsScreen();

      return;
    }

    if (
      x >= 170 &&
      x <= 300 &&
      y >= 130 &&
      y <= 185
    ) {
      drawHistoryScreen();

      return;
    }

    return;
  }

  if (currentScreen == FUEL_ODO) {

    if (
      x >= 20 &&
      x <= 105 &&
      y >= 215 &&
      y <= 235
    ) {
      odoInput = "";

      drawFuelOdoScreen();

      return;
    }

    if (
      x >= 215 &&
      x <= 300 &&
      y >= 215 &&
      y <= 235
    ) {
      if (odoInput.length() > 0) {

        currentOdo =
          odoInput.toFloat();

        litersInput = "";

        drawFuelLitersScreen();
      }

      return;
    }

    handleNumberInput(
      x,
      y,
      odoInput
    );

    return;
  }

  if (currentScreen == FUEL_LITERS) {

    if (
      x >= 20 &&
      x <= 105 &&
      y >= 215 &&
      y <= 235
    ) {
      litersInput = "";

      drawFuelLitersScreen();

      return;
    }

    if (
      x >= 215 &&
      x <= 300 &&
      y >= 215 &&
      y <= 235
    ) {
      if (litersInput.length() > 0) {

        currentLiters =
          litersInput.toFloat();

        priceInput = "";

        drawFuelPriceScreen();
      }

      return;
    }

    handleNumberInput(
      x,
      y,
      litersInput
    );

    return;
  }

  if (currentScreen == FUEL_PRICE) {

    if (
      x >= 20 &&
      x <= 105 &&
      y >= 215 &&
      y <= 235
    ) {
      priceInput = "";

      drawFuelPriceScreen();

      return;
    }

    if (
      x >= 215 &&
      x <= 300 &&
      y >= 215 &&
      y <= 235
    ) {
      if (priceInput.length() > 0) {

        currentPrice =
          priceInput.toFloat();

        calculateFuel();

        saveFuelRecord();

        drawFuelResultScreen();
      }

      return;
    }

    handleNumberInput(
      x,
      y,
      priceInput
    );

    return;
  }

  if (currentScreen == FUEL_RESULT) {

    if (
      x >= 110 &&
      x <= 210 &&
      y >= 215 &&
      y <= 235
    ) {
      drawHomeScreen();

      return;
    }
  }

  if (currentScreen == HISTORY) {

    if (
      x >= 110 &&
      x <= 210 &&
      y >= 215 &&
      y <= 235
    ) {
      drawHomeScreen();

      return;
    }
  }

  if (currentScreen == STATS) {

    if (
      x >= 110 &&
      x <= 210 &&
      y >= 215 &&
      y <= 235
    ) {
      drawHomeScreen();

      return;
    }
  }
}

void handleNumberInput(
  int x,
  int y,
  String &input
) {
  int startX = 20;
  int startY = 150;

  int buttonW = 48;
  int buttonH = 28;

  if (
    y >= startY &&
    y <= startY + buttonH
  ) {

    if (
      x >= startX &&
      x <= startX + buttonW
    ) {
      input += "1";

    } else if (
      x >= startX + 57 &&
      x <= startX + 57 + buttonW
    ) {
      input += "2";

    } else if (
      x >= startX + 114 &&
      x <= startX + 114 + buttonW
    ) {
      input += "3";

    } else if (
      x >= startX + 171 &&
      x <= startX + 171 + buttonW
    ) {
      input += "4";

    } else if (
      x >= startX + 228 &&
      x <= startX + 228 + buttonW
    ) {
      input += "5";
    }
  }

  else if (
    y >= startY + 34 &&
    y <= startY + 34 + buttonH
  ) {

    if (
      x >= startX &&
      x <= startX + buttonW
    ) {
      input += "6";

    } else if (
      x >= startX + 57 &&
      x <= startX + 57 + buttonW
    ) {
      input += "7";

    } else if (
      x >= startX + 114 &&
      x <= startX + 114 + buttonW
    ) {
      input += "8";

    } else if (
      x >= startX + 171 &&
      x <= startX + 171 + buttonW
    ) {
      input += "9";

    } else if (
      x >= startX + 228 &&
      x <= startX + 228 + buttonW
    ) {
      input += "0";
    }
  }

  else if (
    x >= startX + 114 &&
    x <= startX + 114 + buttonW &&
    y >= startY + 68 &&
    y <= startY + 68 + buttonH
  ) {

    if (input.indexOf('.') == -1) {
      input += ".";
    }
  }

  if (currentScreen == FUEL_ODO) {

    drawFuelOdoScreen();

  } else if (
    currentScreen == FUEL_LITERS
  ) {

    drawFuelLitersScreen();

  } else if (
    currentScreen == FUEL_PRICE
  ) {

    drawFuelPriceScreen();
  }
}

void calculateFuel() {

  previousOdo =
    getLastOdometer();

  distance =
    currentOdo - previousOdo;

  totalCost =
    currentLiters * currentPrice;

  if (
    currentLiters > 0 &&
    distance > 0
  ) {

    fuelEconomy =
      distance / currentLiters;

  } else {

    fuelEconomy = 0;
  }

  Serial.println();
  Serial.println("FUEL CALCULATION");

  Serial.print("Previous ODO: ");
  Serial.println(previousOdo);

  Serial.print("Current ODO: ");
  Serial.println(currentOdo);

  Serial.print("Distance: ");
  Serial.println(distance);

  Serial.print("Liters: ");
  Serial.println(currentLiters);

  Serial.print("Price/L: ");
  Serial.println(currentPrice);

  Serial.print("Total Cost: ");
  Serial.println(totalCost);

  Serial.print("Fuel Economy: ");
  Serial.println(fuelEconomy);
}

float getLastOdometer() {

  if (!SD.exists("/FUEL.CSV")) {

    Serial.println(
      "No previous fuel record."
    );

    return 0;
  }

  File file =
    SD.open("/FUEL.CSV");

  if (!file) {

    Serial.println(
      "Could not open FUEL.CSV"
    );

    return 0;
  }

  float lastOdoValue = 0;

  if (file.available()) {

    file.readStringUntil('\n');
  }

  while (file.available()) {

    String line =
      file.readStringUntil('\n');

    line.trim();

    if (line.length() == 0) {
      continue;
    }

    int firstComma =
      line.indexOf(',');

    if (firstComma == -1) {
      continue;
    }

    String odo =
      line.substring(
        0,
        firstComma
      );

    lastOdoValue =
      odo.toFloat();
  }

  file.close();

  Serial.print("Last ODO from SD: ");
  Serial.println(lastOdoValue);

  return lastOdoValue;
}

void saveFuelRecord() {

  bool newFile =
    !SD.exists("/FUEL.CSV");

  File file =
    SD.open(
      "/FUEL.CSV",
      FILE_APPEND
    );

  if (!file) {

    Serial.println(
      "ERROR: Could not open FUEL.CSV"
    );

    return;
  }

  if (newFile) {

    file.println(
      "ODO,LITERS,PRICE_PER_LITER,TOTAL_COST,DISTANCE,KM_PER_LITER"
    );
  }

  file.print(
    currentOdo,
    0
  );

  file.print(",");

  file.print(
    currentLiters,
    2
  );

  file.print(",");

  file.print(
    currentPrice,
    2
  );

  file.print(",");

  file.print(
    totalCost,
    2
  );

  file.print(",");

  file.print(
    distance,
    2
  );

  file.print(",");

  file.println(
    fuelEconomy,
    2
  );

  file.close();

  Serial.println(
    "Fuel record saved to SD."
  );
}

void calculateStatistics() {

  totalFuel = 0;
  totalSpending = 0;
  totalDistance = 0;
  averageEconomy = 0;
  fuelEntries = 0;
  lastOdo = 0;

  if (!SD.exists("/FUEL.CSV")) {
    return;
  }

  File file =
    SD.open("/FUEL.CSV");

  if (!file) {
    return;
  }

  if (file.available()) {
    file.readStringUntil('\n');
  }

  while (file.available()) {

    String line =
      file.readStringUntil('\n');

    line.trim();

    if (line.length() == 0) {
      continue;
    }

    int comma1 =
      line.indexOf(',');

    int comma2 =
      line.indexOf(
        ',',
        comma1 + 1
      );

    int comma3 =
      line.indexOf(
        ',',
        comma2 + 1
      );

    int comma4 =
      line.indexOf(
        ',',
        comma3 + 1
      );

    int comma5 =
      line.indexOf(
        ',',
        comma4 + 1
      );

    if (
      comma1 == -1 ||
      comma2 == -1 ||
      comma3 == -1 ||
      comma4 == -1 ||
      comma5 == -1
    ) {
      continue;
    }

    String odoString =
      line.substring(
        0,
        comma1
      );

    String litersString =
      line.substring(
        comma1 + 1,
        comma2
      );

    String costString =
      line.substring(
        comma3 + 1,
        comma4
      );

    String distanceString =
      line.substring(
        comma4 + 1,
        comma5
      );

    String economyString =
      line.substring(
        comma5 + 1
      );

    float odo =
      odoString.toFloat();

    float liters =
      litersString.toFloat();

    float cost =
      costString.toFloat();

    float recordDistance =
      distanceString.toFloat();

    float economy =
      economyString.toFloat();

    totalFuel += liters;

    totalSpending += cost;

    totalDistance += recordDistance;

    if (economy > 0) {
      averageEconomy += economy;
    }

    fuelEntries++;

    lastOdo = odo;
  }

  file.close();

  if (fuelEntries > 0) {

    averageEconomy =
      averageEconomy / fuelEntries;
  }

  Serial.println();
  Serial.println("FUEL STATISTICS");

  Serial.print("Total Fuel: ");
  Serial.println(totalFuel);

  Serial.print("Total Spending: ");
  Serial.println(totalSpending);

  Serial.print("Total Distance: ");
  Serial.println(totalDistance);

  Serial.print("Average Economy: ");
  Serial.println(averageEconomy);

  Serial.print("Fuel Entries: ");
  Serial.println(fuelEntries);

  Serial.print("Last ODO: ");
  Serial.println(lastOdo);
}
