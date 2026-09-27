/*
 * 01_HolaMundo - Lo minimo para ver algo en el display del panel HF-8890DT.
 *
 * Conexiones (Arduino Leonardo):
 *   D8  -> ET1620 pin 18 (DIN)   + pull-up 4.7k a VDD
 *   D9  -> ET1620 pin 19 (CLK)
 *   D10 -> ET1620 pin 20 (STB)
 *   GND comun. VDD del panel a 5V.
 */

#include <HF8890DT.h>

HF8890DT display(8, 9, 10);   // DIN, CLK, STB

void setup() {
  display.begin();
  display.setBrightness(7);

  display.testPattern();      // 8.8.8. + todos los LEDs
  delay(1500);
  display.clear();

  display.print("HOL");
  delay(1000);
  display.print(123);
}

void loop() {
}
