#include <Adafruit_GFX.h>
#include <SPI.h>
#include <Adafruit_ILI9341.h>
#include <Arduino.h>
#include <Adafruit_FT6206.h>
#include <SD.h>

// PINS

#define TFT_DC 2
#define TFT_CS 15

#define SD_CS 5

#define SPI_SCK 12
#define SPI_MISO 13
#define SPI_MOSI 11

#define I2C_SDA 10
#define I2C_SCL 8

// DISPLAY

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

// TOUCHSCREEN

Adafruit_FT6206 ctp = Adafruit_FT6206();

// COLORS

#define BG_COLOR       ILI9341_BLACK
#define HEADER_COLOR   ILI9341_BLUE
#define TEXT_COLOR     ILI9341_WHITE
#define BUTTON_COLOR   ILI9341_DARKGREY
#define BORDER_COLOR   ILI9341_CYAN
#define GREEN_COLOR    ILI9341_GREEN
#define RED_COLOR      ILI9341_RED
#define YELLOW_COLOR   ILI9341_YELLOW

// APPLICATION STATES

enum Screen {
  HOME,
  FUEL_ODO,
  FUEL_LITERS,
  FUEL_PRICE,
  FUEL_RESULT
};

Screen currentScreen = HOME;

// FUEL DATA

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

// SETUP

void setup() {

  Serial.begin(115200);

  delay(500);

  Serial.println();
  Serial.println("==============================");
  Serial.println("  FUEL & MAINTENANCE CYBERDECK");
  Serial.println("==============================");

  // I2C

  Wire.setPins(I2C_SDA, I2C_SCL);

  Serial.println("I2C pins configured");

  // SPI

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

  // DISPLAY

  Serial.println("Starting ILI9341...");

  tft.begin();

  tft.setRotation(1);

  tft.fillScreen(BG_COLOR);

  Serial.println("Display OK");

  // TOUCHSCREEN

  Serial.println("Starting FT6206...");

  if (!ctp.begin(40)) {

    Serial.println("Touchscreen failed!");

    while (1) {
      delay(100);
    }
  }

  Serial.println("Touchscreen OK");

  // SD CARD

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

  // HOME

  drawHomeScreen();

  Serial.println("System ready.");
}

// LOOP

void loop() {

  if (ctp.touched()) {

    TS_Point p = ctp.getPoint();

    int x = map(p.y, 0, 320, 0, 320);
    int y = map(p.x, 0, 240, 240, 0);

    Serial.print("Touch: ");
    Serial.print(x);
    Serial.print(", ");
    Serial.println(y);

    handleTouch(x, y);

    delay(250);
  }
}

// HOME SCREEN

void drawHomeScreen() {

  currentScreen = HOME;

  tft.fillScreen(BG_COLOR);

  // header

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

  // status

  tft.setTextSize(1);

  tft.setCursor(250, 15);

  tft.println("ONLINE");

  // buttons

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

  // status

  tft.setTextColor(GREEN_COLOR);

  tft.setTextSize(1);

  tft.setCursor(20, 215);

  tft.println("SYSTEM READY");

  tft.setCursor(220, 215);

  tft.println("SD: READY");
}

// FUEL ODOMETER SCREEN

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

  // input box

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

  // keypad

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

// FUEL LITERS SCREEN

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

// FUEL PRICE SCREEN

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

// NUMBER PAD

void drawNumberPad() {

  int startX = 20;
  int startY = 150;

  int buttonW = 55;
  int buttonH = 28;

  int gap = 8;

  drawKey(startX, startY, buttonW, buttonH, "1");
  drawKey(startX + 63, startY, buttonW, buttonH, "2");
  drawKey(startX + 126, startY, buttonW, buttonH, "3");
  drawKey(startX + 189, startY, buttonW, buttonH, "4");
  drawKey(startX + 252, startY, buttonW, buttonH, "5");

  drawKey(startX, startY + 34, buttonW, buttonH, "6");
  drawKey(startX + 63, startY + 34, buttonW, buttonH, "7");
  drawKey(startX + 126, startY + 34, buttonW, buttonH, "8");
  drawKey(startX + 189, startY + 34, buttonW, buttonH, "9");
  drawKey(startX + 252, startY + 34, buttonW, buttonH, "0");

  drawKey(
    startX + 126,
    startY + 68,
    buttonW,
    buttonH,
    "."
  );
}

// DRAW NUMBER KEY

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

  tft.setCursor(
    x + 24,
    y + 10
  );

  tft.println(label);
}
// FUEL RESULT

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

// HEADER

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

// BUTTON

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

  int textWidth = strlen(label) * 12;

  int textX =
    x + (w - textWidth) / 2;

  int textY =
    y + (h / 2) - 8;

  tft.setCursor(textX, textY);

  tft.println(label);
}

// SMALL BUTTON
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

  tft.setCursor(textX, textY);

  tft.println(label);
}
// TOUCH HANDLER


