

//-------------------------------------------------------------------------------
//
//  Written by Tim Inman
//
//-------------------------------------------------------------------------------

#include <Wire.h>
#include <SPI.h>
#include <TinyScreen.h>
#include <Adafruit_BMP280.h>
#include <Time.h>
#include <TimeLib.h>

//Library must be passed the board type
//TinyScreenDefault for TinyScreen shields
//TinyScreenAlternate for alternate address TinyScreen shields
//TinyScreenPlus for TinyScreen+
TinyScreen display = TinyScreen(TinyScreenPlus);
int topRowSelected = 0;
int bottomRowSelected = 0;
int mode = 0;
Adafruit_BMP280 bme; // I2C is the interface used in TinyScreen+ for all sensors

void setup(void) {
  Wire.begin();//initialize I2C before we can initialize TinyScreen- not needed for TinyScreen+
  display.begin();
  //setBrightness(brightness);//sets main current level, valid levels are 0-15
  display.setBrightness(15);
  display.setFlip(true);
  setTime(16, 06, 0, 7, 16, 2016);

}

void loop() {
  if (mode == 0) {
    display.clearScreen();
    delay(200);

  } else if (mode == 1) {
    sensors();
  } else if (mode == 2) {
    calculator();
  } else if (mode == 3) {
    watch();
  }

  modeSelect();
  delay(200);

}

void sensors() {
  display.clearScreen();

  display.setFont(liberationSans_8ptFontInfo);
  display.fontColor(TS_8b_White, TS_8b_Black);
  if (!bme.begin()) //check if you are able to read the sensor
  {
    display.print("Not valid BMP280");
    while (1);
  }
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

void watch() {
  display.clearScreen();
  display.setCursor(30, 22);
  display.setFont(liberationSans_8ptFontInfo);
  display.fontColor(TS_8b_White, TS_8b_Black);
  display.print(hourFormat12());
  display.print(":");
  display.print(minute());
  display.print(":");
  display.print(second());
  if (isAM()) {
    display.print("AM");
  } else {
    display.print("PM");
  }
  float battVoltageReading = getBattVoltage();

  // Print to TinyScreen+
  display.fontColor(TS_8b_Blue, TS_8b_Black); // Because blue is cool
  display.setCursor(0, 40);

  display.print(battVoltageReading);
  display.print("v");
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
    mode += 1;
    if (mode > 3) {
      mode = 0;
    }
  }
}
float getVCC() {
  SYSCTRL->VREF.reg |= SYSCTRL_VREF_BGOUTEN;
  while (ADC->STATUS.bit.SYNCBUSY == 1);
  ADC->SAMPCTRL.bit.SAMPLEN = 0x1;
  while (ADC->STATUS.bit.SYNCBUSY == 1);
  ADC->INPUTCTRL.bit.MUXPOS = 0x19;         // Internal bandgap input
  while (ADC->STATUS.bit.SYNCBUSY == 1);
  ADC->CTRLA.bit.ENABLE = 0x01;             // Enable ADC
  while (ADC->STATUS.bit.SYNCBUSY == 1);
  ADC->SWTRIG.bit.START = 1;  // Start conversion
  ADC->INTFLAG.bit.RESRDY = 1;  // Clear the Data Ready flag
  while (ADC->STATUS.bit.SYNCBUSY == 1);
  ADC->SWTRIG.bit.START = 1;  // Start the conversion again to throw out first value
  while ( ADC->INTFLAG.bit.RESRDY == 0 );   // Waiting for conversion to complete
  uint32_t valueRead = ADC->RESULT.reg;
  while (ADC->STATUS.bit.SYNCBUSY == 1);
  ADC->CTRLA.bit.ENABLE = 0x00;             // Disable ADC
  while (ADC->STATUS.bit.SYNCBUSY == 1);
  SYSCTRL->VREF.reg &= ~SYSCTRL_VREF_BGOUTEN;
  float vcc = (1.1 * 1023.0) / valueRead;
  return vcc;
}

// Calculate the battery voltage
float getBattVoltage(void) {
  const int VBATTpin = A4;
  float VCC = getVCC();

  // Use resistor division and math to get the voltage
  float resistorDiv = 0.5;
  float ADCres = 1023.0;
  float battVoltageReading = analogRead(VBATTpin);
  battVoltageReading = analogRead(VBATTpin); // Throw out first value
  float battVoltage = VCC * battVoltageReading / ADCres / resistorDiv;

  return battVoltage;
}
