
/************************************************
  ControlArduino4.ino

  Universidad del Norte Santo Tomás de Aquino
  Facultad de Ingeniería
  Laboratorio de Robótica
  Trabajo de Campo: EBRA
  Año académico 2017
  Autores: Fernández Pablo Daniel, Soraire Lourdes Sofía
  Director: Sbrugnera G.

  Programa de control de Arduino MEGA
  Controla todos los dispositivos conectados al Arduino

  Versión 2018-03-14

************************************************/

#include <LiquidCrystal.h>
#include <PS2Keyboard.h>
#include <DFRobotDFPlayerMini.h>


// Constantes generales *******************************************
#define   STEPS_A   20
#define   STEPS_B   180
#define   STEPS_E   35
#define   STEPS_F   360
#define   STEPS_START   100

/*
  Constantes de símbolos Braille de las letras "a" hasta "z"
*/
int g_codigosLetras[] = {1, 12, 14, 145, 15, 124, 1245, 125, 24, 245, 13, 123, 134, 1345, 135, 1234, 12345, 1235, 234, 2345, 136, 1236, 2456, 1346, 12456, 1356};
/*
  Constantes de símbolos Braille de los dígitos "0" hasta "9"
*/
int g_codigosDigitos[] = {1, 12, 14, 145, 15, 124, 1245, 125, 24, 245};

/*
  Constantes de archivos .MP3
*/
#define  MP3_IMPRIMIR  1
#define  MP3_ERROR     4
#define  MP3_BORRAR   56
#define  MP3_CANCELAR   2
#define  MP3_SIG_LINEA  55
#define  MP3_A        29
#define  MP3_CERO     15
#define  MP3_ESPACIO  3
#define  MP3_COMILLA  5
#define  MP3_ARROBA 27
#define  MP3_NUMERAL  6
#define  MP3_PESOS  7
#define  MP3_AMPERSAND  8
#define  MP3_BARRA  14
#define  MP3_ABRE_PAR 9
#define  MP3_CIERRA_PAR 10
#define  MP3_PUNTO  13
#define  MP3_COMA 11
#define  MP3_MENOS  12
#define  MP3_DOS_PUNTOS 25
#define  MP3_GUION  28
#define  MP3_IGUAL  26


// Constantes de definición del hardware **************************

// Teclado
#define PIN_KEYBOARD_DATA  3
#define PIN_KEYBOARD_CLOCK 2

// Display
#define PIN_DISPLAY_D4    4
#define PIN_DISPLAY_D5    5
#define PIN_DISPLAY_D6    6
#define PIN_DISPLAY_D7    7
#define PIN_DISPLAY_RS    16
#define PIN_DISPLAY_EN    17

// Motor Y (cabezal)
#define PIN_MOTORY_1    30
#define PIN_MOTORY_2    32
#define PIN_MOTORY_3    34
#define PIN_MOTORY_4    36

// Motor X (cinta)
#define PIN_MOTORX_1    40
#define PIN_MOTORX_2    42
#define PIN_MOTORX_3    44
#define PIN_MOTORX_4    46

// Sensor cinta 1 (conector G)
#define PIN_CINTA_1     18

// Sensor cinta 2 (conector H)
#define PIN_CINTA_2     19

// Sensor fin de carrera (conector E)
#define PIN_FIN_CARRERA     20

// Driver electroimán (conector F)
#define PIN_ELECTROIMAN     21


// Variables de estado ************************************
/*
  Momento del loop actual, en milisegundos
*/
unsigned long g_momentoActual = 0;
/*
  Buffer actual de la línea 1 del display
*/
String g_displayLine1;
/*
  Buffer actual de la línea 1 del display
*/
String g_displayLine2;
/*
  Columna actual del display
*/
int g_myColumnCursor = 0;
/*
  Línea actual del display: 1 o 2
*/
int g_currentDisplayLine = 1;
/*
  Línea actual del buffer: 1, 2 o 3
*/
int g_currentBufferLine = 1;

/*
  Fase actual del motor cinta (X)
*/
int g_currentStepX = 0;

