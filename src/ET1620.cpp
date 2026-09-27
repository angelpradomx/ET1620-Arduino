#include "ET1620.h"

// Definicion fuera de clase (C++11 la pide si la constante se usa por referencia)
const uint8_t ET1620::RAM_SIZE;

/* Comandos del ET1620 (datasheet pag. 3-4) */
#define CMD_DATA_WRITE_AUTOINC 0x40   // Command 2: normal, auto-incremento, escritura
#define CMD_ADDR(a)            (0xC0 | ((a) & 0x0F))          // Command 3
#define CMD_DISPLAY(on, duty)  (0x80 | ((on) ? 0x08 : 0x00) | ((duty) & 0x07))  // Command 4

ET1620::ET1620(uint8_t pinDin, uint8_t pinClk, uint8_t pinStb)
  : _din(pinDin), _clk(pinClk), _stb(pinStb),
    _brightness(7), _on(true), _autoUpdate(true) {
  for (uint8_t i = 0; i < RAM_SIZE; i++) _ram[i] = 0;
}

/* ------------------------------------------------------------------ */
/* Capa fisica: bit-bang de 3 hilos.                                   */
/* Tiempos minimos (datasheet pag. 7): PWclk >= 400ns, tsetup >= 100ns, */
/* thold >= 100ns, tclk-stb >= 1us, PWstb >= 1us. Los delays de 2us     */
/* dejan margen de sobra en cualquier AVR o ESP32.                      */
/* ------------------------------------------------------------------ */

void ET1620::strobeStart() {
  digitalWrite(_clk, HIGH);
  digitalWrite(_stb, LOW);
  delayMicroseconds(2);
}

void ET1620::strobeStop() {
  digitalWrite(_clk, HIGH);
  delayMicroseconds(2);
  digitalWrite(_stb, HIGH);
  delayMicroseconds(2);
}

void ET1620::writeByte(uint8_t b) {
  // LSB primero: el diagrama de comunicacion serial manda b0 antes que b7.
  for (uint8_t i = 0; i < 8; i++) {
    digitalWrite(_clk, LOW);
    digitalWrite(_din, (b & 0x01) ? HIGH : LOW);
    delayMicroseconds(2);          // tsetup
    digitalWrite(_clk, HIGH);      // el chip latchea en el flanco de subida
    delayMicroseconds(2);          // thold / PWclk
    b >>= 1;
  }
}

void ET1620::command(uint8_t cmd) {
  strobeStart();
  writeByte(cmd);
  strobeStop();
}

/* ------------------------------------------------------------------ */

void ET1620::begin(Mode mode) {
  pinMode(_din, OUTPUT);
  pinMode(_clk, OUTPUT);
  pinMode(_stb, OUTPUT);
  digitalWrite(_stb, HIGH);
  digitalWrite(_clk, HIGH);
  digitalWrite(_din, LOW);

  delay(200);                      // el flowchart del datasheet pide 200 ms
  command((uint8_t)mode);          // Command 1

  for (uint8_t i = 0; i < RAM_SIZE; i++) _ram[i] = 0;
  update();                        // la RAM al encender es indefinida: limpiarla
}

void ET1620::setMode(Mode mode) {
  command((uint8_t)mode);
  update();
}

void ET1620::update() {
  command(CMD_DATA_WRITE_AUTOINC);          // Command 2
  strobeStart();
  writeByte(CMD_ADDR(0x00));                // Command 3: arrancar en 0x00
  for (uint8_t i = 0; i < RAM_SIZE; i++) {  // DATA1..DATAn (max 12 bytes)
    writeByte(_ram[i]);
  }
  strobeStop();
  command(CMD_DISPLAY(_on, _brightness));   // Command 4
}

void ET1620::maybeUpdate() {
  if (_autoUpdate) update();
}

void ET1620::setAutoUpdate(bool enabled) {
  _autoUpdate = enabled;
  if (enabled) update();
}

/* ------------------------------------------------------------------ */

void ET1620::setBrightness(uint8_t level) {
  if (level > 7) level = 7;
  _brightness = level;
  command(CMD_DISPLAY(_on, _brightness));
}

void ET1620::displayOn() {
  _on = true;
  command(CMD_DISPLAY(true, _brightness));
}

void ET1620::displayOff() {
  _on = false;
  command(CMD_DISPLAY(false, _brightness));
}

/* ------------------------------------------------------------------ */

uint8_t ET1620::gridToAddr(uint8_t grid) {
  if (grid < 1 || grid > 6) return 0xFF;
  return (uint8_t)((grid - 1) * 2);
}

void ET1620::writeRam(uint8_t addr, uint8_t value) {
  if (addr >= RAM_SIZE) return;
  _ram[addr] = value;
  maybeUpdate();
}

uint8_t ET1620::readRam(uint8_t addr) const {
  return (addr < RAM_SIZE) ? _ram[addr] : 0;
}

void ET1620::writeBit(uint8_t addr, uint8_t bit, bool on) {
  if (addr >= RAM_SIZE || bit > 7) return;
  if (on) _ram[addr] |=  (uint8_t)(1 << bit);
  else    _ram[addr] &= (uint8_t)~(1 << bit);
  maybeUpdate();
}

bool ET1620::readBit(uint8_t addr, uint8_t bit) const {
  if (addr >= RAM_SIZE || bit > 7) return false;
  return (_ram[addr] & (1 << bit)) != 0;
}

void ET1620::writeGrid(uint8_t grid, uint8_t segments) {
  uint8_t a = gridToAddr(grid);
  if (a == 0xFF) return;
  writeRam(a, segments);
}

uint8_t ET1620::readGrid(uint8_t grid) const {
  uint8_t a = gridToAddr(grid);
  return (a == 0xFF) ? 0 : _ram[a];
}

void ET1620::writeGridBit(uint8_t grid, uint8_t bit, bool on) {
  uint8_t a = gridToAddr(grid);
  if (a == 0xFF) return;
  writeBit(a, bit, on);
}

void ET1620::clear() {
  for (uint8_t i = 0; i < RAM_SIZE; i++) _ram[i] = 0;
  update();
}

void ET1620::fill(uint8_t value) {
  for (uint8_t i = 0; i < RAM_SIZE; i++) _ram[i] = value;
  update();
}