void handleTouch(int x, int y) {

  // HOME
  if (currentScreen == HOME) {

    // FUELLOG

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
  }

  // ODOMETER
  if (currentScreen == FUEL_ODO) {

    handleNumberInput(
      x,
      y,
      odoInput
    );

    //CLEAR
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

    //NEXT

    if (
      x >= 215 &&
      x <= 300 &&
      y >= 215 &&
      y <= 235
    ) {

      if (odoInput.length() > 0) {
        currentOdo = odoInput.toFloat();
        litersInput = "";
        drawFuelLitersScreen();
      }

      return;
    }
  }

  // LITERS
  if (currentScreen == FUEL_LITERS) {

    handleNumberInput(
      x,
      y,
      litersInput
    );

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
  }

  // PRICE
  if (currentScreen == FUEL_PRICE) {

    handleNumberInput(x, y, priceInput);

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
  }

  // RESULT
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
}

// NUMBER INPUT
void handleNumberInput(
  int x,
  int y,
  String &input
) {

  int startX = 20;
  int startY = 150;

  int buttonW = 55;
  int buttonH = 28;

  // row1
  if (
    y >= startY &&
    y <= startY + buttonH
  ) {

    if (x >= startX &&
        x <= startX + buttonW) {
      input += "1";
    }

    else if (
      x >= startX + 63 &&
      x <= startX + 63 + buttonW
    ) {
      input += "2";
    }

    else if (
      x >= startX + 126 &&
      x <= startX + 126 + buttonW
    ) {
      input += "3";
    }

    else if (
      x >= startX + 189 &&
      x <= startX + 189 + buttonW
    ) {
      input += "4";
    }

    else if (
      x >= startX + 252 &&
      x <= startX + 252 + buttonW
    ) {
      input += "5";
    }
  }

  //row2

  else if (
    y >= startY + 34 &&
    y <= startY + 34 + buttonH
  ) {

    if (
      x >= startX &&
      x <= startX + buttonW
    ) {
      input += "6";
    }

    else if (
      x >= startX + 63 &&
      x <= startX + 63 + buttonW
    ) {
      input += "7";
    }

    else if (
      x >= startX + 126 &&
      x <= startX + 126 + buttonW
    ) {
      input += "8";
    }

    else if (
      x >= startX + 189 &&
      x <= startX + 189 + buttonW
    ) {
      input += "9";
    }

    else if (
      x >= startX + 252 &&
      x <= startX + 252 + buttonW
    ) {
      input += "0";
    }
  }

  // decimalpoint
  else if (
    x >= startX + 126 &&
    x <= startX + 126 + buttonW &&
    y >= startY + 68 &&
    y <= startY + 68 + buttonH
  ) {

    if (input.indexOf('.') == -1) {
      input += ".";
    }
  }

  // redraw current screen
  if (currentScreen == FUEL_ODO) {
    drawFuelOdoScreen();
  }

  else if (currentScreen == FUEL_LITERS) {
    drawFuelLitersScreen();
  }

  else if (currentScreen == FUEL_PRICE) {
    drawFuelPriceScreen();
  }
}

// CALCULATE FUEL
void calculateFuel() {

  // read previous odometer

  previousOdo = getLastOdometer();
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
  Serial.println("===== FUEL CALCULATION =====");

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

  Serial.println("============================");
}

// GET LAST ODOMETER
float getLastOdometer() {

  if (!SD.exists("/FUEL.CSV")) {
    Serial.println(
      "No previous fuel record."
    );
    return currentOdo;
  }

  File file =
    SD.open("/FUEL.CSV");

  if (!file) {
    Serial.println(
      "Could not open FUEL.CSV"
    );
    return currentOdo;
  }

  float lastOdo = 0;

  while (file.available()) {
    String line =
      file.readStringUntil('\n');
    if (line.length() == 0) {
      continue;
    }

    int firstComma =
      line.indexOf(',');

    if (firstComma == -1) {
      continue;
    }

    int secondComma =
      line.indexOf(
        ',',
        firstComma + 1
      );

    if (secondComma == -1) {
      continue;
    }

    String odo =
      line.substring(
        firstComma + 1,
        secondComma
      );

    lastOdo =
      odo.toFloat();
  }

  file.close();

  return lastOdo;
}

// SAVE FUEL RECORD
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

  //header

  if (newFile) {
    file.println(
      "ODO,LITERS,PRICE_PER_LITER,TOTAL_COST,DISTANCE,KM_PER_LITER"
    );
  }

  // record
  file.print(currentOdo, 0);
  file.print(",");

  file.print(currentLiters, 2);
  file.print(",");

  file.print(currentPrice, 2);
  file.print(",");

  file.print(totalCost, 2);
  file.print(",");

  file.print(distance, 2);
  file.print(",");

  file.println(fuelEconomy, 2);

  file.close();

  Serial.println(
    "Fuel record saved to SD."
  );
}