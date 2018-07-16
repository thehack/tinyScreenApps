//-------------------------------------------------------------------------------
//
//  Written by Tim Inman
//
//-------------------------------------------------------------------------------

#include <Wire.h>
#include <SPI.h>
#include <TinyScreen.h>
#include <Adafruit_BMP280.h>

//Library must be passed the board type
//TinyScreenDefault for TinyScreen shields
//TinyScreenAlternate for alternate address TinyScreen shields
//TinyScreenPlus for TinyScreen+
TinyScreen display = TinyScreen(TinyScreenPlus);

int topRowSelected = 0;
int bottomRowSelected = 0;
int mode = 1;
Adafruit_BMP280 bme; // I2C is the interface used in TinyScreen+ for all sensors

void setup(void) {
  Wire.begin();//initialize I2C before we can initialize TinyScreen- not needed for TinyScreen+
  display.begin();
  //setBrightness(brightness);//sets main current level, valid levels are 0-15
  display.setBrightness(15);
  display.setFlip(true);
}

void loop() {
  if (mode == -1) {
    calculator();
  } else if (mode == 1) {
    sensors();
  }
  modeSelect();
      delay(200);

}

void sensors() {
  display.setFont(liberationSans_8ptFontInfo);
  display.fontColor(TS_8b_White, TS_8b_Black);
  if (!bme.begin()) //check if you are able to read the sensor
  {
    display.print("Not valid BMP280");
    while (1);
  }
  display.clearScreen();
  display.setCursor(5, 5);
  display.print("Temp = ");
  //call library function to reach temp value and converting it to farenheit
  display.print((bme.readTemperature() * 9 / 5 + 32 - 13)); //you can delete '*9/5+32-13' to show in celsius
  display.print(" *F");
  display.setCursor(5, 25);
  display.print("Pr = ");
  //call library function to reach pressure value
  display.print(bme.readPressure() / 3389.39);
  display.print(" inHg.");
  display.setCursor(5, 45);
  display.print("Alt = ");
  //call library function to calculate altitude from pressure
  display.print(bme.readAltitude(1015.3)); // this should be adjusted to your local forcase
  display.print(" m  ");
}
void calculator() {
  display.clearScreen();
  if (topRowSelected > -1) {
    display.setFont(thinPixel7_10ptFontInfo);

    int i;
    for (i = 0; i < 10; i = i + 1) {
      display.setCursor((10 + i * 8) , 5);

      if (topRowSelected == i) {
        display.fontColor(TS_8b_White, TS_8b_Black);
      }
      else {
        display.fontColor(TS_8b_Blue, TS_8b_Black);
      }

      display.print(i);
      display.setCursor((10 + i * 8) , 20);

      if (bottomRowSelected == i) {
        display.fontColor(TS_8b_White, TS_8b_Black);
      }
      else {
        display.fontColor(TS_8b_Blue, TS_8b_Black);
      }

      display.print(i);
    }

    display.fontColor(TS_8b_Yellow, TS_8b_Black);

    display.setCursor(10, 35);
    display.setFont(liberationSans_22ptFontInfo);
    display.print(bottomRowSelected * topRowSelected);
  }
  readInput();
}

void readInput() {
  if (display.getButtons(TSButtonUpperLeft)) {
    topRowSelected = topRowSelected - 1;
  }
  if (display.getButtons(TSButtonLowerLeft)) {
    bottomRowSelected = bottomRowSelected - 1;
  }
  if (display.getButtons(TSButtonUpperRight)) {
    topRowSelected = topRowSelected + 1;
  }
  if (display.getButtons(TSButtonLowerRight)) {
    bottomRowSelected = bottomRowSelected + 1;
  }
}

void modeSelect() {
  if (display.getButtons(TSButtonUpperLeft) && display.getButtons(TSButtonUpperRight)) {
    mode = mode * -1;
  }
}
