/*
 * 05_DriverCrudo - Uso de la clase generica ET1620 sin la capa de placa.
 *
 * Util si conectas el chip a otro display, o si quieres animaciones que
 * trabajen directo sobre la RAM del chip.
 *
 * Muestra ademas el patron de agrupar escrituras: con setAutoUpdate(false)
 * modificas todo lo que quieras y mandas un solo flush con update(), en vez
 * de una transmision serial completa por cada cambio.
 */

#include <ET1620.h>

ET1620 chip(8, 9, 10);   // DIN, CLK, STB

void setup() {
  chip.begin(ET1620::MODE_8SEG_6GRID);
  chip.setBrightness(ET1620::DUTY_14_16);
}

void loop() {
  // Animacion: una "serpiente" recorriendo los 6 grids
  for (uint8_t g = 1; g <= 6; g++) {
    chip.setAutoUpdate(false);
    for (uint8_t i = 1; i <= 6; i++) chip.writeGrid(i, 0x00);
    chip.writeGrid(g, 0xFF);
    chip.setAutoUpdate(true);   // hace el flush de golpe
    delay(120);
  }

  // Barrido bit a bit dentro de un solo grid
  for (uint8_t bit = 0; bit < 8; bit++) {
    chip.writeGrid(1, (uint8_t)(1 << bit));
    delay(80);
  }

  chip.clear();
  delay(300);
}