/*
  Fase actual del motor cabezal (Y)
*/
int g_currentStepY = 0;


/*
  Buffers que contienen las tres líneas a imprimir, en ASCII
  Cadenas de hasta 16 caracteres, terminadas en \0
*/
char g_bufferAscii1[17];
char g_bufferAscii2[17];
char g_bufferAscii3[17];

/*
  Buffers que contienen los caracteres Braille que hay que
  imprimir en cada línea. Por ejemplo, "145" significa
  imprimir los puntos 1, 4 y 5.
*/
int g_bufferBraille1[32];
int g_bufferBraille2[32];
int g_bufferBraille3[32];

/*
  Longitudes de los buffers Braille
*/
int g_bufferLen1;
int g_bufferLen2;
int g_bufferLen3;

/*
  Flag para detener la impresión
*/
boolean g_cancelPrint = false;

String g_charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789\"@#$&/().,-:_= ";


// Objetos de acceso al hardware ********************************

// DFPlayer
#define g_dfPlayerSerial    Serial3

DFRobotDFPlayerMini g_dfplayer;

// Display
LiquidCrystal g_lcd(PIN_DISPLAY_RS, PIN_DISPLAY_EN,
                    PIN_DISPLAY_D4, PIN_DISPLAY_D5,
                    PIN_DISPLAY_D6, PIN_DISPLAY_D7);

PS2Keyboard g_keyboard;


// Primitivas de control de hardware ************************

void mp3reproduce(int p_char) {
  if (p_char >= 'A' && p_char <= 'Z') {
    int l_orden = p_char - 'A' + MP3_A;
    g_dfplayer.play(l_orden);
    return;
  } // end if
  if (p_char >= '0' && p_char <= '9') {
    int l_orden = p_char - '0' + MP3_CERO;
    g_dfplayer.play(l_orden);
    return;
  } // end if

  // Imprimibles especiales
  switch (p_char) {
    case ' ': g_dfplayer.play(MP3_ESPACIO); break;
    case '"': g_dfplayer.play(MP3_COMILLA); break;
    case '@': g_dfplayer.play(MP3_ARROBA); break; // !!
    case '#': g_dfplayer.play(MP3_NUMERAL); break; // !!
    case '$': g_dfplayer.play(MP3_PESOS); break;
    case '&': g_dfplayer.play(MP3_AMPERSAND); break;
    case '/': g_dfplayer.play(MP3_BARRA); break;
    case '(': g_dfplayer.play(MP3_ABRE_PAR); break;
    case ')': g_dfplayer.play(MP3_CIERRA_PAR); break;
    case '.': g_dfplayer.play(MP3_PUNTO); break;
    case ',': g_dfplayer.play(MP3_COMA); break;
    case '-': g_dfplayer.play(MP3_MENOS); break;
    case ':': g_dfplayer.play(MP3_DOS_PUNTOS); break;
    case '_': g_dfplayer.play(MP3_GUION); break;
    case '=': g_dfplayer.play(MP3_IGUAL); break;
    default: g_dfplayer.play(MP3_ERROR);
  } // end switch
}


void offMotorX() {
  digitalWrite(PIN_MOTORX_1, LOW);
  digitalWrite(PIN_MOTORX_2, LOW);
  digitalWrite(PIN_MOTORX_3, LOW);
  digitalWrite(PIN_MOTORX_4, LOW);
}

void offMotorY() {
  digitalWrite(PIN_MOTORY_1, LOW);
  digitalWrite(PIN_MOTORY_2, LOW);
  digitalWrite(PIN_MOTORY_3, LOW);
  digitalWrite(PIN_MOTORY_4, LOW);
}

