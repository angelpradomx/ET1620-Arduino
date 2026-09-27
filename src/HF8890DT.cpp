#include "HF8890DT.h"

// Definicion fuera de clase (C++11 la pide si la constante se usa por referencia)
const uint8_t HF8890DT::DIGITS;

/* ------------------------------------------------------------------ */
/* Mapeo fisico determinado por escaneo bit a bit sobre la placa real  */
/* ------------------------------------------------------------------ */

// Indice = segmento logico (a,b,c,d,e,f,g,dp), valor = bit fisico en la RAM.
static const uint8_t SEG_BIT[8] = { 0, 2, 6, 3, 4, 1, 7, 5 };

// Digito 0 (izq), 1 (centro), 2 (der) -> numero de grid.
static const uint8_t DIGIT_GRID[HF8890DT::DIGITS] = { 2, 1, 6 };

/* ------------------------------------------------------------------ */
/* Fuente de 7 segmentos, en bits logicos                              */
/* ------------------------------------------------------------------ */

static const uint8_t FONT_DIGIT[10] = {
  0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F
};

// A..Z. Las letras que no se pueden representar en 7 segmentos
// (K, M, V, W, X) quedan en 0x00.
static const uint8_t FONT_ALPHA[26] = {
  /* A */ 0x77, /* b */ 0x7C, /* C */ 0x39, /* d */ 0x5E, /* E */ 0x79,
  /* F */ 0x71, /* G */ 0x3D, /* H */ 0x76, /* I */ 0x06, /* J */ 0x1E,
  /* K */ 0x00, /* L */ 0x38, /* M */ 0x00, /* n */ 0x54, /* O */ 0x3F,
  /* P */ 0x73, /* q */ 0x67, /* r */ 0x50, /* S */ 0x6D, /* t */ 0x78,
  /* U */ 0x3E, /* V */ 0x00, /* W */ 0x00, /* X */ 0x00, /* y */ 0x6E,
  /* Z */ 0x5B
};

uint8_t HF8890DT::charToPattern(char c) {
  if (c >= '0' && c <= '9') return FONT_DIGIT[c - '0'];
  if (c >= 'A' && c <= 'Z') return FONT_ALPHA[c - 'A'];
  if (c >= 'a' && c <= 'z') return FONT_ALPHA[c - 'a'];
  switch (c) {
    case ' ': return 0x00;
    case '-': return SEG_G;
    case '_': return SEG_D;
    case '=': return SEG_G | SEG_D;
    case '\'': return SEG_F;
    case '"': return SEG_F | SEG_B;
    case '*':
    case '~': return SEG_A | SEG_B | SEG_F | SEG_G;   // simbolo de grado
    case '?': return SEG_A | SEG_B | SEG_E | SEG_G;
    case '[':
    case '(': return SEG_A | SEG_F | SEG_E | SEG_D;
    case ']':
    case ')': return SEG_A | SEG_B | SEG_C | SEG_D;
    default:  return 0x00;
  }
}

/* ------------------------------------------------------------------ */
/* Traduccion logico <-> fisico                                        */
/* ------------------------------------------------------------------ */

uint8_t HF8890DT::toPhysical(uint8_t logicalPattern) {
  uint8_t out = 0;
  for (uint8_t s = 0; s < 8; s++) {
    if (logicalPattern & (uint8_t)(1 << s)) out |= (uint8_t)(1 << SEG_BIT[s]);
  }
  return out;
}

uint8_t HF8890DT::toLogical(uint8_t physicalPattern) {
  uint8_t out = 0;
  for (uint8_t s = 0; s < 8; s++) {
    if (physicalPattern & (uint8_t)(1 << SEG_BIT[s])) out |= (uint8_t)(1 << s);
  }
  return out;
}

uint8_t HF8890DT::digitGrid(uint8_t digit) {
  return (digit < DIGITS) ? DIGIT_GRID[digit] : 0xFF;
}

/* ------------------------------------------------------------------ */

