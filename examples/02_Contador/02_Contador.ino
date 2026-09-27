/*
 * 02_Contador - Conteo, numeros negativos, decimales y control de brillo.
 */

#include <HF8890DT.h>

HF8890DT display(8, 9, 10);   // DIN, CLK, STB

void setup() {
  display.begin();
  display.setBrightness(7);
}

void loop() {
  // Conteo 0 -> 999
  for (int i = 0; i <= 999; i++) {
    display.print(i);
    delay(10);
  }

  // Negativos (rango -99 a 999)
  for (int i = 0; i >= -99; i--) {
    display.print(i);
    delay(20);
  }

  // Decimales
  for (float v = 0.0f; v < 10.0f; v += 0.1f) {
    display.print(v, 1);
    delay(40);
  }

  // Fade de brillo sobre un valor fijo
  display.print("8.8.8.");
  for (int8_t b = 7; b >= 0; b--) { display.setBrightness(b); delay(150); }
  for (int8_t b = 0; b <= 7; b++) { display.setBrightness(b); delay(150); }

  // Parpadeo usando el on/off del chip (no reescribe la RAM)
  for (uint8_t i = 0; i < 6; i++) {
    display.displayOff(); delay(200);
    display.displayOn();  delay(200);
  }
}
