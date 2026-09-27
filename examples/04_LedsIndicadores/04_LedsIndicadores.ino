/*
 * 04_LedsIndicadores - Los LEDs del panel cuelgan de GRID3, GRID4 y GRID5.
 *
 * Este ejemplo los recorre uno por uno mostrando en el display que grid y
 * que bit se esta encendiendo, para que termines de identificar cual LED
 * fisico es cual. Anota la correspondencia mientras corre.
 *
 * El display y los LEDs son independientes: escribir LEDs no borra el numero.
 */

#include <HF8890DT.h>

HF8890DT display(8, 9, 10);   // DIN, CLK, STB

void setup() {
  Serial.begin(115200);
  display.begin();
  display.setBrightness(7);
}

void loop() {
  for (uint8_t grid = 3; grid <= 5; grid++) {
    for (uint8_t bit = 0; bit < 8; bit++) {
      display.clearLeds();
      display.setLed(grid, bit, true);

      // Muestra "G.B" en el display: grid en el primer digito, bit en el ultimo
      char buf[5];
      buf[0] = (char)('0' + grid);
      buf[1] = '.';
      buf[2] = ' ';
      buf[3] = (char)('0' + bit);
      buf[4] = '\0';
      display.print(buf);

      Serial.print(F("GRID")); Serial.print(grid);
      Serial.print(F("  bit")); Serial.println(bit);

      delay(1500);
    }
  }

  // Todos juntos un momento
  display.print("ALL");
  for (uint8_t g = 3; g <= 5; g++) display.setLedGrid(g, 0xFF);
  delay(2000);
  display.clearLeds();
}
