/*
 * 03_Escaner - Herramienta de mapeo: enciende un solo bit de RAM a la vez
 * e imprime cual es por Serial.
 *
 * Usalo si conectas otra placa con ET1620 y necesitas descubrir que pad
 * corresponde a que SEG/GRID. Para el HF-8890DT el mapeo ya esta resuelto
 * dentro de la clase HF8890DT; esto queda como referencia y para verificar.
 *
 * Comandos por Serial (115200):
 *   s          arrancar escaneo
 *   x          detener
 *   n          siguiente paso manual
 *   v MS       velocidad en ms por paso (default 2000)
 *   b A B      encender solo addr A bit B
 *   c          limpiar
 *   a          encender toda la RAM
 */

#include <ET1620.h>

ET1620 chip(8, 9, 10);   // DIN, CLK, STB

bool     escaneando = false;
uint8_t  scanAddr = 0, scanBit = 0;
uint32_t scanT = 0;
uint16_t scanMs = 2000;

void paso() {
  chip.setAutoUpdate(false);
  for (uint8_t i = 0; i < ET1620::RAM_SIZE; i++) chip.writeRam(i, 0);
  chip.writeRam(scanAddr, (uint8_t)(1 << scanBit));
  chip.setAutoUpdate(true);

  Serial.print(F("addr=0x"));
  if (scanAddr < 16) Serial.print('0');
  Serial.print(scanAddr, HEX);
  Serial.print(F("  bit=")); Serial.print(scanBit);
  Serial.print(F("  ->  GRID")); Serial.print(scanAddr / 2 + 1);
  Serial.print(F("  SEG")); Serial.print(scanAddr % 2 == 0 ? scanBit + 1 : scanBit + 9);
  if (scanAddr % 2 != 0) Serial.print(F("  (no existe en modo 8x6)"));
  Serial.println();

  if (++scanBit > 7) {
    scanBit = 0;
    if (++scanAddr >= ET1620::RAM_SIZE) {
      scanAddr = 0;
      Serial.println(F("--- fin del ciclo ---"));
    }
  }
}

void setup() {
  Serial.begin(115200);
  chip.begin(ET1620::MODE_8SEG_6GRID);
  chip.setBrightness(7);

  uint32_t t0 = millis();
  while (!Serial && millis() - t0 < 4000) { }

  Serial.println(F("Escaner ET1620. 's' para arrancar, 'n' paso a paso."));
}

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd.length() == 0) return;

    switch (cmd.charAt(0)) {
      case 's': escaneando = true; scanAddr = 0; scanBit = 0; scanT = 0; break;
      case 'x': escaneando = false; Serial.println(F("[detenido]")); break;
      case 'n': escaneando = false; paso(); break;
      case 'v': {
        int sp = cmd.indexOf(' ');
        if (sp > 0) { scanMs = (uint16_t)constrain(cmd.substring(sp + 1).toInt(), 100, 20000);
                      Serial.print(F("[vel] ")); Serial.println(scanMs); }
        break;
      }
      case 'b': {
        escaneando = false;
        int sp = cmd.indexOf(' '), sp2 = cmd.indexOf(' ', sp + 1);
        if (sp < 0 || sp2 < 0) { Serial.println(F("uso: b <addr> <bit>")); break; }
        uint8_t a = cmd.substring(sp + 1, sp2).toInt();
        uint8_t b = cmd.substring(sp2 + 1).toInt();
        chip.setAutoUpdate(false);
        for (uint8_t i = 0; i < ET1620::RAM_SIZE; i++) chip.writeRam(i, 0);
        chip.writeBit(a, b, true);
        chip.setAutoUpdate(true);
        break;
      }
      case 'a': escaneando = false; chip.fill(0xFF); break;
      case 'c': escaneando = false; chip.clear(); break;
    }
  }

  if (escaneando && millis() - scanT >= scanMs) {
    scanT = millis();
    paso();
  }
}