HF8890DT::HF8890DT(uint8_t pinDin, uint8_t pinClk, uint8_t pinStb)
  : ET1620(pinDin, pinClk, pinStb), _leadingZeros(false) {
  for (uint8_t i = 0; i < DIGITS; i++) _dp[i] = false;
}

void HF8890DT::begin() {
  for (uint8_t i = 0; i < DIGITS; i++) _dp[i] = false;
  ET1620::begin(MODE_8SEG_6GRID);   // el panel usa 8 segmentos x 6 grids
}

/* ------------------------------------------------------------------ */
/* Digitos                                                             */
/* ------------------------------------------------------------------ */

void HF8890DT::applyDigit(uint8_t digit, uint8_t logicalPattern) {
  if (digit >= DIGITS) return;
  if (_dp[digit]) logicalPattern |= SEG_DP;
  writeGrid(DIGIT_GRID[digit], toPhysical(logicalPattern));
}

void HF8890DT::setDigit(uint8_t digit, uint8_t pattern) {
  if (digit >= DIGITS) return;
  if (pattern & SEG_DP) _dp[digit] = true;
  applyDigit(digit, pattern);
}

void HF8890DT::setDigitChar(uint8_t digit, char c, bool dot) {
  if (digit >= DIGITS) return;
  _dp[digit] = dot;
  applyDigit(digit, charToPattern(c));
}

void HF8890DT::setDecimalPoint(uint8_t digit, bool on) {
  if (digit >= DIGITS) return;
  _dp[digit] = on;
  writeGridBit(DIGIT_GRID[digit], SEG_BIT[7], on);
}

bool HF8890DT::decimalPoint(uint8_t digit) const {
  return (digit < DIGITS) ? _dp[digit] : false;
}

void HF8890DT::clearDigits() {
  bool prev = autoUpdate();
  setAutoUpdate(false);
  for (uint8_t i = 0; i < DIGITS; i++) {
    _dp[i] = false;
    writeGrid(DIGIT_GRID[i], 0x00);
  }
  setAutoUpdate(prev);
  if (!prev) update();
}

/* ------------------------------------------------------------------ */
/* print()                                                             */
/* ------------------------------------------------------------------ */

bool HF8890DT::print(int value) {
  bool neg = value < 0;
  long v = neg ? -(long)value : (long)value;

  if ((neg && v > 99) || (!neg && v > 999)) {
    bool prev = autoUpdate();
    setAutoUpdate(false);
    for (uint8_t i = 0; i < DIGITS; i++) { _dp[i] = false; applyDigit(i, SEG_G); }
    setAutoUpdate(prev);
    if (!prev) update();
    return false;
  }

  uint8_t d[DIGITS] = { (uint8_t)(v / 100), (uint8_t)((v / 10) % 10), (uint8_t)(v % 10) };

  bool prev = autoUpdate();
  setAutoUpdate(false);

  bool blanking = !_leadingZeros;
  int8_t minusAt = -1;

  for (uint8_t i = 0; i < DIGITS; i++) {
    uint8_t pat;
    if (blanking && d[i] == 0 && i < DIGITS - 1) {
      pat = 0x00;
      minusAt = (int8_t)i;          // el menos va justo antes del primer digito
    } else {
      blanking = false;
      pat = FONT_DIGIT[d[i]];
    }
    applyDigit(i, pat);
  }

  if (neg) {
    uint8_t pos = (minusAt >= 0) ? (uint8_t)minusAt : 0;
    applyDigit(pos, SEG_G);
  }

  setAutoUpdate(prev);
  if (!prev) update();
  return true;
}