/*
   Mueve un semipaso el motor X (cinta)
*/
void moveStepperX(int p_dir) {
  if (g_cancelPrint) {
    return;
  }
  offMotorX();
  offMotorY(); // impide el movimiento simultáneo
  if (p_dir == 1) {
    // Avanza un paso
    g_currentStepX++;
    if (g_currentStepX > 7) {
      g_currentStepX = 0;
    } // end if
  } else {
    // Retrocede un paso
    g_currentStepX--;
    if (g_currentStepX < 0) {
      g_currentStepX = 7;
    } // end if
  } // end if

  switch (g_currentStepX) {
    case 0: {
        digitalWrite(PIN_MOTORX_1, HIGH);
      } break;
    case 1: {
        digitalWrite(PIN_MOTORX_1, HIGH);
        digitalWrite(PIN_MOTORX_2, HIGH);
      } break;
    case 2: {
        digitalWrite(PIN_MOTORX_2, HIGH);
      } break;
    case 3: {
        digitalWrite(PIN_MOTORX_2, HIGH);
        digitalWrite(PIN_MOTORX_3, HIGH);
      } break;
    case 4: {
        digitalWrite(PIN_MOTORX_3, HIGH);
      } break;
    case 5: {
        digitalWrite(PIN_MOTORX_3, HIGH);
        digitalWrite(PIN_MOTORX_4, HIGH);
      } break;
    case 6: {
        digitalWrite(PIN_MOTORX_4, HIGH);
      } break;
    case 7: {
        digitalWrite(PIN_MOTORX_4, HIGH);
        digitalWrite(PIN_MOTORX_1, HIGH);
      } break;
  } // end switch
  delay(7);

}

/*
   Avanza la cinta p_steps pasos (eje X)
*/
void moveXForward(int p_steps) {
  while (p_steps-- > 0) {
    moveStepperX(1);
  } // end while
}

/*
   Retrocede la cinta p_steps pasos (eje X). No debería utilizarse,
   no se hace retroceder la cinta
*/
void moveXBackward(int p_steps) {
  while (p_steps-- > 0) {
    moveStepperX(-1);
  }
}

/*
   Mueve un semipaso el motor Y (cabezal)
   @p_dir: dirección del movimiento, 1 o -1.
*/
void moveStepperY(int p_dir) {
  if (g_cancelPrint) {
    return;
  }

  offMotorX(); // Impide el movimiento simultáneo
  offMotorY();
  if (p_dir == 1) {
    // Avanza un paso
    g_currentStepY++;
    if (g_currentStepY > 7) {
      g_currentStepY = 0;
    } // end if
  } else {
    // Retrocede un paso
    g_currentStepY--;
    if (g_currentStepY < 0) {
      g_currentStepY = 7;
    } // end if
  } // end if

  switch (g_currentStepY) {
    case 0: {
        digitalWrite(PIN_MOTORY_1, HIGH);
      } break;
    case 1: {
        digitalWrite(PIN_MOTORY_1, HIGH);
        digitalWrite(PIN_MOTORY_2, HIGH);
      } break;
    case 2: {
        digitalWrite(PIN_MOTORY_2, HIGH);
      } break;
    case 3: {
        digitalWrite(PIN_MOTORY_2, HIGH);
        digitalWrite(PIN_MOTORY_3, HIGH);
      } break;
    case 4: {
        digitalWrite(PIN_MOTORY_3, HIGH);
      } break;
    case 5: {
        digitalWrite(PIN_MOTORY_3, HIGH);
        digitalWrite(PIN_MOTORY_4, HIGH);
      } break;
    case 6: {
        digitalWrite(PIN_MOTORY_4, HIGH);
      } break;
    case 7: {
        digitalWrite(PIN_MOTORY_4, HIGH);
        digitalWrite(PIN_MOTORY_1, HIGH);
      } break;
  } // end switch
  delay(3); // Con 3 a 5 funciona bien; con 2, no

}

/*
  Mueve el cabezal hacia el fin de carrera
*/
void moveYForward(int p_steps) {
  while (p_steps-- > 0) {
    moveStepperY(1);
  } // end while
}

/*
   Aleja el cabezal del fin de carrera
*/
void moveYBackward(int p_steps) {
  while (p_steps-- > 0) {
    moveStepperY(-1);
  }
}

/*
   Mueve la cinta la distancia A: entre dos columnas del mismo caracter
*/
void moveA() {
  moveXForward(STEPS_A);
}

