/*
 * HF8890DT.h - Capa de placa para el panel tactil HF-8890DT / CL-8890DT-CU-SEB
 *
 * Hereda de ET1620 y le agrega el mapeo fisico que se determino por escaneo
 * bit a bit sobre la placa real:
 *
 *   Segmentos (bit de RAM dentro del grid):
 *     a = 0 (SEG1)   b = 2 (SEG3)   c = 6 (SEG7)   d = 3 (SEG4)
 *     e = 4 (SEG5)   f = 1 (SEG2)   g = 7 (SEG8)   dp = 5 (SEG6)
 *
 *   Digitos del display de 3 cifras (izquierda -> derecha):
 *     digito 0 = GRID2 (addr 0x02)
 *     digito 1 = GRID1 (addr 0x00)
 *     digito 2 = GRID6 (addr 0x0A)
 *
 *   Los grids restantes manejan los LEDs indicadores de la placa:
 *     GRID3 (0x04), GRID4 (0x06), GRID5 (0x08)
 *
 * Los patrones de fuente que recibe y devuelve esta clase son LOGICOS
 * (bit0=a, bit1=b, bit2=c, bit3=d, bit4=e, bit5=f, bit6=g, bit7=dp).
 * La traduccion a los bits fisicos del chip la hace la clase.
 */

#ifndef HF8890DT_H
#define HF8890DT_H

#include "ET1620.h"

// Patrones logicos utiles, por si quieres componerlos a mano
#define SEG_A  0x01
#define SEG_B  0x02
#define SEG_C  0x04
#define SEG_D  0x08
#define SEG_E  0x10
#define SEG_F  0x20
#define SEG_G  0x40
#define SEG_DP 0x80

class HF8890DT : public ET1620 {
public:
  static const uint8_t DIGITS = 3;

  HF8890DT(uint8_t pinDin, uint8_t pinClk, uint8_t pinStb);

  void begin();

  // --- Numeros -------------------------------------------------------------
  // Rango util: -99 a 999. Devuelve false si no cabe (muestra "---").
  bool print(int value);

  // Numero con decimales, ej: print(3.14f, 2) -> "3.14" (usa el punto del
  // digito correspondiente). Devuelve false si no cabe.
  bool print(float value, uint8_t decimals);

  // --- Texto ---------------------------------------------------------------
  // Hasta 3 caracteres. Un '.' despues de un caracter enciende su punto
  // decimal en vez de ocupar un digito: print("1.23") usa los 3 digitos.
  // Soporta 0-9, A-Z (los que se pueden representar), espacio, '-', '_', '='
  // y el grado ('*' o '~').
  bool print(const char *text);

  // --- Control por digito --------------------------------------------------
  // digit: 0 = izquierda, 2 = derecha. pattern en bits logicos.
  void setDigit(uint8_t digit, uint8_t pattern);
  void setDigitChar(uint8_t digit, char c, bool dot = false);
  void setDecimalPoint(uint8_t digit, bool on);
  bool decimalPoint(uint8_t digit) const;
  void clearDigits();                    // borra los 3 digitos, no toca los LEDs

  // Ceros a la izquierda. Por defecto false: print(7) muestra "  7".
  void setLeadingZeros(bool enabled) { _leadingZeros = enabled; }

  // --- LEDs indicadores ----------------------------------------------------
  // grid: 3, 4 o 5. bit: 0..7. Escribir LEDs no altera el contenido del display.
  void setLed(uint8_t grid, uint8_t bit, bool on);
  bool led(uint8_t grid, uint8_t bit) const;
  void setLedGrid(uint8_t grid, uint8_t mask);
  void clearLeds();                      // apaga los 3 grids de LEDs

  // --- Utilidades ----------------------------------------------------------
  void testPattern();                    // enciende 8.8.8. y todos los LEDs

  // Traduce un patron logico (abcdefg.dp) a los bits fisicos del chip.
  static uint8_t toPhysical(uint8_t logicalPattern);
  // Traduce de vuelta: bits fisicos -> patron logico.
  static uint8_t toLogical(uint8_t physicalPattern);
  // Patron logico de un caracter. Devuelve 0x00 si no se puede representar.
  static uint8_t charToPattern(char c);

  // Numero de grid que usa cada digito (por si lo necesitas en crudo)
  static uint8_t digitGrid(uint8_t digit);

private:
  bool _dp[DIGITS];
  bool _leadingZeros;

  void applyDigit(uint8_t digit, uint8_t logicalPattern);
};

#endif // HF8890DT_H
