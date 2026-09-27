# ET1620

Librería Arduino para el LED driver **ET1620** (Etek Microelectronics) y para el panel de control táctil **HF-8890DT / CL-8890DT-CU-SEB**.

Son dos clases:

- **`ET1620`** — driver genérico del chip. Interfaz serial de 3 hilos, RAM de 12 bytes, modos 8×6 / 9×5 / 10×4, brillo y on/off. No sabe nada de tu placa.
- **`HF8890DT`** — hereda de la anterior y le agrega el mapeo físico del panel: display de 3 dígitos, puntos decimales y LEDs indicadores.

Si algún día usas el mismo chip en otro display, trabajas con `ET1620` directo y te ahorras el mapeo equivocado.

## Instalación

Copia la carpeta `ET1620/` completa dentro de tu carpeta de librerías de Arduino:

- Windows: `Documentos\Arduino\libraries\`
- macOS: `~/Documents/Arduino/libraries/`
- Linux: `~/Arduino/libraries/`

Reinicia el IDE. Aparece en *Archivo → Ejemplos → ET1620*.

## Conexiones

| Leonardo | ET1620 | |
|---|---|---|
| D8 | pin 18 | DIN — con pull-up de 4.7k a VDD |
| D9 | pin 19 | CLK |
| D10 | pin 20 | STB |
| GND | pin 12 / 15 | GND común, obligatorio |

El datasheet recomienda 100 Ω en serie en las tres líneas y 100 pF a GND cerca del chip. VDD entre 3.0 y 5.5 V. Si el panel corre a 3.3 V y el micro a 5 V, pon level shifter.

## Uso mínimo

```cpp
#include <HF8890DT.h>

HF8890DT display(8, 9, 10);   // DIN, CLK, STB

void setup() {
  display.begin();
  display.setBrightness(7);
  display.print(123);
}

void loop() {}
```

## API — clase `HF8890DT`

### Números y texto

```cpp
display.print(42);            // "  42" -> en 3 dígitos: " 42"
display.print(-55);           // "-55"   (rango -99 a 999)
display.print(3.14f, 2);      // "3.14"
display.print("HOL");         // texto, hasta 3 caracteres
display.print("1.23");        // el punto no consume un dígito
display.setLeadingZeros(true);// print(7) -> "007"
```

`print()` devuelve `false` si el valor no cabe, y en ese caso muestra `---`.

Caracteres soportados: `0-9`, las letras representables en 7 segmentos (`A b C d E F G H I J L n O P q r S t U y Z`), espacio, `-`, `_`, `=`, `'`, `"`, paréntesis y `*` / `~` para el símbolo de grado.

### Control por dígito

```cpp
display.setDigit(0, SEG_A | SEG_D | SEG_G);   // patrón a mano, dígito izquierdo
display.setDigitChar(1, 'E');
display.setDecimalPoint(2, true);
display.clearDigits();                         // borra dígitos sin tocar los LEDs
```

Los dígitos van de **0 (izquierda) a 2 (derecha)**.

### LEDs indicadores

Los LEDs de la placa cuelgan de los grids que el display no usa:

```cpp
display.setLed(3, 0, true);     // grid 3, bit 0
display.setLedGrid(4, 0xFF);    // todo el grid 4
display.clearLeds();
```

Escribir LEDs no altera el número en pantalla y viceversa. El ejemplo `04_LedsIndicadores` los recorre uno a uno para que identifiques cuál es cuál.

### Brillo y encendido

```cpp
display.setBrightness(0..7);   // duty de 1/16 a 14/16
display.displayOff();          // apaga sin borrar la RAM — ideal para parpadeo
display.displayOn();
```

## API — clase `ET1620`

```cpp
ET1620 chip(8, 9, 10);
chip.begin(ET1620::MODE_8SEG_6GRID);

chip.writeGrid(1, 0xFF);           // grid 1..6, máscara de SEG1..SEG8
chip.writeGridBit(2, 5, true);
chip.writeRam(0x0A, 0x3F);         // acceso crudo por dirección
chip.writeBit(0x00, 3, true);
chip.clear();
chip.fill(0xFF);
```

### Agrupar escrituras

Cada escritura hace un flush completo de los 12 bytes al chip. Si vas a cambiar varias cosas, agrúpalas:

```cpp
chip.setAutoUpdate(false);
// ...muchos writeGrid / writeBit...
chip.setAutoUpdate(true);   // manda todo de una sola vez
```

Importa para animaciones: sin agrupar, una animación de 6 pasos hace 6 transmisiones seriales completas.

## El mapeo del HF-8890DT

Determinado por escaneo bit a bit sobre la placa real, encendiendo un bit de RAM a la vez:

**Segmentos** (bit dentro del byte del grid):

| | a | b | c | d | e | f | g | dp |
|---|---|---|---|---|---|---|---|---|
| bit | 0 | 2 | 6 | 3 | 4 | 1 | 7 | 5 |
| pin | SEG1 | SEG3 | SEG7 | SEG4 | SEG5 | SEG2 | SEG8 | SEG6 |

**Dígitos y LEDs:**

| Elemento | Grid | Dirección |
|---|---|---|
| Dígito izquierdo | GRID2 | `0x02` |
| Dígito central | GRID1 | `0x00` |
| Dígito derecho | GRID6 | `0x0A` |
| LEDs indicadores | GRID3 / GRID4 / GRID5 | `0x04` / `0x06` / `0x08` |

El panel opera en modo **8 SEG × 6 GRID**, que es el default del chip al encender.

## Notas del datasheet

- La RAM del chip es **indefinida al encender**. `begin()` la limpia; no te saltes ese paso.
- En modo 8×6 solo cuentan las direcciones pares. Las impares corresponden a SEG9–SEG14, y SEG13/SEG14 están reasignados como GRID6/GRID5.
- Los tiempos mínimos (CLK ≥ 400 ns, setup/hold ≥ 100 ns, STB ≥ 1 µs) se cumplen con margen amplio; la librería usa 2 µs.
- El protocolo es **LSB primero**.

## Ejemplos incluidos

| | |
|---|---|
| `01_HolaMundo` | lo mínimo para ver algo |
| `02_Contador` | conteo, negativos, decimales, brillo, parpadeo |
| `03_Escaner` | herramienta de mapeo bit a bit por Serial |
| `04_LedsIndicadores` | recorre los LEDs para identificarlos |
| `05_DriverCrudo` | clase genérica, animaciones sobre la RAM |

## Compatibilidad

Probada la lógica en host; el código solo usa `pinMode` / `digitalWrite` / `delayMicroseconds`, así que corre en AVR (Uno, Leonardo, Mega) y ESP32 sin cambios. Para ESP32 basta cambiar los pines del constructor.