/*
   Mueve avanzando el cabezal la distancia B: entre dos filas del mismo caracter
*/
void moveB() {
  moveYBackward(STEPS_B);
}

/*
   Mueve retrocediendo el cabezal la distancia B: entre dos filas del mismo caracter
*/
void move_B() {
  moveYForward(STEPS_B);
}

/*
   Mueve la cinta la distancia E = C - A: entre la segunda columna de un
   caracter, y la primera columna del siguiente
*/
void moveE() {
  moveXForward(STEPS_E);
}

/*
   Mueve avanzando el cabezal (alejando el fin de carrera) la listancia F = D - 2B: entre la tercera fila de
   un caracter y la primera fila del caracter en la siguiente línea
*/
void moveF() {
  moveYBackward(STEPS_F);
}
/*
  Mueve retrocediendo el cabezal (acercando el fin de carrera) la listancia F = D - 2B: entre la tercera fila de
  un caracter y la primera fila del caracter en la siguiente línea
*/
void move_F() {
  moveYForward(STEPS_F);
}

/*
   Busca el final de carrera y mueve el cabezal hasta la fila del 1 de la línea 1,
   primera línea imprimible
*/
void moveToFirstPrintableLine() {
  if (cancelPrint()) {
    return;
  } // end if
  moveToEndHead();
  if (cancelPrint()) {
    return;
  } // end if
  moveYBackward(STEPS_START);
}


/*
   Activa el electroimán (punch) si el símbolo Braille  p_brailleSym contiene el punto p_dotIndex
*/
void punchIfDotted(int p_brailleSym, int p_dotIndex)  {
  String l_strSym = String(p_brailleSym, DEC);
  char l_dotChar = '0' + p_dotIndex;

  int l_charIndex = l_strSym.indexOf(l_dotChar);
  if (l_charIndex < 0) {
    return;
  } // end if

  punch();
}

/*
   Avanza la cinta (X) hasta que los dos sensores S1 y S2 estén ocluidos
   Acepta la cinta para comenzar a imprimirla
*/
void tapeIn() {
  while (!isSensor1() || !isSensor2()) {
    if (cancelPrint()) {
      return;
    } // end if

    moveStepperX(1);
  } // end while

}

/*
  Avanza la cinta (X) hasta que los dos sensores estén desocluídos.
   Expulsa la cinta y apaga los motores.
*/
void tapeOut() {
  while (isSensor1() || isSensor2()) {
    if (cancelPrint()) {
      return;
    } // end if

    moveStepperX(1);
  } // end while

  // Expulsa completamente
  int l_counter = 400;
  while (l_counter--) {
    moveStepperX(1);
  } // end while

  offMotorX();
  offMotorY();

}