bool HF8890DT::print(float value, uint8_t decimals) {
  if (decimals > 2) decimals = 2;

  long scale = 1;
  for (uint8_t i = 0; i < decimals; i++) scale *= 10;

  bool neg = value < 0;
  float av = neg ? -value : value;
  long scaled = (long)(av * scale + 0.5f);

  uint8_t maxDigits = neg ? (DIGITS - 1) : DIGITS;
  long limit = 1;
  for (uint8_t i = 0; i < maxDigits; i++) limit *= 10;

  if (scaled >= limit) {
    bool prev = autoUpdate();
    setAutoUpdate(false);
    for (uint8_t i = 0; i < DIGITS; i++) { _dp[i] = false; applyDigit(i, SEG_G); }
    setAutoUpdate(prev);
    if (!prev) update();
    return false;
  }

  bool prev = autoUpdate();
  setAutoUpdate(false);

  // Renderiza de derecha a izquierda
  uint8_t patterns[DIGITS];
  bool    dots[DIGITS];
  for (uint8_t i = 0; i < DIGITS; i++) { patterns[i] = 0x00; dots[i] = false; }

  long rem = scaled;
  int8_t pos = (int8_t)(DIGITS - 1);
  uint8_t written = 0;

  // Escribe al menos decimals+1 cifras, para que siempre haya parte entera.
  while (pos >= 0 && (rem > 0 || written <= decimals)) {
    patterns[pos] = FONT_DIGIT[rem % 10];
    rem /= 10;
    written++;
    pos--;
  }

  if (decimals > 0) {
    int8_t dotDigit = (int8_t)(DIGITS - 1 - decimals);
    if (dotDigit >= 0) dots[dotDigit] = true;
  }

  if (neg && pos >= 0) patterns[pos] = SEG_G;

  for (uint8_t i = 0; i < DIGITS; i++) {
    _dp[i] = dots[i];
    applyDigit(i, patterns[i]);
  }

  setAutoUpdate(prev);
  if (!prev) update();
  return true;
}

bool HF8890DT::print(const char *text) {
  if (!text) return false;

  uint8_t patterns[DIGITS];
  bool    dots[DIGITS];
  for (uint8_t i = 0; i < DIGITS; i++) { patterns[i] = 0x00; dots[i] = false; }

  uint8_t slot = 0;
  bool overflow = false;

  for (const char *p = text; *p; p++) {
    if (*p == '.') {
      // Un punto pegado al caracter anterior enciende su dp en vez de gastar slot
      if (slot > 0 && !dots[slot - 1]) { dots[slot - 1] = true; continue; }
    }
    if (slot >= DIGITS) { overflow = true; break; }
    patterns[slot] = charToPattern(*p);
    slot++;
  }

  bool prev = autoUpdate();
  setAutoUpdate(false);
  for (uint8_t i = 0; i < DIGITS; i++) {
    _dp[i] = dots[i];
    applyDigit(i, patterns[i]);
  }
  setAutoUpdate(prev);
  if (!prev) update();

  return !overflow;
}

/* ------------------------------------------------------------------ */
/* LEDs indicadores (grids 3, 4 y 5)                                   */
/* ------------------------------------------------------------------ */

void HF8890DT::setLed(uint8_t grid, uint8_t bit, bool on) {
  if (grid < 3 || grid > 5) return;
  writeGridBit(grid, bit, on);
}

bool HF8890DT::led(uint8_t grid, uint8_t bit) const {
  if (grid < 3 || grid > 5 || bit > 7) return false;
  return (readGrid(grid) & (1 << bit)) != 0;
}

void HF8890DT::setLedGrid(uint8_t grid, uint8_t mask) {
  if (grid < 3 || grid > 5) return;
  writeGrid(grid, mask);
}

void HF8890DT::clearLeds() {
  bool prev = autoUpdate();
  setAutoUpdate(false);
  for (uint8_t g = 3; g <= 5; g++) writeGrid(g, 0x00);
  setAutoUpdate(prev);
  if (!prev) update();
}

/* ------------------------------------------------------------------ */

void HF8890DT::testPattern() {
  bool prev = autoUpdate();
  setAutoUpdate(false);
  for (uint8_t i = 0; i < DIGITS; i++) { _dp[i] = true; applyDigit(i, 0x7F); }
  for (uint8_t g = 3; g <= 5; g++) writeGrid(g, 0xFF);
  setAutoUpdate(prev);
  if (!prev) update();
}
