/*
 * ET1620.h - Driver generico para el LED driver ET1620 (Etek Microelectronics)
 *
 * Interfaz serial de 3 hilos (DIN / CLK / STB), LSB primero.
 * Hasta 8 SEG x 6 GRID (tambien 9x5 y 10x4). RAM de display de 12 bytes.
 *
 * Basado en el datasheet ET1620 Rev 1.3.
 *
 * Esta clase es agnostica de la placa: solo habla con el chip. Para el panel
 * HF-8890DT usa la clase HF8890DT (HF8890DT.h), que ya trae el mapeo fisico.
 */

#ifndef ET1620_H
#define ET1620_H

#include <Arduino.h>

class ET1620 {
public:
  // Modos de display (Command 1). El chip arranca en 8x6 por defecto.
  enum Mode : uint8_t {
    MODE_10SEG_4GRID = 0x00,
    MODE_9SEG_5GRID  = 0x01,
    MODE_8SEG_6GRID  = 0x02
  };

  // Brillo (Command 4). El nombre indica el duty cycle real.
  enum Brightness : uint8_t {
    DUTY_1_16  = 0, DUTY_2_16  = 1, DUTY_4_16  = 2, DUTY_10_16 = 3,
    DUTY_11_16 = 4, DUTY_12_16 = 5, DUTY_13_16 = 6, DUTY_14_16 = 7
  };

  static const uint8_t RAM_SIZE = 12;

  ET1620(uint8_t pinDin, uint8_t pinClk, uint8_t pinStb);

  // Configura pines, espera el arranque del chip, fija el modo y limpia la RAM
  // (el datasheet advierte que su contenido es indefinido al encender).
  void begin(Mode mode = MODE_8SEG_6GRID);

  // --- Control del display -------------------------------------------------
  void setBrightness(uint8_t level);   // 0..7
  uint8_t brightness() const { return _brightness; }
  void displayOn();
  void displayOff();
  bool isOn() const { return _on; }
  void setMode(Mode mode);

  // --- Acceso a la RAM -----------------------------------------------------
  // Direcciones 0x00..0x0B. En modo 8x6: direccion par = GRIDn (n = addr/2 + 1),
  // bits 0..7 = SEG1..SEG8. Las impares (SEG9..SEG14) no aplican en ese modo.
  void writeRam(uint8_t addr, uint8_t value);
  uint8_t readRam(uint8_t addr) const;
  void writeBit(uint8_t addr, uint8_t bit, bool on);
  bool readBit(uint8_t addr, uint8_t bit) const;

  // Igual que arriba pero indexado por numero de grid (1..6).
  void writeGrid(uint8_t grid, uint8_t segments);
  uint8_t readGrid(uint8_t grid) const;
  void writeGridBit(uint8_t grid, uint8_t bit, bool on);

  void clear();
  void fill(uint8_t value = 0xFF);

  // --- Control de refresco -------------------------------------------------
  // Por defecto cada escritura hace flush al chip. Apagalo para agrupar
  // muchos cambios y mandarlos de una sola vez con update().
  void setAutoUpdate(bool enabled);
  bool autoUpdate() const { return _autoUpdate; }
  void update();   // manda los 12 bytes de RAM + el comando de display

  // --- Bajo nivel ----------------------------------------------------------
  // Manda un comando suelto en su propia trama STB. Util para experimentar.
  void command(uint8_t cmd);

  // Direccion de RAM correspondiente a un grid (1..6). Devuelve 0xFF si invalido.
  static uint8_t gridToAddr(uint8_t grid);

protected:
  uint8_t _ram[RAM_SIZE];

private:
  uint8_t _din, _clk, _stb;
  uint8_t _brightness;
  bool    _on;
  bool    _autoUpdate;

  void    strobeStart();
  void    strobeStop();
  void    writeByte(uint8_t b);
  void    maybeUpdate();
};

#endif // ET1620_H