/*
  Mueve el cabezal hasta que encuentra el fin del recorrido
*/
void moveToEndHead() {
  while (!isEndHead()) {
    moveStepperY(1);
  } // end while
}
/*
   Imprime los seis puntos de los tres caracteres Braille indicados.
   Parte desde el punto 1 de la línea 1 de la columna a imprimir.
   Termina en el punto 1 de la línea 1 de la columna siguiente.
*/
void printThreeSymbols(int p_braille1, int p_braille2, int p_braille3) {
  if (cancelPrint()) {
    return;
  } // end if

  if (p_braille1 == 0 && p_braille2 == 0 && p_braille3 == 0) {
    // Nada por hacer.
    moveE();
    return;
  } // end if

  // Caracter linea 1 columna 123
  punchIfDotted(p_braille1, 1);
  moveB();
  punchIfDotted(p_braille1, 2);
  moveB();
  punchIfDotted(p_braille1, 3);
  if (cancelPrint()) {
    return;
  } // end if

  if (p_braille2 != 0 || p_braille3 != 0) {
    // Caracter linea 2 columna 123
    moveF();
    punchIfDotted(p_braille2, 1);
    moveB();
    punchIfDotted(p_braille2, 2);
    moveB();
    punchIfDotted(p_braille2, 3);
    if (cancelPrint()) {
      return;
    } // end if

    if (p_braille3 != 0) {
      // Caracter linea 3 columna 123
      moveF();
      punchIfDotted(p_braille3, 1);
      moveB();
      punchIfDotted(p_braille3, 2);
      moveB();
      punchIfDotted(p_braille3, 3);
      moveA();
      punchIfDotted(p_braille3, 6);
      move_B();
      punchIfDotted(p_braille3, 5);
      move_B();
      punchIfDotted(p_braille3, 4);
      if (cancelPrint()) {
        return;
      } // end if

      move_F();
    } else {
      // Directamente avanza a la columna 654
      moveA();
    } // end if
    punchIfDotted(p_braille2, 6);
    move_B();
    punchIfDotted(p_braille2, 5);
    move_B();
    punchIfDotted(p_braille2, 4);
    if (cancelPrint()) {
      return;
    } // end if

    move_F();
  } else {
    // Directamente avanza a la columna 654
    moveA();
  } // end if
  punchIfDotted(p_braille1, 6);
  move_B();
  punchIfDotted(p_braille1, 5);
  move_B();
  punchIfDotted(p_braille1, 4);
  // Avanza hacia la siguiente columna y termina
  if (cancelPrint()) {
    return;
  } // end if

  moveE();

}
/*
  Maniobra completa de impresión
*/
void printAll() {
  g_cancelPrint = false;
  tapeOut();
  delay(1000);
  tapeIn();
  delay(1000);
  moveToFirstPrintableLine();

  // Calcula la línea de máxima longitud
  int l_maxIndex = g_bufferLen1;
  if (g_bufferLen2 > l_maxIndex) {
    l_maxIndex = g_bufferLen2;
  } // end if
  if (g_bufferLen3 > l_maxIndex) {
    l_maxIndex = g_bufferLen3;
  } // end if
  // Recorre los buffers por columna
  for (int l_index = 0; l_index < l_maxIndex; l_index++) {
    if (cancelPrint()) {
      return;
    } // end if

    int l_braille1 = g_bufferBraille1[l_index];
    int l_braille2 = g_bufferBraille2[l_index];
    int l_braille3 = g_bufferBraille3[l_index];
    printThreeSymbols(l_braille1, l_braille2, l_braille3);
  } // end for
  tapeOut();
}

/*
   Devuelve true cuando la cinta está sobre el sensor 1
*/
boolean isSensor1() {
  return digitalRead(PIN_CINTA_1) == 0;
}

/*
   Devuelve true cuando la cinta está sobre el sensor 2
*/
boolean isSensor2() {
  return digitalRead(PIN_CINTA_2) == 0;
}

/*
   Devuelve true cuando la cinta está sobre el sensor 2
*/
boolean isEndHead() {
  return digitalRead(PIN_FIN_CARRERA) == 0;
}

void punch() {
  digitalWrite(PIN_ELECTROIMAN, HIGH);
  delay(30);
  digitalWrite(PIN_ELECTROIMAN, LOW);
}

/*
  Actualiza el display LCD a partir del contenido del buffer en memoria
*/
void updateDisplay() {
  Serial.print("updateDisplay ");
  Serial.print(g_displayLine1);
  Serial.print(" // ");
  Serial.println(g_displayLine2);
  g_lcd.setCursor(0, 0);
  g_lcd.print(g_displayLine1);
  g_lcd.setCursor(0, 1);
  g_lcd.print(g_displayLine2);
  g_lcd.setCursor(g_myColumnCursor, g_currentDisplayLine - 1);
}

/*
  Limpia el display y posiciona al comienzo de la línea 1
*/
void clearDisplay() {
  Serial.println("ClearDisplay");
  g_lcd.clear();
  g_displayLine1 = "";
  g_displayLine2 = "";
  g_currentDisplayLine = 1;
  g_myColumnCursor = 0;
  updateDisplay();
}


