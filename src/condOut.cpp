/*
    This file is part of the PolarPlotterCore library.
    Copyright (c) 2024 Benjamin Carleski

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#include "condOut.h"

CondOut::CondOut(LiquidCrystal& lcd, bool useLcd)
  : lcd(lcd),
    useLcd(useLcd) {
}

void CondOut::init() {
  if (useLcd) {
    lcd.begin(16, 2);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Setup");
  }
  Serial.begin(115200);
  unsigned long start = millis();

  // Wait for up to 3 seconds for the Serial device to become available
  while (!Serial && (millis() - start) <= 3000)
    ;

  Serial.println("Starting");
}

void CondOut::print(String& value) {
  if (Serial) {
    Serial.print(value);
  }
}

void CondOut::print(const char value[]) {
  if (Serial) {
    Serial.print(value);
  }
}

void CondOut::print(int value, int base) {
  if (Serial) {
    Serial.print(value, base);
  }
}

void CondOut::print(unsigned int value, int base) {
  if (Serial) {
    Serial.print(value, base);
  }
}

void CondOut::print(float value, int base) {
  if (Serial) {
    Serial.print(value, base);
  }
}

void CondOut::print(double value, int base) {
  if (Serial) {
    Serial.print(value, base);
  }
}

void CondOut::print(long value, int base) {
  if (Serial) {
    Serial.print(value, base);
  }
}

void CondOut::print(unsigned long value, int base) {
  if (Serial) {
    Serial.print(value, base);
  }
}

void CondOut::print(String name, Point& value) {
  if (Serial) {
    Serial.print(name);
    Serial.print("=(");
    Serial.print(value.getX(), 4);
    Serial.print(", ");
    Serial.print(value.getY(), 4);
    Serial.print(", ");
    Serial.print(value.getRadius(), 4);
    Serial.print(", ");
    Serial.print(value.getAzimuth(), 4);
    Serial.print(")");
  }
}

void CondOut::println(String& value) {
  if (Serial) {
    Serial.println(value);
  }
}

void CondOut::println(const char value[]) {
  if (Serial) {
    Serial.println(value);
  }
}

void CondOut::println(int value, int base) {
  if (Serial) {
    Serial.println(value, base);
  }
}

void CondOut::println(unsigned int value, int base) {
  if (Serial) {
    Serial.println(value, base);
  }
}

void CondOut::println(float value, int base) {
  if (Serial) {
    Serial.println(value, base);
  }
}

void CondOut::println(double value, int base) {
  if (Serial) {
    Serial.println(value, base);
  }
}

void CondOut::println(long value, int base) {
  if (Serial) {
    Serial.println(value, base);
  }
}

void CondOut::println(unsigned long value, int base) {
  if (Serial) {
    Serial.println(value, base);
  }
}

void CondOut::println(String name, Point& value) {
  if (Serial) {
    this->print(name, value);
    Serial.println();
  }
}

void CondOut::clear() {
  if (useLcd) {
    lcd.clear();
  }
}

void CondOut::setCursor(int col, int row) {
  if (useLcd) {
    lcd.setCursor(col, row);
  }
}

void CondOut::lcdPrint(String& value) {
  if (useLcd) {
    lcd.print(value);
  }
}

void CondOut::lcdPrint(const char value[]) {
  if (useLcd) {
    lcd.print(value);
  }
}

void CondOut::lcdPrint(long value, int base) {
  if (useLcd) {
    lcd.print(value, base);
  }
}

void CondOut::lcdPrint(const char line1[], String& line2) {
  if (useLcd) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(line1);
    lcd.setCursor(0, 1);
    lcd.print(line2);
  }
}

void CondOut::lcdPrint(const char line1[], const char line2[]) {
  if (useLcd) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(line1);
    lcd.setCursor(0, 1);
    lcd.print(line2);
  }
}