void printDisplayLines(String p_line1, String p_line2) {
  Serial.print("printDisplayLines:");
  Serial.print(p_line1);
  Serial.print(" // ");
  Serial.println(p_line2);
  clearDisplay();
  g_displayLine1 = "";
  g_displayLine1.concat(p_line1);
  g_displayLine2 = "";
  g_displayLine2.concat(p_line2);
  updateDisplay();
}

/*
  Agrega en el display un caracter, al final de la línea de pantalla vigente
*/
void addDisplayChar(int p_toDisplay) {

  if (g_currentDisplayLine == 1) {
    // Primera linea
    g_displayLine1 = g_displayLine1 + (char)p_toDisplay;

    g_myColumnCursor = g_displayLine1.length();

    if (g_displayLine1.length() >= 16) {
      // Desborda desde la primera línea hacia la segunda
      g_displayLine2 = g_displayLine1;
      g_displayLine1.remove(16);
      g_displayLine2.remove(0, 16);
      g_myColumnCursor = g_displayLine2.length();
      g_currentDisplayLine = 2;
      g_currentBufferLine++;
    } // end if
  } else {
    // Segunda linea
    g_displayLine2 = g_displayLine2 + (char)p_toDisplay;
    if (g_displayLine2.length() >= 16) {
      if (g_currentBufferLine == 3) {
        // No puede desbordar más. Queda donde está
        g_displayLine2.remove(16);
        // TODO: hacer un beep indicando el final
      } else {
        g_currentBufferLine++;
        // Desborda segunda linea
        g_displayLine1 = g_displayLine2;
        g_lcd.clear();
        g_displayLine1.remove(16);
        g_displayLine2.remove(0, 16);
      }// end if
    } // end if
    g_myColumnCursor = g_displayLine2.length();
  } // end if

  updateDisplay();
}




/*
   Agrega un símbolo p_cod Braille al final del buffer indicado. Si p_prefijo no es cero,
    lo agrega antes de p_cod
*/
void addBrailleSym(int &p_bufferLen, int p_bufferBraille[], int p_prefijo, int p_cod) {

  if (p_prefijo != 0) {
    p_bufferBraille[p_bufferLen++] = p_prefijo;
    Serial.print(p_prefijo);
    Serial.print("-");
  } // end if
  p_bufferBraille[p_bufferLen++] = p_cod;
  Serial.println(p_cod);

}

/*
   Traduce el contenido completo de un buffer ASCII hacia el buffer Braille
   Sólo traduce letras en minúsculas
*/
void translate(char p_bufferAscii[], int &p_bufferLen, int p_bufferBraille[]) {
  int l_asciiIndex = 0;
  while (true) {
    char l_ascii = p_bufferAscii[l_asciiIndex];
    if (l_ascii == '\0') {
      break;
    } // end if
    if (l_ascii >= 'A' && l_ascii <= 'Z') {
      // Letra mayúscula. Convierte
      l_ascii = l_ascii - 'A' + 'a';
    }
    if (l_ascii >= 'a' && l_ascii <= 'z') {
      // Letra
      int l_tableIndex = l_ascii - 'a';
      addBrailleSym(p_bufferLen, p_bufferBraille, 0, g_codigosLetras[l_tableIndex]);
    } else {
      if (l_ascii >= '0' && l_ascii <= '9') {
        // Dígito
        int l_tableIndex = l_ascii - '0';
        addBrailleSym(p_bufferLen, p_bufferBraille, 3456, g_codigosDigitos[l_tableIndex]);
      } else {
        // Simbolo
        switch (l_ascii) {
          case '"': {
              addBrailleSym(p_bufferLen, p_bufferBraille, 1256, 1256);
            } break;
          case '@': {
              addBrailleSym(p_bufferLen, p_bufferBraille, 0, 5);
            } break;
          case '#': {
              addBrailleSym(p_bufferLen, p_bufferBraille, 3456, 5);
            } break;
          case '$': {
              addBrailleSym(p_bufferLen, p_bufferBraille, 456, 234);
            } break;
          case '&': {
              addBrailleSym(p_bufferLen, p_bufferBraille, 6, 12346);
            } break;
          case '/': {
              addBrailleSym(p_bufferLen, p_bufferBraille, 6, 2);
            } break;
          case '(': {
              addBrailleSym(p_bufferLen, p_bufferBraille, 0, 126);
            } break;
          case ')': {
              addBrailleSym(p_bufferLen, p_bufferBraille, 0, 345);
            } break;
          case '.': {
              addBrailleSym(p_bufferLen, p_bufferBraille, 0, 3);
            } break;
          case ',': {
              addBrailleSym(p_bufferLen, p_bufferBraille, 0, 2);
            } break;
          case '-': {
              addBrailleSym(p_bufferLen, p_bufferBraille, 0, 36);
            } break;
          case ':': {
              addBrailleSym(p_bufferLen, p_bufferBraille, 0, 25);
            } break;
          case '_': {
              addBrailleSym(p_bufferLen, p_bufferBraille, 36, 36);
            } break;
          case '=': {
              addBrailleSym(p_bufferLen, p_bufferBraille, 0, 2356);
            } break;
          case 164: { // ñ
              addBrailleSym(p_bufferLen, p_bufferBraille, 0, 12456);
            } break;
          case ' ': {
              addBrailleSym(p_bufferLen, p_bufferBraille, 0, 0);
            } break;
        } // end switch
      } // end if
    } // end if
    l_asciiIndex++;
  } // end while
}

/*
  Traduce el contenido de los tres buffers ASCII, hacia los tres buffers Braille,
  anteponiendo los prefijos que fueran necesarios para cada caracter
*/
void translate() {
  // Limpia los tres buffers Braille
  g_bufferLen1 = 0;
  g_bufferLen2 = 0;
  g_bufferLen3 = 0;
  memset(g_bufferBraille1, 0, sizeof(g_bufferBraille1));
  memset(g_bufferBraille2, 0, sizeof(g_bufferBraille2));
  memset(g_bufferBraille3, 0, sizeof(g_bufferBraille3));

  // Traduce
  translate(g_bufferAscii1, g_bufferLen1, g_bufferBraille1);
  translate(g_bufferAscii2, g_bufferLen2, g_bufferBraille2);
  translate(g_bufferAscii3, g_bufferLen3, g_bufferBraille3);

}



// Agentes de programación cooperativa **********************************

void agenteTeclado() {
  if (!g_keyboard.available()) {
    // Nada por hacer.
    return;
  } // end if

  char l_keyChar = g_keyboard.read();

  if (l_keyChar >= 'a' && l_keyChar <= 'z') {
    // Convierte a mayúscula
    l_keyChar = l_keyChar - 'a' + 'A';
  } // end if

  switch (l_keyChar) {
    case PS2_ENTER: { // salto de linea
        pressEnter();
      } break;
    case PS2_TAB: { // tabulador
        pressTab();
      } break;
    case PS2_ESC: { // escape
        pressEscape();
      } break;
    case 127: { // backspace
        pressBackspace();
      } break;
    case 43: { // imprimir
        pressPrint();
      } break;
    default: { // otro caracter
        if (g_charset.indexOf(l_keyChar) >= 0) {
          // Caracter imprimible
          pressPrintable(l_keyChar);
        } // end if
      }
  } // end switch
}

/*
  Lee todo
*/
void pressTab() {
  leeString(g_displayLine1);
  g_dfplayer.play(MP3_SIG_LINEA);
  delay(700);
  leeString(g_displayLine2);
}

void leeString(String p_str) {
  if (p_str.length() == 0) {
    return;
  }
  for (int l_index = 0; l_index < p_str.length(); l_index++) {
    char l_char = p_str.charAt(l_index);
    mp3reproduce(l_char);
    delay(700);
  } // end for
}

void pressBackspace() {
  g_dfplayer.play(MP3_BORRAR);

  // Corrige buffer de display
  if (g_currentDisplayLine == 1) {
    if (g_myColumnCursor > 0) {
      g_displayLine1.remove(g_displayLine1.length() - 1);
      g_myColumnCursor--;
    } else {
      // Salta a línea anterior, si existe
    } // end if
  } else {
    // Línea 2 de pantalla
    if (g_myColumnCursor > 0) {
      g_displayLine2.remove(g_displayLine2.length() - 1);
      g_myColumnCursor--;
    } else {
      // Salta a línea anterior
      g_displayLine1.remove(g_displayLine1.length() - 1);
      g_myColumnCursor = g_displayLine1.length();
      g_currentDisplayLine = 1;
    } // end if
  } // end if
  g_lcd.clear();
  updateDisplay();

}

void pressEnter() {
  g_dfplayer.play(MP3_SIG_LINEA);
  if (g_currentDisplayLine == 1) {
    g_currentDisplayLine++;
    g_myColumnCursor = 0;
    updateDisplay();
  } // end if

}

void pressPrint() {
  g_dfplayer.play(MP3_IMPRIMIR);

  g_displayLine1.toCharArray(g_bufferAscii1, 17);
  g_displayLine2.toCharArray(g_bufferAscii2, 17);

  translate();
  printAll();
}

boolean cancelPrint() {
  if (g_keyboard.available()) {
    char l_keyChar = g_keyboard.read();
    if (l_keyChar == PS2_ESC) {
      g_dfplayer.play(MP3_CANCELAR);
      g_cancelPrint = true;
    } // end if
  } // end if

  return g_cancelPrint;

}


void pressPrintable(int p_keyChar) {
  Serial.print(p_keyChar);
  // Reproduce el audio
  mp3reproduce(p_keyChar);
  // Muestra en el LCD
  addDisplayChar(p_keyChar);
}

/*
  Borra todo los buffers
*/
void pressEscape() {
  g_dfplayer.play(MP3_CANCELAR);
  clearDisplay();
}



/*
   Punto de entrada de la aplicación
*/
void setup() {
  // Inicialización del hardware ***************************

  Serial.begin(115200);

  // Inicializacion teclado PS2
  g_keyboard.begin(PIN_KEYBOARD_DATA, PIN_KEYBOARD_CLOCK, PS2Keymap_Spanish);

  // Display LCD
  g_lcd.begin(16, 2); // Inicializa la interface para el LCD screen, y determina sus dimensiones (ancho y alto) del display
  g_lcd.blink();
  printDisplayLines("UNSTA-FI", "ROBRA");

  // Inicialización DFPlayer
  g_dfPlayerSerial.begin(9600);
  if (!g_dfplayer.begin(g_dfPlayerSerial)) {
    Serial.println("ERROR DE INICIALIZACION DFPLAYER");
    while (true); // detiene el proceso
  } // end if
  g_dfplayer.volume(30);  //Set volume value. From 0 to 30
  g_dfplayer.play(MP3_ESPACIO); // Audio de bienvenida

  // Motor Y (cabezal)
  pinMode(PIN_MOTORY_1, OUTPUT);
  pinMode(PIN_MOTORY_2, OUTPUT);
  pinMode(PIN_MOTORY_3, OUTPUT);
  pinMode(PIN_MOTORY_4, OUTPUT);

  // Motor X (cinta)
  pinMode(PIN_MOTORX_1, OUTPUT);
  pinMode(PIN_MOTORX_2, OUTPUT);
  pinMode(PIN_MOTORX_3, OUTPUT);
  pinMode(PIN_MOTORX_4, OUTPUT);

  // Sensor cinta 1 (conector G)
  pinMode(PIN_CINTA_1, INPUT);

  // Sensor cinta 2 (conector H)
  pinMode(PIN_CINTA_2, INPUT);

  // Sensor fin de carrera (conector E)
  pinMode(PIN_FIN_CARRERA, INPUT);

  // Establece el pull-up del sensor de fin de carrera
  digitalWrite(PIN_FIN_CARRERA, true);

  // Driver electroimán (conector F)
  pinMode(PIN_ELECTROIMAN, OUTPUT);

  delay(2000);
  clearDisplay();

}

/*
   Ciclo principal de la aplicación
*/
void loop() {
  g_momentoActual = millis();

  // Llamada a agentes
  agenteTeclado();

}
