#include <Arduino.h>         
#include <Wire.h>
#include <AMS_5600.h> 
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <FlashStorage.h>
#include <Adafruit_NeoPixel.h>

#define NEOP      9   // pin de neopixel
#define PIX_NUM   12  // numero de neopixels
// Crea instancia e inicializa Neopixels
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(PIX_NUM, NEOP, NEO_GRB + NEO_KHZ800);

// Crea instancia para el sensor
AMS_5600 ams;

#define OSC_FREQ        48000000
#define I2CSPEED        1000000

#define USB             1
#define SERIAL          0

#define ONE_THIRD_FULL_ANGLE    1365
#define TWO_THIRD_FULL_ANGLE    2730
#define PIX_PORTION     342
#define CCW             0
#define CW              1
#define NO_ACK          0
#define MIN_ACK         1
#define FULL_ACK        2
#define CALL            0
#define CHANGE          1
#define FLUID           2
#define DUTY            0
#define FREQUENCY       1  

#define VERSION         1
#define ANGLEMAX        2
#define ANGLEMIN        3
#define ANGLEFULL       4
#define ANGLE           5
#define DIRECTION       6
#define TURN            7
#define PA27MIN_ANGLE   8
#define PA27MAX_ANGLE   9
#define PA22MIN_ANGLE   10
#define PA22MAX_ANGLE   11
#define PA19MIN_ANGLE   12
#define PA19MAX_ANGLE   13
#define PA18MIN_ANGLE   14
#define PA18MAX_ANGLE   15
#define PA07MIN_TURN    16
#define PA07MAX_TURN    17
#define PA06MULTIPLE    18
#define BAUD            19

#define RED             1
#define GREEN           2
#define BLUE            3
#define YELLOW          4
#define MAGENTA         5
#define CYAN            6
#define WHITE           7

#define SINGLE          1
#define RISING          2
#define FALLING         3

FlashStorage(ADD_ack_level, int);
FlashStorage(ADD_flag_angleFluid, short);
FlashStorage(ADD_flag_turnFluid, short);
FlashStorage(ADD_updateMode, short);
FlashStorage(ADD_max_angle, int);
FlashStorage(ADD_min_angle, int);
FlashStorage(ADD_turn, long long);
FlashStorage(ADD_turnUpdate, short);
FlashStorage(ADD_direction, short);
FlashStorage(ADD_angleContinue, short);
FlashStorage(ADD_angleUpdate, short);
FlashStorage(ADD_turnContinue, short);
FlashStorage(ADD_pa27_min_angle, int);
FlashStorage(ADD_pa27_max_angle, int);
FlashStorage(ADD_pa27_enable, int);
FlashStorage(ADD_pa27_logic, short);
FlashStorage(ADD_pa22_min_angle, int);
FlashStorage(ADD_pa22_max_angle, int);
FlashStorage(ADD_pa22_enable, int);
FlashStorage(ADD_pa22_logic, short);
FlashStorage(ADD_pa19_min_angle, int);
FlashStorage(ADD_pa19_max_angle, int);
FlashStorage(ADD_pa19_enable, int);
FlashStorage(ADD_pa19_logic, short);
FlashStorage(ADD_pa18_min_angle, int);
FlashStorage(ADD_pa18_max_angle, int);
FlashStorage(ADD_pa18_enable, int);
FlashStorage(ADD_pa18_logic, short);
FlashStorage(ADD_pa07_min_turn, int);
FlashStorage(ADD_pa07_max_turn, int);
FlashStorage(ADD_pa07_enable, int);
FlashStorage(ADD_pa07_logic, short);
FlashStorage(ADD_pa06_turn_mult, int);
FlashStorage(ADD_pa06_enable, int);
FlashStorage(ADD_pa06_logic, short);
FlashStorage(ADD_pa06_pulse_duration, int);

FlashStorage(ADD_baud, long);
FlashStorage(ADD_ackDelay, unsigned int);
FlashStorage(ADD_color_bright, unsigned int);
FlashStorage(ADD_pixel_enable, short);
FlashStorage(ADD_pixel_color, int);
FlashStorage(ADD_pixel_effect, int);
FlashStorage(ADD_turn_pulse_duration, int);

// Variables globales
bool input_channel;
int max_angle = 360;
int min_angle = 0;
const long long min_turn = -2147483648;
const long long max_turn = 2147483647;
int full_angle = max_angle - min_angle;
int ack_level = FULL_ACK;
int raw_angle;
int angle;
int raw_new_angle;
int new_angle;
int user_angle;
int user_turn;
int raw_old_angle;
int old_angle;
int ackDelay = 1000;
int requestValue = NULL;
int color_bright = 50;
int pixel_color = RED;
int pixel_effect = SINGLE;
int turn_pulse_duration = 1000;   // uS
int pa27_min_angle = 0;
int pa27_max_angle = 90;
int pa22_min_angle = 90;
int pa22_max_angle = 180;
int pa19_min_angle = 180;
int pa19_max_angle = 270;
int pa18_min_angle = 270;
int pa18_max_angle = 360;
int pa07_min_turn = 0;
int pa07_max_turn = 10;
int pa06_pulse_duration = 100;    // uS
long long pa06_turn_mult = 10;  // multiplo de vueltas para dar un pulso en PA06
int updateMode = CALL;
int ind[5] = {0,0,0,0,0}; // posiciones de los comandos

long baud = 9600;
long long turn = 0;
long long new_turn;
long long old_turn;

short flag_newCommand = false;
short flag_newTurn = false;
short flag_turnChanged = false;
short flag_angleFluid = true;
short flag_angleChanged = false;
short flag_turnFluid = true;
short flag_direction = CW;
short flag_error_syntax = false;
short flag_error_range = false;
short flag_error_void = false;
short flag_pa27_level = HIGH;
short flag_pa22_level = HIGH;
short flag_pa19_level = HIGH;
short flag_pa18_level = HIGH;
short flag_pa07_level = HIGH;
short flag_pa06_level = HIGH;
short pa27_enable = true;
short pa27_logic = true;
short pa22_enable = true;
short pa22_logic = true;
short pa19_enable = true;
short pa19_logic = true;
short pa18_enable = true;
short pa18_logic = true;
short pa07_enable = true;
short pa07_logic = true;
short pa06_enable = true;
short pa06_logic = true;
short pixel_enable = true;


String firmwareVersion = "PMXAS5600N12 - Version 1.0 - 28/02/2019";
String readString; // string introducido por usuario
String subCom[6] = {"","","","","",""}; // String para cada subcomando

// Funcion que recupera los datos guardados en Flash
void load_values() {
  ack_level = ADD_ack_level.read();
  flag_angleFluid = ADD_flag_angleFluid.read();
  flag_turnFluid = ADD_flag_turnFluid.read();
  updateMode = ADD_updateMode.read();
  min_angle = ADD_min_angle.read();                       // angulo minimo
  max_angle = ADD_max_angle.read();                       // angulo maximo
  turn = ADD_turn.read();                                 // numero de vueltas
  flag_direction = ADD_direction.read();                  // sentido de giro incremental
  pa27_min_angle = ADD_pa27_min_angle.read();             // angulo minimo para PA27
  pa27_max_angle = ADD_pa27_max_angle.read();             // angulo maximo para PA27
  pa27_enable = ADD_pa27_enable.read();                   // habilitacion de PA27
  pa27_logic = ADD_pa27_logic.read();                     // logica del pin PA27
  pa22_min_angle = ADD_pa22_min_angle.read();             // angulo minimo para PA22
  pa22_max_angle = ADD_pa22_max_angle.read();             // angulo maximo para PA22
  pa22_enable = ADD_pa22_enable.read();                   // habilitacion de PA22
  pa22_logic = ADD_pa22_logic.read();                     // logica del pin PA22
  pa19_min_angle = ADD_pa19_min_angle.read();             // angulo minimo para PA19
  pa19_max_angle = ADD_pa19_max_angle.read();             // angulo maximo para PA19
  pa19_enable = ADD_pa19_enable.read();                   // habilitacion de PA19
  pa19_logic = ADD_pa19_logic.read();                     // logica del pin PA19
  pa18_min_angle = ADD_pa18_min_angle.read();             // angulo minimo para PA18
  pa18_max_angle = ADD_pa18_max_angle.read();             // angulo maximo para PA18
  pa18_enable = ADD_pa18_enable.read();                   // habilitacion de PA18
  pa18_logic = ADD_pa18_logic.read();                     // logica del pin PA18
  pa07_min_turn = ADD_pa07_min_turn.read();               // vueltas minimas para PA07
  pa07_max_turn = ADD_pa07_max_turn.read();               // vueltas maximas para PA07
  pa07_enable = ADD_pa07_enable.read();                   // habilitacion de PA07
  pa07_logic = ADD_pa07_logic.read();                     // logica del pin PA07
  pa06_turn_mult = ADD_pa06_turn_mult.read();             // multiplo de vueltas para PA06
  pa06_enable = ADD_pa06_enable.read();                   // habilitacion de PA06
  pa06_logic = ADD_pa06_logic.read();                     // logica del pin PA06
  pa06_pulse_duration = ADD_pa06_pulse_duration.read();   // guarda la duracion del pulso en PA06
  color_bright = ADD_color_bright.read();                 // brillo de los neopixels
  pixel_color = ADD_pixel_color.read();                   // color de los neopixels
  pixel_effect = ADD_pixel_effect.read();                 // efecto de los neopixels
}

// Funcion que configura las entradas y salidas
void config_io() {
  // configura entrada o salida
  pinMode(DIR_, OUTPUT);  // Configura el pin para el cambio de sentido
  pinMode(PA27, OUTPUT);    // pin27 de rango de angulo como salida
  pinMode(PA22, OUTPUT);    // pin22 de rango de angulo como salida
  pinMode(PA19, OUTPUT);    // pin19 de rango de angulo como salida
  pinMode(PA18, OUTPUT);    // pin18 de rango de angulo como salida
  pinMode(PA14, OUTPUT);    // pin pulso por revolucion CW como salida
  pinMode(PA09, OUTPUT);    // pin pulso por revolucion CCW como salida
  pinMode(PA07, OUTPUT);    // pin07 rango de vueltas como salida
  pinMode(PA06, OUTPUT);    // pin06 multiplo de vueltas como salida

  // determina estado inicial de salidas
  digitalWrite(PA14, LOW);
  digitalWrite(PA09, LOW);
  digitalWrite(PA06, LOW);
  digitalWrite(DIR_, flag_direction);
}

// Funcion que configura el PWM del PIN PA05
void configurePWM() {
  // PWM on PA05
  PORT->Group[0].DIRSET.reg = PORT_PA05;
  PORT->Group[0].OUTCLR.reg = PORT_PA05;
  PORT->Group[0].PINCFG[5].reg |= PORT_PINCFG_PMUXEN;
  PORT->Group[0].PMUX[4 >> 1].reg =  PORT_PMUX_PMUXO_E;
      
  PM->APBCMASK.reg |= PM_APBCMASK_TCC0;
  GCLK->CLKCTRL.reg = (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID(TCC0_GCLK_ID));
  while (GCLK->STATUS.bit.SYNCBUSY) {}
  
  TCC0->CTRLA.reg = TCC_CTRLA_SWRST;
  while (TCC0->SYNCBUSY.reg & TCC_SYNCBUSY_SWRST) {}
  TCC0->CTRLBCLR.reg = TCC_CTRLBCLR_DIR;
  while (TCC0->SYNCBUSY.reg & TCC_SYNCBUSY_CTRLB) {}
  
  TCC0->CTRLA.reg = (TCC_CTRLA_PRESCSYNC_GCLK_Val | TCC_CTRLA_PRESCALER(TCC_CTRLA_PRESCALER_DIV256_Val));
  TCC0->WAVE.reg = (TCC_WAVE_WAVEGEN_NPWM);
  while (TCC0->SYNCBUSY.reg & TCC_SYNCBUSY_WAVE) {}
  TCC0->PER.reg = 74;
  while (TCC0->SYNCBUSY.reg & TCC_SYNCBUSY_PER) {}
  //TCC0->CC[1].reg = 1;
  TCC0->CTRLA.reg |= (TCC_CTRLA_ENABLE);
}

// Funcion que determna el nivel de acknowledge que se emite despues de un comando
void ack(String var_name, int variable) {
  switch(ack_level) {
    case NO_ACK:
      break;
    case MIN_ACK:
      if(input_channel == USB) Serial.println("ok");
      else Serial1.println("ok");
      delay(ackDelay);
      break;
    case FULL_ACK:
      if(input_channel == USB) {
        Serial.print(var_name + ": ");
        Serial.println(variable);
      }
      else
      {
        Serial1.print(var_name + ": ");
        Serial1.println(variable);
      }
      delay(ackDelay);
      break;
  }
}

// Funcion que imprime la version del programa
void printVersion() {
  if(input_channel == USB) {
    Serial.println(firmwareVersion + "ProyectilMx - Hazlo real");
    Serial.println("Ingresa info; para instrucciones");
  }
  else {
    Serial1.println(firmwareVersion + "ProyectilMx - Hazlo real");
    Serial1.println("Ingresa info; para instrucciones");
  }
}

// Funcion que imprime las instrucciones de como usar los comandos de la placa
void print_instructions() {
  if(input_channel == USB) {
    Serial.println("\n ---------------");
    Serial.println("| Instrucciones |");
    Serial.println(" ---------------");
    Serial.println("--------------------------------------------------------------------------------------------------");
    Serial.println("Hay 3 comandos principales:");
    Serial.println("set  - Escribe en la memoria de la tarjeta un nuevo valor");
    Serial.println("get  - Lee desde la memoria de la tarjeta un valor existente");
    Serial.println("save - Salva un valor en memoria");
    Serial.println("Para separar comandos se utiliza el caracter coma (,)");
    Serial.println("Para que se considere un comando valido, debe terminar con punto y coma (;)");
    Serial.println("--------------------------------------------------------------------------------------------------");
    Serial.println("Comandos GET Para lectura de valores:");
    Serial.println("get,registro;");
    Serial.println("get,registro,subregistro;");
    Serial.println("donde: \'registro\'    es la variable a leer.");
    Serial.println("       \',\'           es el caracter de separacion");
    Serial.println("       \'subregistro\' puede ser un subregistro de registro.");
    Serial.println("       \';\'           es el caracter de terminacion de comando");
    Serial.println("--------------------------------------------------------------------------------------------------");
    Serial.println("Comandos SET para cambiar la configuracion:");
    Serial.println("set,registro;");
    Serial.println("set,registro,____;");
    Serial.println("set,registro,subregistro,____;");
    Serial.println("donde: \'registro\'    es la variable a configurar.");
    Serial.println("       \',\'           es el caracter de separacion");
    Serial.println("       \'subregistro\' puede ser un subregistro de registro.");
    Serial.println("       \',\'           es el caracter de separacion");
    Serial.println("       \'____\'        es el valor que se escribira en ese registro.");
    Serial.println("       \';\'           es el caracter de terminacion de comando");
    Serial.println("--------------------------------------------------------------------------------------------------");
    Serial.println("Ejemplo de comandos set:");
    Serial.println("set,angle,min,-180;   -> configura el angulo minimo en -180");
    Serial.println("set,angle,max,180;    -> configura el angulo maximo en 180");
    Serial.println("set,turn,10;          -> reinicia el contador de vueltas a 10");
    Serial.println("set,pa27,max,100;     -> configura el angulo maximo de PA27 en 100");
    Serial.println("--------------------------------------------------------------------------------------------------");  
    Serial.println("Lista de comandos get:");
    Serial.println("get,version;     -> devuelve la version del firmware");
    Serial.println("get,angle;       -> devuelve el valor del angulo actual.");
    Serial.println("get,angle,max;   -> devuelve el valor del angulo maximo.");
    Serial.println("get,angle,min;   -> devuelve el valor del angulo minimo.");
    Serial.println("get,angle,full;  -> devuelve el valor de angulo total (max - min)");
    Serial.println("get,turn;        -> devuelve el numero de vueltas que ha dado el dial (positivo o negativo)");
    Serial.println("get,pa27,min;    -> devuelve el valor del angulo minimo para la salida PA27.");
    Serial.println("get,pa27,max;    -> devuelve el valor del angulo maximo para la salida PA27.");
    Serial.println("get,pa22,min;    -> devuelve el valor del angulo minimo para la salida PA22.");
    Serial.println("get,pa22,max;    -> devuelve el valor del angulo maximo para la salida PA22.");
    Serial.println("get,pa19,min;    -> devuelve el valor del angulo minimo para la salida PA19.");
    Serial.println("get,pa19,max;    -> devuelve el valor del angulo maximo para la salida PA19.");
    Serial.println("get,pa18,min;    -> devuelve el valor del angulo minimo para la salida PA18.");
    Serial.println("get,pa18,max;    -> devuelve el valor del angulo maximo para la salida PA18.");
    Serial.println("get,pa07,min;    -> devuelve el valor del angulo minimo para el rango de vueltas de PA07.");
    Serial.println("get,pa07,max;    -> devuelve el valor del angulo maximo para el rango de vueltas de PA07.");
    Serial.println("get,pa06,mult;   -> devuelve el del multiplo de vueltas para PA06.");
    Serial.println("get,dir;         -> devuelve la direccion de incremento al girar el dial (CW o CCW)");
    Serial.println("get,baud;        -> devuelve la velocidad de comunicacion en baudios.");
    Serial.println("--------------------------------------------------------------------------------------------------");
    Serial.println("Lista de comandos set:");
    Serial.println("set,ack,none;             -> Configura tipo de acknowledge a ninguno");
    Serial.println("set,ack,min;              -> Configura tipo de acknowledge al minimo, correcto = OK, incorrecto = ERROR");
    Serial.println("set,ack,full;             -> Configura tipo de acknowledge a detallado");
    Serial.println("set,ack,delay;            -> Tiempo de espera despues del acknowledge");
    Serial.println("set,dir,cw;               -> Direccion de incremento a favor de las manecillas");
    Serial.println("set,dir,ccw;              -> Direccion de incremento en contra de las manecillas");
    Serial.println("set,update,fluid,angle;   -> Impresion constante de angulo");
    Serial.println("set,update,fluid,turn;    -> Impresion constante de numero de vuelta");
    Serial.println("set,update,fluid,both;    -> Impresion constante de angulo y numero de vuelta");
    Serial.println("set,update,change,angle;  -> Impresion de angulo en cuanto cambia de valor");
    Serial.println("set,update,change,turn;   -> Impresion de numero de vuelta en cuanto cambia de valor");
    Serial.println("set,update,change,both;   -> Impresion de ambas variables en cuanto camban de valor");
    Serial.println("set,update,call;          -> Configura el modo de impresion bajo consulta");
    Serial.println("set,angle,min,____;       -> Configura el angulo minimo (rango: -2047 a 2048)");
    Serial.println("set,angle,max,____;       -> Configura el angulo maximo (rango: -2047 a 2048)");
    Serial.println("set,turn,____;            -> Configura el valor actual de vueltas (rango: -2147483648 a 2147483647)");
    Serial.println("set,turn,pulse,____;      -> Configura la duracion del pulso por revolucion en PA14 y PA09 (rango: 5 a 100 uS)");
    Serial.println("set,baud,____;            -> configura la velocidad de comunicacion en baudios, valores:");
    Serial.println("                             9600, 19200, 38400, 57600, 74880, 115200, 230400 ,250000 o 500000");
    Serial.println("                             Si se desenegiza la placa, este valor volvera al previo guardado.");
    Serial.println("                             Para que permanezca en memoria, debe usar el comando save.");
    Serial.println("set,pa27,min,____;        -> configura el angulo minimo para la salida PA27 (rango: minAngle a maxAngle)");
    Serial.println("set,pa27,max,____;        -> configura el angulo maximo para la salida PA27 (rango: minAngle - maxAngle)");
    Serial.println("set,pa27,high;            -> configura en alto la zona de accion de PA27 entre dos angulos");
    Serial.println("set,pa27,low;             -> configura en bajo la zona de accion de PA27 entre dos angulos");
    Serial.println("set,pa27,enable;          -> habilita el PA27 para su uso acotado entre 2 angulos");
    Serial.println("set,pa27,disable;         -> deshabilita el PA27 para su uso acotado entre 2 angulos");
    Serial.println("set,pa22,min,____;        -> configura el angulo minimo para la salida PA22 (rango: minAngle a maxAngle)");
    Serial.println("set,pa22,max,____;        -> configura el angulo maximo para la salida PA22 (rango: minAngle a maxAngle)");
    Serial.println("set,pa22,high;            -> configura en alto la zona de accion de PA22 entre dos angulos");
    Serial.println("set,pa22,low;             -> configura en bajo la zona de accion de PA22 entre dos angulos");
    Serial.println("set,pa22,enable;          -> habilita el PA22 para su uso acotado entre 2 angulos");
    Serial.println("set,pa22,disable;         -> deshabilita el PA22 para su uso acotado entre 2 angulos");
    Serial.println("set,pa19,min,____;        -> configura el angulo minimo para la salida PA19 (rango: minAngle a maxAngle)");
    Serial.println("set,pa19,max,____;        -> configura el angulo maximo para la salida PA19 (rango: minAngle a maxAngle)");
    Serial.println("set,pa19,high;            -> configura en alto la zona de accion de PA19 entre dos angulos");
    Serial.println("set,pa19,low;             -> configura en bajo la zona de accion de PA19 entre dos angulos");
    Serial.println("set,pa19,enable;          -> habilita el PA19 para su uso acotado entre 2 angulos");
    Serial.println("set,pa19,disable;         -> deshabilita el PA19 para su uso acotado entre 2 angulos");
    Serial.println("set,pa18,min,____;        -> configura el angulo minimo para la salida PA18 (rango: minAngle a maxAngle)");
    Serial.println("set,pa18,max,____;        -> configura el angulo maximo para la salida PA18 (rango: minAngle a maxAngle)");
    Serial.println("set,pa18,high;            -> configura en alto la zona de accion de PA18 entre dos angulos");
    Serial.println("set,pa18,low;             -> configura en bajo la zona de accion de PA18 entre dos angulos");
    Serial.println("set,pa18,enable;          -> habilita el PA18 para su uso acotado entre 2 angulos");
    Serial.println("set,pa18,disable;         -> deshabilita el PA18 para su uso acotado entre 2 angulos");
    Serial.println("set,pa07,min,____;        -> configura las vueltas minimas para la salida PA07 (rango: minTurn a maxTurn)");
    Serial.println("set,pa07,max,____;        -> configura el angulo maximo para la salida PA07 (rango: minTurn a maxTurn)");
    Serial.println("set,pa07,high;            -> configura en alto la zona de accion de PA07 dentro del rango de vueltas");
    Serial.println("set,pa07,low;             -> configura en bajo la zona de accion de PA07 dentro del rango de vueltas");
    Serial.println("set,pa07,enable;          -> habilita el PA07 para su uso acotado en numero de vueltas");
    Serial.println("set,pa07,disable;         -> deshabilita el PA07 para su uso acotado en numero de vueltas");
    Serial.println("set,pa06,mult,____;       -> configura el multiplo de vueltas para la salida PA07 (rango: minTurn a maxTurn)");
    Serial.println("set,pa06,high;            -> configura en alto el pulso de PA06 durante el tiempo programable");
    Serial.println("set,pa06,low;             -> configura en bajo el pulso de PA06 durante el tiempo programable");
    Serial.println("set,pa06,enable;          -> habilita el PA06 para su uso como pulso en multiplos de vueltas");
    Serial.println("set,pa06,disable;         -> deshabilita el PA06 para su uso como pulso en multiplos de vueltas");
    Serial.println("set,pixel,bright,____;    -> Configura el brillo de los neopixels (rango: 0 a 255)");
    Serial.println("set,pixel,effect,single;  -> Configura el efecto de los neopixels a un pixel a la vez");
    Serial.println("set,pixel,effect,rise;    -> Los neopixels se iran encendiendo");
    Serial.println("set,pixel,effect,fall;    -> Los neopixels se iran apagando");
    Serial.println("set,pixel,enable;         -> Habilita los neopixels");
    Serial.println("set,pixel,disable;        -> Desahibita los neopixels");
    Serial.println("set,pixel,color,____;     -> Cambia el color de los neopixels, valores:");
    Serial.println("                            red, green, blue, yellow, magenta, cyan, white");
    Serial.println("--------------------------------------------------------------------------------------------------");
    Serial.println("Lista de comandos save:");
    Serial.println("save,baud;                -> Guarda el valor de la velocidad de la comunicacion Serial");
    Serial.println("                            cuando se ocupa set,baud,____; solamente se actualiza baud.");
    Serial.println("                            Al utilizar save, la siguiente vez que se energice, permanecera");
    Serial.println("save,turn;                -> Guarda el valor del numero de vueltas\n");
    Serial.println("Comandos directos:");
    Serial.println("stop;                     -> Detiene el modo fluido y deja de enviar caracteres");
    Serial.println("info;                     -> Imprime este menu");
    Serial.println("reset;                    -> Regresa la configuracion al estado de fabrica " );
  }
  else {
    Serial1.println("\n ---------------");
    Serial1.println("| Instrucciones |");
    Serial1.println(" ---------------");
    Serial1.println("--------------------------------------------------------------------------------------------------");
    Serial1.println("Hay 3 comandos principales:");
    Serial1.println("set  - Escribe en la memoria de la tarjeta un nuevo valor");
    Serial1.println("get  - Lee desde la memoria de la tarjeta un valor existente");
    Serial1.println("save - Salva un valor en memoria");
    Serial1.println("Para separar comandos se utiliza el caracter coma (,)");
    Serial1.println("Para que se considere un comando valido, debe terminar con punto y coma (;)");
    Serial1.println("--------------------------------------------------------------------------------------------------");
    Serial1.println("Comandos GET Para lectura de valores:");
    Serial1.println("get,registro;");
    Serial1.println("get,registro,subregistro;");
    Serial1.println("donde: \'registro\'    es la variable a leer.");
    Serial1.println("       \',\'           es el caracter de separacion");
    Serial1.println("       \'subregistro\' puede ser un subregistro de registro.");
    Serial1.println("       \';\'           es el caracter de terminacion de comando");
    Serial1.println("--------------------------------------------------------------------------------------------------");
    Serial1.println("Comandos SET para cambiar la configuracion:");
    Serial1.println("set,registro;");
    Serial1.println("set,registro,____;");
    Serial1.println("set,registro,subregistro,____;");
    Serial1.println("donde: \'registro\'    es la variable a configurar.");
    Serial1.println("       \',\'           es el caracter de separacion");
    Serial1.println("       \'subregistro\' puede ser un subregistro de registro.");
    Serial1.println("       \',\'           es el caracter de separacion");
    Serial1.println("       \'____\'        es el valor que se escribira en ese registro.");
    Serial1.println("       \';\'           es el caracter de terminacion de comando");
    Serial1.println("--------------------------------------------------------------------------------------------------");
    Serial1.println("Ejemplo de comandos set:");
    Serial1.println("set,angle,min,-180;   -> configura el angulo minimo en -180");
    Serial1.println("set,angle,max,180;    -> configura el angulo maximo en 180");
    Serial1.println("set,turn,10;          -> reinicia el contador de vueltas a 10");
    Serial1.println("set,pa27,max,100;     -> configura el angulo maximo de PA27 en 100");
    Serial1.println("--------------------------------------------------------------------------------------------------");  
    Serial1.println("Lista de comandos get:");
    Serial1.println("get,version;     -> devuelve la version del firmware");
    Serial1.println("get,angle;       -> devuelve el valor del angulo actual.");
    Serial1.println("get,angle,max;   -> devuelve el valor del angulo maximo.");
    Serial1.println("get,angle,min;   -> devuelve el valor del angulo minimo.");
    Serial1.println("get,angle,full;  -> devuelve el valor de angulo total (max - min)");
    Serial1.println("get,turn;        -> devuelve el numero de vueltas que ha dado el dial (positivo o negativo)");
    Serial1.println("get,pa27,min;    -> devuelve el valor del angulo minimo para la salida PA27.");
    Serial1.println("get,pa27,max;    -> devuelve el valor del angulo maximo para la salida PA27.");
    Serial1.println("get,pa22,min;    -> devuelve el valor del angulo minimo para la salida PA22.");
    Serial1.println("get,pa22,max;    -> devuelve el valor del angulo maximo para la salida PA22.");
    Serial1.println("get,pa19,min;    -> devuelve el valor del angulo minimo para la salida PA19.");
    Serial1.println("get,pa19,max;    -> devuelve el valor del angulo maximo para la salida PA19.");
    Serial1.println("get,pa18,min;    -> devuelve el valor del angulo minimo para la salida PA18.");
    Serial1.println("get,pa18,max;    -> devuelve el valor del angulo maximo para la salida PA18.");
    Serial1.println("get,pa07,min;    -> devuelve el valor del angulo minimo para el rango de vueltas de PA07.");
    Serial1.println("get,pa07,max;    -> devuelve el valor del angulo maximo para el rango de vueltas de PA07.");
    Serial1.println("get,pa06,mult;   -> devuelve el del multiplo de vueltas para PA06.");
    Serial1.println("get,dir;         -> devuelve la direccion de incremento al girar el dial (CW o CCW)");
    Serial1.println("get,baud;        -> devuelve la velocidad de comunicacion en baudios.");
    Serial1.println("--------------------------------------------------------------------------------------------------");
    Serial1.println("Lista de comandos set:");
    Serial1.println("set,ack,none;             -> Configura tipo de acknowledge a ninguno");
    Serial1.println("set,ack,min;              -> Configura tipo de acknowledge al minimo, correcto = OK, incorrecto = ERROR");
    Serial1.println("set,ack,full;             -> Configura tipo de acknowledge a detallado");
    Serial1.println("set,ack,delay;            -> Tiempo de espera despues del acknowledge");
    Serial1.println("set,dir,cw;               -> Direccion de incremento a favor de las manecillas");
    Serial1.println("set,dir,ccw;              -> Direccion de incremento en contra de las manecillas");
    Serial1.println("set,update,fluid,angle;   -> Impresion constante de angulo");
    Serial1.println("set,update,fluid,turn;    -> Impresion constante de numero de vuelta");
    Serial1.println("set,update,fluid,both;    -> Impresion constante de angulo y numero de vuelta");
    Serial1.println("set,update,change,angle;  -> Impresion de angulo en cuanto cambia de valor");
    Serial1.println("set,update,change,turn;   -> Impresion de numero de vuelta en cuanto cambia de valor");
    Serial1.println("set,update,change,both;   -> Impresion de ambas variables en cuanto camban de valor");
    Serial1.println("set,update,call;          -> Configura el modo de impresion bajo consulta");
    Serial1.println("set,angle,min,____;       -> Configura el angulo minimo (rango: -2047 a 2048)");
    Serial1.println("set,angle,max,____;       -> Configura el angulo maximo (rango: -2047 a 2048)");
    Serial1.println("set,turn,____;            -> Configura el valor actual de vueltas (rango: -2147483648 a 2147483647)");
    Serial1.println("set,turn,pulse,____;      -> Configura la duracion del pulso por revolucion en PA14 y PA09 (rango: 5 a 100 uS)");
    Serial1.println("set,baud,____;            -> configura la velocidad de comunicacion en baudios, valores:");
    Serial1.println("                             9600, 19200, 38400, 57600, 74880, 115200, 230400 ,250000 o 500000.");
    Serial1.println("                             Si se desenegiza la placa, este valor volvera al previo guardado.");
    Serial1.println("                             Para que permanezca en memoria, debe usar el comando save.");
    Serial1.println("set,pa27,min,____;        -> configura el angulo minimo para la salida PA27 (rango: minAngle a maxAngle)");
    Serial1.println("set,pa27,max,____;        -> configura el angulo maximo para la salida PA27 (rango: minAngle - maxAngle)");
    Serial1.println("set,pa27,high;            -> configura en alto la zona de accion de PA27 entre dos angulos");
    Serial1.println("set,pa27,low;             -> configura en bajo la zona de accion de PA27 entre dos angulos");
    Serial1.println("set,pa27,enable;          -> habilita el PA27 para su uso acotado entre 2 angulos");
    Serial1.println("set,pa27,disable;         -> deshabilita el PA27 para su uso acotado entre 2 angulos");
    Serial1.println("set,pa22,min,____;        -> configura el angulo minimo para la salida PA22 (rango: minAngle a maxAngle)");
    Serial1.println("set,pa22,max,____;        -> configura el angulo maximo para la salida PA22 (rango: minAngle a maxAngle)");
    Serial1.println("set,pa22,high;            -> configura en alto la zona de accion de PA22 entre dos angulos");
    Serial1.println("set,pa22,low;             -> configura en bajo la zona de accion de PA22 entre dos angulos");
    Serial1.println("set,pa22,enable;          -> habilita el PA22 para su uso acotado entre 2 angulos");
    Serial1.println("set,pa22,disable;         -> deshabilita el PA22 para su uso acotado entre 2 angulos");
    Serial1.println("set,pa19,min,____;        -> configura el angulo minimo para la salida PA19 (rango: minAngle a maxAngle)");
    Serial1.println("set,pa19,max,____;        -> configura el angulo maximo para la salida PA19 (rango: minAngle a maxAngle)");
    Serial1.println("set,pa19,high;            -> configura en alto la zona de accion de PA19 entre dos angulos");
    Serial1.println("set,pa19,low;             -> configura en bajo la zona de accion de PA19 entre dos angulos");
    Serial1.println("set,pa19,enable;          -> habilita el PA19 para su uso acotado entre 2 angulos");
    Serial1.println("set,pa19,disable;         -> deshabilita el PA19 para su uso acotado entre 2 angulos");
    Serial1.println("set,pa18,min,____;        -> configura el angulo minimo para la salida PA18 (rango: minAngle a maxAngle)");
    Serial1.println("set,pa18,max,____;        -> configura el angulo maximo para la salida PA18 (rango: minAngle a maxAngle)");
    Serial1.println("set,pa18,high;            -> configura en alto la zona de accion de PA18 entre dos angulos");
    Serial1.println("set,pa18,low;             -> configura en bajo la zona de accion de PA18 entre dos angulos");
    Serial1.println("set,pa18,enable;          -> habilita el PA18 para su uso acotado entre 2 angulos");
    Serial1.println("set,pa18,disable;         -> deshabilita el PA18 para su uso acotado entre 2 angulos");
    Serial1.println("set,pa07,min,____;        -> configura las vueltas minimas para la salida PA07 (rango: minTurn a maxTurn)");
    Serial1.println("set,pa07,max,____;        -> configura el angulo maximo para la salida PA07 (rango: minTurn a maxTurn)");
    Serial1.println("set,pa07,high;            -> configura en alto la zona de accion de PA07 dentro del rango de vueltas");
    Serial1.println("set,pa07,low;             -> configura en bajo la zona de accion de PA07 dentro del rango de vueltas");
    Serial1.println("set,pa07,enable;          -> habilita el PA07 para su uso acotado en numero de vueltas");
    Serial1.println("set,pa07,disable;         -> deshabilita el PA07 para su uso acotado en numero de vueltas");
    Serial1.println("set,pa06,mult,____;       -> configura el multiplo de vueltas para la salida PA07 (rango: minTurn a maxTurn)");
    Serial1.println("set,pa06,high;            -> configura en alto el pulso de PA06 durante el tiempo programable");
    Serial1.println("set,pa06,low;             -> configura en bajo el pulso de PA06 durante el tiempo programable");
    Serial1.println("set,pa06,enable;          -> habilita el PA06 para su uso como pulso en multiplos de vueltas");
    Serial1.println("set,pa06,disable;         -> deshabilita el PA06 para su uso como pulso en multiplos de vueltas");
    Serial1.println("set,pixel,bright,____;    -> Configura el brillo de los neopixels (rango: 0 a 255)");
    Serial1.println("set,pixel,effect,single;  -> Configura el efecto de los neopixels a un pixel a la vez");
    Serial1.println("set,pixel,effect,rise;    -> Los neopixels se iran encendiendo");
    Serial1.println("set,pixel,effect,fall;    -> Los neopixels se iran apagando");
    Serial1.println("set,pixel,enable;         -> Habilita los neopixels");
    Serial1.println("set,pixel,disable;        -> Desahibita los neopixels");
    Serial1.println("set,pixel,color,____;     -> Cambia el color de los neopixels, valores:");
    Serial1.println("                             red, green, blue, yellow, magenta, cyan, white");
    Serial1.println("--------------------------------------------------------------------------------------------------");
    Serial1.println("Lista de comandos save:");
    Serial1.println("save,baud;                -> Guarda el valor de la velocidad de la comunicacion Serial");
    Serial1.println("                             cuando se ocupa set,baud,____; solamente se actualiza baud.");
    Serial1.println("                             Al utilizar save, la siguiente vez que se energice, permanecera");
    Serial1.println("save,turn;                -> Guarda el valor del numero de vueltas\n");
    Serial1.println("Comandos directos:");
    Serial1.println("stop;                     -> Detiene el modo fluido y deja de enviar caracteres");
    Serial1.println("info;                     -> Imprime este menu");
    Serial1.println("reset;                    -> Regresa la configuracion al estado de fabrica " );
  }
}

// Funcion que determina el comando que se recibe por serial
void getCommand() {
  if (Serial.available())  {
    char c = Serial.read();
    if (c == ';') {
      readString.toLowerCase();
      int counter = 0;
      for(int i=0; i<6; i++) subCom[i] = ""; 
      for(int i=0; i<readString.length(); i++) {
        if(readString.charAt(i) == ',') {
          counter++;
        }
        else subCom[counter] += readString.charAt(i);
      }
      flag_newCommand = true;
      input_channel = USB;
      readString = ""; //clears variable for new input
    }  
    else {     
      readString += c; //makes the string readString
    }
  }

  if (Serial1.available())  {
    char c = Serial1.read();
    if (c == ';') {
      readString.toLowerCase();
      int counter = 0;
      for(int i=0; i<6; i++) subCom[i] = ""; 
      for(int i=0; i<readString.length(); i++) {
        if(readString.charAt(i) == ',') {
          counter++;
        }
        else subCom[counter] += readString.charAt(i);
      }
      flag_newCommand = true;
      input_channel = SERIAL;
      readString = ""; //clears variable for new input
    }  
    else {     
      readString += c; //makes the string readString
    }
  }
}

// Funcion que revisa que no haya errores en los comandos
void checkErrors(int commandNumber) {
  if(subCom[commandNumber] == NULL) flag_error_void = true;
  else flag_error_syntax = true;
}

// Funcion que determina el tipo de error que hubo
void errorManage() {
  if(flag_error_syntax == true) {  // si hubo un error de sintaxis en el comando
    if(ack_level == FULL_ACK){
      if(input_channel == USB) Serial.println("syntaxErr");  // imprime err
      else Serial1.println("syntaxErr");  // imprime err
      delay(ackDelay);
    }
    else if(ack_level == MIN_ACK) {
      if(input_channel == USB) Serial.println("error");
      else Serial1.println("error");
    }
  }

  if(flag_error_range == true && ack_level == FULL_ACK) {  // si hubo con error de rango en un valor
    if(input_channel == USB) Serial.println("outRange");   // imprime outRange
    else Serial1.println("outRange");   // imprime outRange
    delay(ackDelay);
  }

  if(flag_error_void == true && ack_level == FULL_ACK) {
    if(input_channel == USB) Serial.println("noValue");
    else Serial1.println("noValue");
    delay(ackDelay);
  }

  flag_error_range = false;     // desactiva bandera de outRange
  flag_error_void = false;      // desactiva bandera de vacio
  flag_error_syntax = false;     // desactiva bandera de error
}

// Funcion que ejecuta los comandos recibidos por serial
void changeManage() {
  raw_angle = ams.getRawAngle();  // captura del angulo absoluto del sensor AS5600
  user_angle = raw_angle;
  TCC0->CC[1].reg = map(raw_angle,0,4095,0,75);   // actualiza el valor del angulo absoluto para el PWM

  // ----------------------------------------------------------------------------------------
  // Inicia determinacion del numero de vueltas hacia ambos sentidos
  // ----------------------------------------------------------------------------------------
  new_angle = angle;  // guarda un nuevo angulo
  raw_new_angle = raw_angle;    // guarda un nuevo angulo absoluto del sensor AS5600
  angle = map(raw_angle, 0, 4095, min_angle, max_angle);  // mapeo del angulo a maximo y minimo
    
  if(raw_new_angle < ONE_THIRD_FULL_ANGLE && raw_old_angle > TWO_THIRD_FULL_ANGLE)  {
    if(turn < 2147483647) turn++;
    flag_newTurn = true;
  }
  else if(raw_old_angle < ONE_THIRD_FULL_ANGLE && raw_new_angle > TWO_THIRD_FULL_ANGLE) {
    if(turn > -2147483648) turn--;
    flag_newTurn = true;
  }

  // Bandera de cambio de angulo
  if(new_angle != old_angle) {
    flag_angleChanged = true;
  }

  old_angle = new_angle;
  raw_old_angle = raw_new_angle;
  // ----------------------------------------------------------------------------------------
  // Termina determinacion del numero de vueltas hacia ambos sentidos
  // ----------------------------------------------------------------------------------------
  // ---------------------------------------------------
  // Inicia cambio de vuelta
  // ---------------------------------------------------
  new_turn = turn;    // guarda un nuevo numero de vueltas
  user_turn = turn;
  
  if(new_turn != old_turn) {
    flag_turnChanged = true;
  }

  if(pa27_enable == HIGH) {
    if(angle >= pa27_min_angle && angle <= pa27_max_angle) digitalWrite(PA27, flag_pa27_level);
    else digitalWrite(PA27, !flag_pa27_level);
  }  
  if(pa22_enable == HIGH) {
    if(angle >= pa22_min_angle && angle <= pa22_max_angle) digitalWrite(PA22, flag_pa22_level);
    else digitalWrite(PA22, !flag_pa22_level);
  }
  if(pa19_enable == HIGH) {
    if(angle >= pa19_min_angle && angle <= pa19_max_angle) digitalWrite(PA19, flag_pa19_level);
    else digitalWrite(PA19, !flag_pa19_level);
  }
  if(pa18_enable == HIGH) {
    if(angle >= pa18_min_angle && angle <= pa18_max_angle) digitalWrite(PA18, flag_pa18_level);
    else digitalWrite(PA18, !flag_pa18_level);
  }

  // Bandera de generacion de pulso en CW y CCW
  if(new_turn > old_turn) {
    digitalWrite(PA14, HIGH);
    delayMicroseconds(turn_pulse_duration-4);
    digitalWrite(PA14, LOW);
  }
  else if(new_turn < old_turn) {
    digitalWrite(PA09, HIGH);
    delayMicroseconds(turn_pulse_duration-4);
    digitalWrite(PA09, LOW);
  }

  // Bandera para PA07, se activa dentro de un rango de vueltas
  if(pa07_enable == true) {
    if(new_turn >= pa07_min_turn && new_turn <= pa07_max_turn) {
      if(pa07_logic == HIGH) digitalWrite(PA07, HIGH);
      else digitalWrite(PA07, HIGH);
    }
    else if(pa07_logic == HIGH) digitalWrite(PA07, LOW);
    else  digitalWrite(PA07, HIGH);
  }
  else digitalWrite(PA07, LOW);

  
  // Bandera para PA06, se activa cuando el numero de vueltas es multiplo de pa06_turn_mult
  if(pa06_enable == true) {
    if(new_turn != old_turn) {
      long modulus = abs(new_turn) % pa06_turn_mult;
      if(modulus == 0) {
        if(flag_pa06_level == true) digitalWrite(PA06, HIGH);
        else digitalWrite(PA06, LOW);
      } else {
        if(flag_pa06_level == true) digitalWrite(PA06, LOW);
        else digitalWrite(PA06, HIGH);
      }
    }
  }
  else digitalWrite(PA06, LOW);
  
  // Guarda angulo y vuelta para otra comparacion
  old_turn = new_turn;
  // ---------------------------------------------------
  // Termina cambio de vuelta
  // ---------------------------------------------------

  // Manejo de Neopixels
  if(pixel_enable == true) {
    if(flag_direction == CW) {
      for(int i=0; i<PIX_NUM; i++) {
        switch(pixel_effect) {
          case SINGLE:
            if(raw_angle >= PIX_PORTION*i && raw_angle < PIX_PORTION*(i+1)) {
              switch(pixel_color) {
                case RED: pixels.setPixelColor(11-i, pixels.Color(color_bright,0,0)); break;
                case GREEN: pixels.setPixelColor(11-i, pixels.Color(0,color_bright,0)); break;
                case BLUE: pixels.setPixelColor(11-i, pixels.Color(0,0,color_bright)); break;
                case YELLOW: pixels.setPixelColor(11-i, pixels.Color(color_bright,color_bright,0)); break;
                case MAGENTA: pixels.setPixelColor(11-i, pixels.Color(color_bright,0,color_bright)); break;
                case CYAN: pixels.setPixelColor(11-i, pixels.Color(0,color_bright,color_bright)); break;
                case WHITE: pixels.setPixelColor(11-i, pixels.Color(color_bright,color_bright,color_bright)); break;
              }
            }
            else pixels.setPixelColor(11-i, pixels.Color(0,0,0));
            break;
          case FALLING:
            if(raw_angle < PIX_PORTION*i + (PIX_PORTION)/2) {
              switch(pixel_color) {
                case RED: pixels.setPixelColor(11-i, pixels.Color(color_bright,0,0)); break;
                case GREEN: pixels.setPixelColor(11-i, pixels.Color(0,color_bright,0)); break;
                case BLUE: pixels.setPixelColor(11-i, pixels.Color(0,0,color_bright)); break;
                case YELLOW: pixels.setPixelColor(11-i, pixels.Color(color_bright,color_bright,0)); break;
                case MAGENTA: pixels.setPixelColor(11-i, pixels.Color(color_bright,0,color_bright)); break;
                case CYAN: pixels.setPixelColor(11-i, pixels.Color(0,color_bright,color_bright)); break;
                case WHITE: pixels.setPixelColor(11-i, pixels.Color(color_bright,color_bright,color_bright)); break;
              }
            }
            else pixels.setPixelColor(11-i, pixels.Color(0,0,0));
            break;
          case RISING:
            if(raw_angle >= PIX_PORTION*i + (PIX_PORTION)/2) {
              switch(pixel_color) {
                case RED: pixels.setPixelColor(11-i, pixels.Color(color_bright,0,0)); break;
                case GREEN: pixels.setPixelColor(11-i, pixels.Color(0,color_bright,0)); break;
                case BLUE: pixels.setPixelColor(11-i, pixels.Color(0,0,color_bright)); break;
                case YELLOW: pixels.setPixelColor(11-i, pixels.Color(color_bright,color_bright,0)); break;
                case MAGENTA: pixels.setPixelColor(11-i, pixels.Color(color_bright,0,color_bright)); break;
                case CYAN: pixels.setPixelColor(11-i, pixels.Color(0,color_bright,color_bright)); break;
                case WHITE: pixels.setPixelColor(11-i, pixels.Color(color_bright,color_bright,color_bright)); break;
              }
            }
            else pixels.setPixelColor(11-i, pixels.Color(0,0,0));
            break;
        }
      }
    }
    else if(flag_direction == CCW) {
      for(int i=0; i<PIX_NUM; i++) {
        switch(pixel_effect) {
          case SINGLE:
            if(raw_angle >= PIX_PORTION*i && raw_angle < PIX_PORTION*(i+1)) {
              switch(pixel_color) {
                case RED: pixels.setPixelColor(i, pixels.Color(color_bright,0,0)); break;
                case GREEN: pixels.setPixelColor(i, pixels.Color(0,color_bright,0)); break;
                case BLUE: pixels.setPixelColor(i, pixels.Color(0,0,color_bright)); break;
                case YELLOW: pixels.setPixelColor(i, pixels.Color(color_bright,color_bright,0)); break;
                case MAGENTA: pixels.setPixelColor(i, pixels.Color(color_bright,0,color_bright)); break;
                case CYAN: pixels.setPixelColor(i, pixels.Color(0,color_bright,color_bright)); break;
                case WHITE: pixels.setPixelColor(i, pixels.Color(color_bright,color_bright,color_bright)); break;
              }
            }
            else pixels.setPixelColor(i, pixels.Color(0,0,0));
            break;
          case FALLING:
            if(raw_angle < PIX_PORTION*i + (PIX_PORTION)/2) {
              switch(pixel_color) {
                case RED: pixels.setPixelColor(i, pixels.Color(color_bright,0,0)); break;
                case GREEN: pixels.setPixelColor(i, pixels.Color(0,color_bright,0)); break;
                case BLUE: pixels.setPixelColor(i, pixels.Color(0,0,color_bright)); break;
                case YELLOW: pixels.setPixelColor(i, pixels.Color(color_bright,color_bright,0)); break;
                case MAGENTA: pixels.setPixelColor(i, pixels.Color(color_bright,0,color_bright)); break;
                case CYAN: pixels.setPixelColor(i, pixels.Color(0,color_bright,color_bright)); break;
                case WHITE: pixels.setPixelColor(i, pixels.Color(color_bright,color_bright,color_bright)); break;
              }
            }
            else pixels.setPixelColor(i, pixels.Color(0,0,0));
            break;
          case RISING:
            if(raw_angle >= PIX_PORTION*i + (PIX_PORTION)/2) {
              switch(pixel_color) {
                case RED: pixels.setPixelColor(i, pixels.Color(color_bright,0,0)); break;
                case GREEN: pixels.setPixelColor(i, pixels.Color(0,color_bright,0)); break;
                case BLUE: pixels.setPixelColor(i, pixels.Color(0,0,color_bright)); break;
                case YELLOW: pixels.setPixelColor(i, pixels.Color(color_bright,color_bright,0)); break;
                case MAGENTA: pixels.setPixelColor(i, pixels.Color(color_bright,0,color_bright)); break;
                case CYAN: pixels.setPixelColor(i, pixels.Color(0,color_bright,color_bright)); break;
                case WHITE: pixels.setPixelColor(i, pixels.Color(color_bright,color_bright,color_bright)); break;
              }
            }
            else pixels.setPixelColor(i, pixels.Color(0,0,0));
            break;
        }
      }
    }
    pixels.show();
  }

  // Manejo de salida analogica 0-3.3v
  analogWrite(14, map(angle, min_angle, max_angle, 0, 1023));

  // Actualizacion de angulo, no. de vueltas o ambos, de forma constante (fluid)
  if(updateMode == FLUID) {
    if(flag_angleFluid == true && flag_turnFluid == false) {
      if(input_channel == USB) Serial.println(angle);
      else Serial1.println(angle);
    }
    if(flag_angleFluid == false && flag_turnFluid == true) {
      if(input_channel == USB) Serial.println(long(turn));
      else  Serial.println(long(turn));
    }
    if(flag_angleFluid == true && flag_turnFluid == true) {
      if(input_channel == USB) {
        Serial.print("a");
        Serial.print(angle);
        Serial.print(",t");
        Serial.println(long(turn));
      } else {
        Serial1.print("a");
        Serial1.print(angle);
        Serial1.print(",t");
        Serial1.println(long(turn));
      }
    }
  }
  // Actualizacion de angulo, no. de vueltas o ambos cada que cambian de valor (change)
  else if(updateMode == CHANGE) {
    if(flag_angleFluid == true && flag_turnFluid == false && flag_angleChanged == true) {
      flag_angleChanged = false;
      if(input_channel == USB) Serial.println(angle);
      else  Serial1.println(angle);
    }
    if(flag_angleFluid == false && flag_turnFluid == true && flag_turnChanged == true) {
      flag_turnChanged = false;
      if(input_channel == USB) Serial.println(long(turn));
      else Serial1.println(long(turn));
    }
    if(flag_angleFluid == true && flag_turnFluid == true) {
      if(flag_angleChanged == true) {
        flag_angleChanged = false;
        if(input_channel == USB) {
          Serial.print("a");
          Serial.println(angle);
        } else {
          Serial1.print("a");
          Serial1.println(angle);
        }
      }
      if(flag_turnChanged == true) {
        flag_turnChanged = false;
        if(input_channel == USB) {
          Serial.print("t");
          Serial.println(long(turn));
        } else {
          Serial1.print("t");
          Serial1.println(long(turn));
        }
      }
    }
  }
  
  // Actualizacion de angulo, no. de vueltas o ambos cada que el usuario lo solicita (call)
  if(requestValue != NULL) {
    switch(requestValue) {
      case VERSION:       
        printVersion();                     
        break;
      case ANGLEMAX:      
        if(input_channel == USB) Serial.println(max_angle);          
        else Serial1.println(max_angle);  
        break;
      case ANGLEMIN:      
        if(input_channel == USB) Serial.println(min_angle);
        else Serial1.println(min_angle);
        break;
      case ANGLEFULL:     
        if(input_channel == USB) Serial.println(full_angle);
        else Serial1.println(full_angle);      
        break;  
      case ANGLE:         
        if(input_channel == USB) Serial.println(angle);
        else Serial1.println(angle);       
        break;
      case DIRECTION:       
        if(flag_direction == 0) {
          if(input_channel == USB) Serial.println("CW");
          else Serial1.println("CW");
        }
        else {
          if(input_channel == USB) Serial.println("CCW");
          else Serial1.println("CCW");
        }
        break;
      case TURN:          
        if(input_channel == USB) Serial.println(long(turn)); 
        else Serial1.println(long(turn));         
        break;
      case PA27MIN_ANGLE: 
        if(input_channel == USB) Serial.println(pa27_min_angle);
        else Serial1.println(pa27_min_angle);
        break;
      case PA27MAX_ANGLE: 
        if(input_channel == USB) Serial.println(pa27_max_angle); 
        else Serial1.println(pa27_max_angle);     
        break;
      case PA22MIN_ANGLE: 
        if(input_channel == USB) Serial.println(pa22_min_angle);
        else Serial1.println(pa22_min_angle);    
        break;
      case PA22MAX_ANGLE: 
        if(input_channel == USB) Serial.println(pa22_max_angle);     
        else Serial1.println(pa22_max_angle); 
        break;
      case PA19MIN_ANGLE: 
        if(input_channel == USB) Serial.println(pa19_min_angle);
        else Serial1.println(pa19_min_angle);   
        break;
      case PA19MAX_ANGLE: 
        if(input_channel == USB) Serial.println(pa19_max_angle);
        else Serial1.println(pa19_max_angle);
        break;
      case PA18MIN_ANGLE:   
        if(input_channel == USB) Serial.println(pa18_min_angle);     
        else Serial1.println(pa18_min_angle);  
        break;
      case PA18MAX_ANGLE: 
        if(input_channel == USB) Serial.println(pa18_max_angle);
        else Serial1.println(pa18_max_angle);      
        break;
      case PA07MIN_TURN:  
        if(input_channel == USB) Serial.println(pa07_min_turn);
        else Serial1.println(pa07_min_turn);
        break;
      case PA07MAX_TURN:  
        if(input_channel == USB) Serial.println(pa07_max_turn);
        else Serial1.println(pa07_max_turn);
        break;
      case PA06MULTIPLE:  
        if(input_channel == USB) Serial.println(int(pa06_turn_mult));
        else Serial1.println(int(pa06_turn_mult));
        break;
      case BAUD:          
        if(input_channel == USB) Serial.println(baud);               
        else Serial1.println(baud); 
        break;
    }
    requestValue = NULL;
  }
}

// Funcion que determina que comando se introdujo
void commandManage() {
  if(flag_newCommand == true) {   // si se completo un nuevo comando
    flag_newCommand = false;      // desactiva bandera de nuevo comando, usada en la recepcion serial
    // -------------------------------------------------------------------------
    // INICIO DE GET - COMANDOS DE LECTURA
    // -------------------------------------------------------------------------
    if(subCom[0] == "get") {
      if(subCom[1] == "version") {
        requestValue = VERSION;
      }
      else if(subCom[1] == "angle") {
        if(subCom[2] == "max")      requestValue = ANGLEMAX;
        else if(subCom[2] == "min") requestValue = ANGLEMIN;
        else if(subCom[2] == "full") requestValue = ANGLEFULL;
        else if(subCom[2] == NULL)  requestValue = ANGLE;
      } 
      else if(subCom[1] == "turn")  requestValue = TURN;
      else if(subCom[1] == "pa27") {
        if(subCom[2] == "min")      requestValue = PA27MIN_ANGLE;
        else if(subCom[2] == "max") requestValue = PA27MAX_ANGLE;
      }
      else if(subCom[1] == "pa22") {
        if(subCom[2] == "min")      requestValue = PA22MIN_ANGLE;
        else if(subCom[2] == "max") requestValue = PA22MAX_ANGLE;
      }
      else if(subCom[1] == "pa19") {
        if(subCom[2] == "min")      requestValue = PA19MIN_ANGLE;
        else if(subCom[2] == "max") requestValue = PA19MAX_ANGLE;
      }
      else if(subCom[1] == "pa18") {
        if(subCom[2] == "min")      requestValue = PA18MIN_ANGLE;
        else if(subCom[2] == "max") requestValue = PA18MAX_ANGLE;
      }
      else if(subCom[1] == "pa07") {
        if(subCom[2] == "min")      requestValue = PA07MIN_TURN;
        else if(subCom[2] == "max") requestValue = PA07MAX_TURN;
      }

      else if(subCom[1] == "pa06") {
        if(subCom[2] == "mult")     requestValue = PA06MULTIPLE;
      }
      else if(subCom[1] == "dir")   requestValue = DIRECTION; 
      else if(subCom[1] == "baud")  requestValue = BAUD;
      else checkErrors(1);
    }
    // -------------------------------------------------------------------------
    // FINAL GET - COMANDOS DE LECTURA
    // -------------------------------------------------------------------------
    // -------------------------------------------------------------------------
    // INICIO DE SET - COMANDOS DE CONFIGURACION
    // -------------------------------------------------------------------------
    else if(subCom[0] == "set") {
      // -------------------------------------------------------------------------
      // INICIO DE ACK - NIVEL DE ACKNOWLEDGE
      // SET.ACK.
      // -------------------------------------------------------------------------
      if(subCom[1] == "ack") {
        if(subCom[2] == "none") {
          ack_level = NO_ACK; 
          ADD_ack_level.write(ack_level);
        } else if(subCom[2] == "min") {
          ack_level = MIN_ACK;
          ADD_ack_level.write(ack_level);
          ack("ack", ack_level);
        } else if(subCom[2] == "full") {
          ack_level = FULL_ACK;
          ADD_ack_level.write(ack_level);
          ack("ack", ack_level);
        } else if(subCom[2] == "delay") {
          if(subCom[3].toInt() <= 5000 && subCom[3].toInt() >= 0) {
            ackDelay = subCom[3].toInt();
            ADD_ackDelay.write(ackDelay);
            ack("ackDelay", ackDelay);
          }
          else if(subCom[3].toInt() > 5000 || subCom[3].toInt() < 0) {
            flag_error_range = true;
          }
          else checkErrors(3);
        } 
        else checkErrors(2);
      }
      // -------------------------------------------------------------------------
      // FINAL DE ACK - NIVEL DE ACKNOWLEDGE
      // -------------------------------------------------------------------------
      // -------------------------------------------------------------------------
      // INICIO DE DIR - DIRECCION DE GIRO
      // SET.DIR.
      // ------------------------------------------------------------------------- 
      else if(subCom[1] == "dir") {
        // SET.DIR.CW
        if(subCom[2] == "cw") {   // A favor de las agujas del reloj
          flag_direction = CW;
          digitalWrite(DIR_, flag_direction);
          ADD_direction.write(flag_direction);
          ack("direction CW:", flag_direction);
        }
        // SET.DIR.CCW
        else if(subCom[2] == "ccw") {   // En contra de las agujas del reloj
          flag_direction = CCW;
          digitalWrite(DIR_, flag_direction);
          ADD_direction.write(flag_direction);
          ack("direction CCW:", flag_direction);
        }
        else checkErrors(2);
      }
      // ------------------------------------------------------------------------- 
      // FINAL DE DIR - DIRECCION DE GIRO
      // ------------------------------------------------------------------------- 
      // -------------------------------------------------------------------------
      // INICIO DE UPDATE - ACTUALIZACION DE ANGULO Y NUMERO DE VUELTAS
      // SET.UPDATE.
      // -------------------------------------------------------------------------
      else if(subCom[1] == "update") {
        // SET.UPDATE.FLUID
        if(subCom[2] == "fluid") {    // Actualizacion de datos fluida, en todo momento
          // SET.UPDATE.FLUID.ANGLE
          if(subCom[3] == "angle") {  // actualizacion del angulo
            flag_angleFluid = true;
            flag_turnFluid = false;
            updateMode = FLUID;
            ADD_flag_angleFluid.write(flag_angleFluid);
            ADD_flag_turnFluid.write(flag_turnFluid);
            ADD_updateMode.write(updateMode);
            ack("updateAngle", updateMode);
          }
          // SET.UPDATE.FLUID.TURN
          else if(subCom[3] == "turn") {  // actualizacion del numero de vueltas
            flag_angleFluid = false;
            flag_turnFluid = true;
            updateMode = FLUID;
            ADD_flag_angleFluid.write(flag_angleFluid);
            ADD_flag_turnFluid.write(flag_turnFluid);
            ADD_updateMode.write(updateMode);
            ack("updateTurn", updateMode);
          }
          // SET.UPDATE.FLUID.BOTH
          else if(subCom[3] == "both") {  // actualizacion de angulo y numero de vueltas
            flag_angleFluid = true;
            flag_turnFluid = true;
            updateMode = FLUID;
            ADD_flag_angleFluid.write(flag_angleFluid);
            ADD_flag_turnFluid.write(flag_turnFluid);
            ADD_updateMode.write(updateMode);
            ack("updateBoth", updateMode);
          }
          else checkErrors(3);
        }
        // SET.UPDATE.CHANGE
        else if(subCom[2] == "change") {  // Actualizacion de datos cada de cambian
          // SET.UPDATE.CHANGE.ANGLE
          if(subCom[3] == "angle")  // actualiza el angulo al cambiar
          {
            flag_angleFluid = true;
            flag_turnFluid = false;
            updateMode = CHANGE;
            ADD_flag_angleFluid.write(flag_angleFluid);
            ADD_flag_turnFluid.write(flag_turnFluid);
            ADD_updateMode.write(updateMode);
            ack("changeAngle", flag_angleFluid);
          }
          // SET.UPDATE.CHANGE.TURN
          else if(subCom[3] == "turn") // actualiza el numero de vueltas al cambiar
          {
            flag_angleFluid = false;
            flag_turnFluid = true;
            updateMode = CHANGE;
            ADD_flag_angleFluid.write(flag_angleFluid);
            ADD_flag_turnFluid.write(flag_turnFluid);
            ADD_updateMode.write(updateMode);
            ack("changeturn", flag_turnFluid);
          }
          // SET.UPDATE.CHANGE.BOTH
          else if(subCom[3] == "both")  // actualiza angulo y numero de vueltas al cambiar
          {
            flag_angleFluid = true;
            flag_turnFluid = true;
            updateMode = CHANGE;
            ADD_flag_angleFluid.write(flag_angleFluid);
            ADD_flag_turnFluid.write(flag_turnFluid);
            ADD_updateMode.write(updateMode);
            ack("changeBoth", flag_angleFluid);
          }
          else checkErrors(3);
        }
        // SET.UPDATE.CALL
        else if(subCom[2] == "call") {  // Manda el angulo y numero de vueltas solamente bajo llamada del usuario
          flag_angleFluid = false;
          flag_turnFluid = false;
          updateMode = CALL;
          ADD_flag_angleFluid.write(flag_angleFluid);
          ADD_flag_turnFluid.write(flag_turnFluid);
          ADD_updateMode.write(updateMode);
          ack("change", updateMode);
        }        
        else checkErrors(2);
      }
      // -------------------------------------------------------------------------
      // FINAL DE UPDATE - ACTUALIZACION DE ANGULO Y NUMERO DE VUELTAS
      // -------------------------------------------------------------------------
      // -------------------------------------------------------------------------
      // INICIO DE ANGLE - ANGULO ABSOLUTO
      // SET.ANGLE.
      // -------------------------------------------------------------------------
      else if(subCom[1] == "angle") {   // Actualiza las configuraciones referentes al angulo
        // SET.ANGLE.MIN
        if(subCom[2] == "min") {  // actualiza el angulo minimo
          if(subCom[3].toInt() >= -2047 && subCom[3].toInt() <= max_angle) { 
            min_angle = subCom[3].toInt();
            full_angle = max_angle - min_angle;
            ADD_min_angle.write(min_angle);
            ack("minAngle", min_angle);
          }
          else if(subCom[3] != NULL && (subCom[3].toInt() < -2047 || subCom[3].toInt() >= max_angle)) {
            flag_error_range = true;
          }
          else checkErrors(3);
        }
        // SET.ANGLE.MAX
        else if(subCom[2] == "max") {   // actualiza el angulo maximo
          if(subCom[3].toInt() <= 2048 && subCom[3].toInt() >= min_angle) {
            max_angle = subCom[3].toInt();
            full_angle = max_angle - min_angle;
            ADD_max_angle.write(max_angle);            
            ack("maxAngle", max_angle);
          }
          else if(subCom[3] != NULL && (subCom[3].toInt() > 2048 || subCom[3].toInt() <= min_angle)) {
            flag_error_range = true;
          }
          else checkErrors(3);
        }
        else checkErrors(2);
      }
      // -------------------------------------------------------------------------
      // FINAL DE ANGLE - ANGULO ABSOLUTO
      // -------------------------------------------------------------------------
      // -------------------------------------------------------------------------
      // INICIO DE TURN - NUMERO DE VUELTAS
      // SET.TURN.
      // -------------------------------------------------------------------------
      else if(subCom[1] == "turn") {
        // SET.TURN.PULSE
        if(subCom[2] == "pulse") {  // configura el tiempo del pulso en los pines PA14 y PA09
          if(subCom[3].toInt() >= 5 && subCom[3].toInt() <= 1000) {
            turn_pulse_duration = subCom[3].toInt();
            ADD_turn_pulse_duration.write(turn_pulse_duration);   
            ack("pulseDuration", turn_pulse_duration);
          }
          else if(subCom[3].toInt() < 5 || subCom[3].toInt() > 1000) {
            flag_error_range = true;
          }
          else checkErrors(3);
        }
        else if(subCom[2] != NULL && subCom[2].toInt() > -2147483648 && subCom[2].toInt() < 2147483647) {
          turn = long(subCom[2].toInt());
          ADD_turn.write(turn);
          ack("turn", turn);
        }
        else if(subCom[2] != NULL && (subCom[2].toInt() <= -2147483648 || subCom[2].toInt() >= 2147483647)) {
          flag_error_range = true;
        }
        else checkErrors(2);
      }
      // -------------------------------------------------------------------------
      // FINAL DE TURN - NUMERO DE VUELTAS
      // -------------------------------------------------------------------------
      // -------------------------------------------------------------------------
      // INICIO DE VELOCIDAD SERIAL BAUDRATE
      // SET.BAUD.
      // -------------------------------------------------------------------------
      else if(subCom[1] == "baud") {  // Configura la velocidad en baudios de la velocidad
        if(isDigit(subCom[2].charAt(0)) && isDigit(subCom[2].charAt(1)) && 
           isDigit(subCom[2].charAt(2)) && isDigit(subCom[2].charAt(3))) {
          baud = subCom[2].toInt();
          if(subCom[2] == "9600") {
            if(input_channel == USB) {
              Serial.println("9600 - OK");
              Serial.begin(baud);
            } else {
              Serial1.println("9600 - OK");
              Serial1.begin(baud);
            }
          } else if(subCom[2] == "19200") {
            if(input_channel == USB) {
              Serial.println("19200 - OK");
              Serial.begin(baud);
            } else {
              Serial1.println("19200 - OK");
              Serial1.begin(baud);
            }
          } else if(subCom[2] == "38400") {
            if(input_channel == USB) {
              Serial.println("38400 - OK");
              Serial.begin(baud);
            } else {
              Serial1.println("38400 - OK");
              Serial1.begin(baud);
            }
          } else if(subCom[2] == "57600") {
            if(input_channel == USB) {
              Serial.println("57600 - OK");
              Serial.begin(baud);
            } else {
              Serial1.println("57600 - OK");
              Serial1.begin(baud);
            }
          } else if(subCom[2] == "74880") {
            if(input_channel == USB) {
              Serial.println("74880 - OK");
              Serial.begin(baud);
            } else {
              Serial1.println("74880 - OK");
              Serial1.begin(baud);
            }
          } else if(subCom[2] == "115200") {
            if(input_channel == USB) {
              Serial.println("115200 - OK");
              Serial.begin(baud);
            } else {
              Serial1.println("115200 - OK");
              Serial1.begin(baud);
            }
          } else if(subCom[2] == "230400") {
            if(input_channel == USB) {
              Serial.println("230400 - OK");
              Serial.begin(baud);
            } else {
              Serial1.println("230400 - OK");
              Serial1.begin(baud);
            }
          } else if(subCom[2] == "250000") {
            if(input_channel == USB) {
              Serial.println("250000 - OK");
              Serial.begin(baud);
            } else {
              Serial1.println("250000 - OK");
              Serial1.begin(baud);
            }
          } else if(subCom[2] == "500000") {
            if(input_channel == USB) {
              Serial.println("500000 - OK");
              Serial.begin(baud);
            } else {
              Serial1.println("500000 - OK");
              Serial1.begin(baud);
            }
          }
          ADD_baud.write(baud);
          ack("baudrate", baud);
        }
        else checkErrors(2);
      }
      // -------------------------------------------------------------------------
      // FINAL DE VELOCIDAD SERIAL BAUDRATE
      // -------------------------------------------------------------------------
      // -------------------------------------------------------------------------
      // INICIO DE SALIDA PA27
      // -------------------------------------------------------------------------
      else if(subCom[1] == "pa27") {
        if(subCom[2] == "high") {
          flag_pa27_level = HIGH;
          ADD_pa27_logic.write(flag_pa27_level);
          ack("PA27level", flag_pa27_level);
        } else if(subCom[2] == "low") {
          flag_pa27_level = LOW;
          ADD_pa27_logic.write(flag_pa27_level);
          ack("PA27level", flag_pa27_level);
        }
        else if(subCom[2] == "min") {
          if(subCom[3].toInt() >= min_angle) {
            pa27_min_angle = subCom[3].toInt();
            ADD_pa27_min_angle.write(pa27_min_angle);
            ack("PA27minAngle", pa27_min_angle);
          }
          else checkErrors(3);
        } 
        else if(subCom[2] == "max") {
          if(subCom[3].toInt() <= max_angle) {
            pa27_max_angle = subCom[3].toInt();
            ADD_pa27_max_angle.write(pa27_max_angle);
            ack("PA27maxAngle", pa27_max_angle);
          }
          else checkErrors(3);
        } 
        else if(subCom[2] == "enable") {
          pa27_enable = true;
          ADD_pa27_enable.write(pa27_enable);
          ack("PA27status", pa27_enable);
        } 
        else if(subCom[2] == "disable") {
          pa27_enable = false;
          ADD_pa27_enable.write(pa27_enable);
          ack("PA27status", pa27_enable);
        }
        else checkErrors(2);
      }
      // -------------------------------------------------------------------------
      // FINAL DE SALIDA PA27
      // -------------------------------------------------------------------------
      // -------------------------------------------------------------------------
      // INICIO DE SALIDA PA22
      // -------------------------------------------------------------------------
      else if(subCom[1] == "pa22") {
        if(subCom[2] == "high") {
          flag_pa22_level = HIGH;
          ADD_pa22_logic.write(flag_pa22_level);
          ack("PA22level", flag_pa22_level);
        } else if(subCom[2] == "low") {
          flag_pa22_level = LOW;
          ADD_pa22_logic.write(flag_pa22_level);
          ack("PA22level", flag_pa22_level);
        }
        else if(subCom[2] == "min") {
          if(subCom[3].toInt() >= min_angle) {
            pa22_min_angle = subCom[3].toInt();
            ADD_pa22_min_angle.write(pa22_min_angle);
            ack("PA22minAngle", pa22_min_angle);
          }
          else checkErrors(3);
        } 
        else if(subCom[2] == "max") {
          if(subCom[3].toInt() <= max_angle) {
            pa22_max_angle = subCom[3].toInt();
            ADD_pa22_max_angle.write(pa22_max_angle);
            ack("PA22maxAngle", pa22_max_angle);
          }
          else checkErrors(3);
        } 
        else if(subCom[2] == "enable") {
          pa22_enable = true;
          ADD_pa27_enable.write(pa22_enable);
          ack("PA22status", pa22_enable);
        } 
        else if(subCom[2] == "disable") {
          pa22_enable = false;
          ADD_pa22_enable.write(pa22_enable);
          ack("PA227status", pa22_enable);
        }
        else checkErrors(2);
      }
      // -------------------------------------------------------------------------
      // FINAL DE SALIDA PA22
      // -------------------------------------------------------------------------
      // -------------------------------------------------------------------------
      // INICIO DE SALIDA PA19
      // -------------------------------------------------------------------------
      else if(subCom[1] == "pa19") {
        if(subCom[2] == "high") {
          flag_pa19_level = HIGH;
          ADD_pa19_logic.write(flag_pa19_level);
          ack("PA19level", flag_pa19_level);
        } else if(subCom[2] == "low") {
          flag_pa19_level = LOW;
          ADD_pa19_logic.write(flag_pa19_level);
          ack("PA19level", flag_pa19_level);
        }
        else if(subCom[2] == "min") {
          if(subCom[3].toInt() >= min_angle) {
            pa19_min_angle = subCom[3].toInt();
            ADD_pa19_min_angle.write(pa19_min_angle);
            ack("PA19minAngle", pa19_min_angle);
          }
          else checkErrors(3);
        } 
        else if(subCom[2] == "max") {
          if(subCom[3].toInt() <= max_angle) {
            pa19_max_angle = subCom[3].toInt();
            ADD_pa19_max_angle.write(pa19_max_angle);
            ack("PA19maxAngle", pa19_max_angle);
          }
          else checkErrors(3);
        } 
        else if(subCom[2] == "enable") {
          pa19_enable = true;
          ADD_pa19_enable.write(pa19_enable);
          ack("PA19status", pa19_enable);
        } 
        else if(subCom[2] == "disable") {
          pa19_enable = false;
          ADD_pa19_enable.write(pa19_enable);
          ack("PA19status", pa19_enable);
        }
        else checkErrors(2);
      }
      // -------------------------------------------------------------------------
      // FINAL DE SALIDA PA19
      // -------------------------------------------------------------------------
      // -------------------------------------------------------------------------
      // INICIO DE SALIDA PA18
      // -------------------------------------------------------------------------
      else if(subCom[1] == "pa18") {
        if(subCom[2] == "high") {
          flag_pa18_level = HIGH;
          ADD_pa18_logic.write(flag_pa18_level);
          ack("PA18level", flag_pa18_level);
        } else if(subCom[2] == "low") {
          flag_pa18_level = LOW;
          ADD_pa18_logic.write(flag_pa18_level);
          ack("PA18level", flag_pa18_level);
        }
        else if(subCom[2] == "min") {
          if(subCom[3].toInt() >= min_angle) {
            pa18_min_angle = subCom[3].toInt();
            ADD_pa18_min_angle.write(pa18_min_angle);
            ack("PA18minAngle", pa18_min_angle);
          }
          else checkErrors(3);
        } 
        else if(subCom[2] == "max") {
          if(subCom[3].toInt() <= max_angle) {
            pa18_max_angle = subCom[3].toInt();
            ADD_pa18_max_angle.write(pa18_max_angle);
            ack("PA18maxAngle", pa18_max_angle);
          }
          else checkErrors(3);
        } 
        else if(subCom[2] == "enable") {
          pa18_enable = true;
          ADD_pa18_enable.write(pa27_enable);
          ack("PA18status", pa18_enable);
        } 
        else if(subCom[2] == "disable") {
          pa18_enable = false;
          ADD_pa18_enable.write(pa18_enable);
          ack("PA18status", pa18_enable);
        }
        else checkErrors(2);
      }
      // -------------------------------------------------------------------------
      // FINAL DE SALIDA PA18
      // -------------------------------------------------------------------------
      // -------------------------------------------------------------------------
      // INICIO DE SALIDA PA07
      // -------------------------------------------------------------------------
      else if(subCom[1] == "pa07") {
        if(subCom[2] == "high") {
          flag_pa07_level = HIGH;
          ADD_pa07_logic.write(flag_pa07_level);
          ack("PA07level", flag_pa07_level);
        } else if(subCom[2] == "low") {
          flag_pa07_level = LOW;
          ADD_pa07_logic.write(flag_pa07_level);
          ack("PA07level", flag_pa07_level);
        }
        else if(subCom[2] == "min") {
          if(subCom[3].toInt() >= min_turn) {
            pa07_min_turn = subCom[3].toInt();
            ADD_pa07_min_turn.write(pa07_min_turn);
            ack("PA07minTurn", pa07_min_turn);
          }
          else checkErrors(3);
        } 
        else if(subCom[2] == "max") {
          if(subCom[3].toInt() <= max_turn) {
            pa07_max_turn = subCom[3].toInt();
            ADD_pa07_max_turn.write(pa07_max_turn);
            ack("PA18maxTurn", pa07_max_turn);
          }
          else checkErrors(3);
        } 
        else if(subCom[2] == "enable") {
          pa07_enable = true;
          ADD_pa07_enable.write(pa07_enable);
          ack("PA07status", pa07_enable);
        } 
        else if(subCom[2] == "disable") {
          pa07_enable = false;
          ADD_pa07_enable.write(pa07_enable);
          ack("PA07status", pa07_enable);
        }
        else checkErrors(2);
      }
      // -------------------------------------------------------------------------
      // FINAL DE SALIDA PA07
      // -------------------------------------------------------------------------
      // -------------------------------------------------------------------------
      // INICIO DE SALIDA PA06
      // -------------------------------------------------------------------------
      else if(subCom[1] == "pa06") {
        if(subCom[2] == "high") {
          flag_pa06_level = HIGH;
          ADD_pa06_logic.write(flag_pa06_level);
          ack("PA06level", flag_pa06_level);
        } else if(subCom[2] == "low") {
          flag_pa06_level = LOW;
          ADD_pa06_logic.write(flag_pa06_level);
          ack("PA06level", flag_pa06_level);
        }
        else if(subCom[2] == "mult") {
          if(subCom[3].toInt() >= min_turn && subCom[3].toInt() <= max_turn) {
            pa06_turn_mult = subCom[3].toInt();
            ADD_pa06_turn_mult.write(pa06_turn_mult);
            ack("PA06Multiple", pa07_min_turn);
          }
          else checkErrors(3);
        } 
        else if(subCom[2] == "enable") {
          pa06_enable = true;
          ADD_pa06_enable.write(pa06_enable);
          ack("PA06status", pa06_enable);
        } 
        else if(subCom[2] == "disable") {
          pa06_enable = false;
          ADD_pa06_enable.write(pa06_enable);
          ack("PA06status", pa06_enable);
        }
        else checkErrors(2);
      }
      // -------------------------------------------------------------------------
      // FINAL DE SALIDA PA06
      // -------------------------------------------------------------------------
      // -------------------------------------------------------------------------
      // INICIO DE PIXEL - CONFIGURACION DE NEOPIXELS
      // -------------------------------------------------------------------------
      else if(subCom[1] == "pixel") {   // Configura todo lo relacionado a los neopixels
        // SET.PIXEL.BRIGHT
        if(subCom[2] == "bright") {   // configura el brillo actual
          color_bright = subCom[3].toInt();
          ADD_color_bright.write(color_bright);
          ack("bright", color_bright);
        }
        // SET.PIXEL.EFFECT
        else if(subCom[2] == "effect") {  // configura el tipo de efecto de los neopixels
          if(subCom[3] == "single") {
            pixel_effect = SINGLE;
            ADD_pixel_effect.write(pixel_effect);
            ack("effectSingle", pixel_effect);
          }
          else if(subCom[3] == "rise") {
            pixel_effect = RISING;
            ADD_pixel_effect.write(pixel_effect);
            ack("effectRising", pixel_effect);
          }
          else if(subCom[3] == "fall") {
            pixel_effect = FALLING;
            ADD_pixel_effect.write(pixel_effect);
            ack("effectFalling", pixel_effect);
          }
          else checkErrors(3);
        }
        // SET.PIXEL.COLOR
        else if(subCom[2] == "color") { // configura el color de todos los neopixels
          // SET.PIXEL.COLOR.RED
          if(subCom[3] == "red") {  // color rojo
            pixel_color = RED;
            ADD_pixel_color.write(pixel_color);
            ack("color-red", pixel_color);
          }
          // SET.PIXEL.COLOR.GREEN
          else if(subCom[3] == "green") {  // color verde
            pixel_color = GREEN;
            ADD_pixel_color.write(pixel_color);
            ack("color-green", pixel_color);
          }
          // SET.PIXEL.COLOR.BLUE
          else if(subCom[3] == "blue") {  // color azul
            pixel_color = BLUE;
            ADD_pixel_color.write(pixel_color);
            ack("color-blue", pixel_color);
          }
          // SET.PIXEL.COLOR.YELLOW
          else if(subCom[3] == "yellow") {  // color amarillo
            pixel_color = YELLOW;
            ADD_pixel_color.write(pixel_color);
            ack("color-yellow", pixel_color);
          }
          // SET.PIXEL.COLOR.MAGENTA
          else if(subCom[3] == "magenta") {  // color magenta
            pixel_color = MAGENTA;
            ADD_pixel_color.write(pixel_color);
            ack("color-magenta", pixel_color);
          }
          // SET.PIXEL.COLOR.CYAN
          else if(subCom[3] == "cyan") {  // color cian
            pixel_color = CYAN;
            ADD_pixel_color.write(pixel_color);
            ack("color-cyan", pixel_color);
          }
          // SET.PIXEL.COLOR.WHITE
          else if(subCom[3] == "white") {  // color blanco
            pixel_color = WHITE;
            ADD_pixel_color.write(pixel_color);
            ack("color-white", pixel_color);
          }
          else checkErrors(3);
        }
        // SET.PIXEL.ENABLE
        else if(subCom[2] == "enable") {  // habilita los neopixels
          pixel_enable = true;
          ADD_pixel_enable.write(pixel_enable);
          ack("pixelStatus", pixel_enable);
        }
        // SET.PIXEL.DISABLE
        else if(subCom[2] == "disable") {  // deshabilita los neopixels
          pixel_enable = false;
          ADD_pixel_enable.write(pixel_enable);
          ack("pixelStatus", pixel_enable); 
          for(int i=0; i<PIX_NUM; i++) pixels.setPixelColor(i, pixels.Color(0,0,0));
          pixels.show();
        }
        else checkErrors(2);
      }
      // -------------------------------------------------------------------------
      // FINAL DE PIXEL - CONFIGURACION DE NEOPIXELS
      // -------------------------------------------------------------------------
      else checkErrors(1);
    }
    // COMANDO DIRECTO STOP
    else if(subCom[0] == "stop") {
      flag_angleFluid = false;
      flag_turnFluid = false;
      updateMode = CHANGE;
      ADD_flag_angleFluid.write(flag_angleFluid);
      ADD_flag_turnFluid.write(flag_turnFluid);
      ADD_updateMode.write(updateMode);
      ack("continuos mode", flag_angleFluid);
    }
    // COMANDO DIRECTO RESET
    else if(subCom[0] == "reset") {
      ack_level = FULL_ACK; 
      ackDelay = 100;
      ADD_ackDelay.write(ackDelay);
      flag_direction = CW;
      digitalWrite(DIR_, flag_direction);
      ADD_direction.write(flag_direction);
      flag_angleFluid = true;
      flag_turnFluid = true;
      updateMode = CHANGE;
      ADD_flag_angleFluid.write(flag_angleFluid);
      ADD_flag_turnFluid.write(flag_turnFluid);
      ADD_updateMode.write(updateMode);
      min_angle = 0;
      max_angle = 360;
      full_angle = max_angle - min_angle;
      ADD_min_angle.write(min_angle);
      ADD_max_angle.write(max_angle);
      turn_pulse_duration = 5;
      ADD_turn_pulse_duration.write(turn_pulse_duration);
      turn = 0;
      ADD_turn.write(turn);
      baud = 9600;
      ADD_baud.write(baud);
    
      flag_pa27_level = HIGH;
      ADD_pa27_logic.write(flag_pa27_level);
      pa27_min_angle = 0;
      ADD_pa27_min_angle.write(pa27_min_angle);
      pa27_max_angle = 90;
      ADD_pa27_max_angle.write(pa27_max_angle);
      pa27_enable = true;
      ADD_pa27_enable.write(pa27_enable);
    
      flag_pa22_level = HIGH;
      ADD_pa22_logic.write(flag_pa22_level);
      pa22_min_angle = 90;
      ADD_pa22_min_angle.write(pa22_min_angle);
      pa22_max_angle = 180;
      ADD_pa22_max_angle.write(pa22_max_angle);
      pa22_enable = true;
      ADD_pa22_enable.write(pa22_enable);
    
      flag_pa19_level = HIGH;
      ADD_pa19_logic.write(flag_pa19_level);
      pa19_min_angle = 180;
      ADD_pa19_min_angle.write(pa19_min_angle);
      pa19_max_angle = 270;
      ADD_pa19_max_angle.write(pa19_max_angle);
      pa19_enable = true;
      ADD_pa19_enable.write(pa19_enable);
    
      flag_pa18_level = HIGH;
      ADD_pa18_logic.write(flag_pa18_level);
      pa18_min_angle = 270;
      ADD_pa18_min_angle.write(pa18_min_angle);
      pa18_max_angle = 360;
      ADD_pa18_max_angle.write(pa18_max_angle);
      pa18_enable = true;
      ADD_pa18_enable.write(pa18_enable);

      flag_pa07_level = HIGH;
      ADD_pa07_logic.write(flag_pa07_level);
      pa07_min_turn = 0;
      ADD_pa07_min_turn.write(pa07_min_turn);
      pa07_max_turn = 10;
      ADD_pa07_max_turn.write(pa07_max_turn);
      pa07_enable = true;
      ADD_pa07_enable.write(pa07_enable);
    
      flag_pa06_level = HIGH;
      ADD_pa06_logic.write(flag_pa06_level);
      pa06_turn_mult = 10;
      ADD_pa06_turn_mult.write(pa06_turn_mult);
      pa06_enable = true;
      ADD_pa06_enable.write(pa06_enable);
    
      color_bright = 255;
      ADD_color_bright.write(color_bright);
      pixel_effect = SINGLE;
      ADD_pixel_effect.write(pixel_effect);
      pixel_color = RED;
      ADD_pixel_color.write(pixel_color);
      pixel_enable = true;
      ADD_pixel_enable.write(pixel_enable);

      ack("Configuracion restablecida de fabrica", 1);
    }
    // -------------------------------------------------------------------------
    // FINAL DE SET - COMANDOS DE CONFIGURACION
    // -------------------------------------------------------------------------
    // -------------------------------------------------------------------------
    // INICIO DE SAVE - COMANDO PARA GUARDAR
    // SAVE.
    // -------------------------------------------------------------------------
    else if(subCom[0] == "save") {  // Comando para guardar un registro
      if(subCom[1] == "baud") {   // guarda la velocidad de la comunicacion serial
        ADD_baud.write(baud);
        ack("saveBaudrate", baud);
      }
      else if(subCom[1] == "turn") {
        ADD_turn.write(turn);
        ack("SaveTurnNumber", turn);
      }
      else checkErrors(1);
    }
    // -------------------------------------------------------------------------
    // FINAL DE SAVE - COMANDO PARA GUARDAR
    // -------------------------------------------------------------------------
    // -------------------------------------------------------------------------
    // INICIO DE INFO - IMPRIME TODOS LOS COMANDOS DISPONIBLES POR SERIAL
    // INFO
    // -------------------------------------------------------------------------
    else if(subCom[0] == "info") {
      updateMode = CALL;
      print_instructions();
    }
    else checkErrors(0);
  }
}

// Funcion de configuracion global de la placa
void pmxas5600n12_configure() {
  load_values();            // Carga los valores guardados en Flash
  config_io();              // Configura las entradas/salidas
  Wire.begin();             // Inicializa la comunicacion I2C con el sensor 
  Wire.setClock(I2CSPEED);  // Configura la velocidad de I2C a fast mode 1mbit/s
  Serial.begin(baud);       // Inicializa comunicacion serial por USB
  Serial1.begin(baud);
  delay(1500);
  printVersion();           // Imprime la version del firmware
  configurePWM();           // Configura el pin, timer, frecuencia, periodo, del PWM en el pin PA05
  pixels.begin();           // Inicializa los 12 neopixels de la placa
  printVersion();           // Imprime la version de la placa
}

// Funcion que agrupa todas las funciones de control de la placa
void pmxas5600n12_exe() {
  getCommand();     // Revisa si hay un nuevo comando entrante por el serial configurado
  commandManage();  // Ejecuta los comandos
  changeManage();   // Gestiona los cambios que se hacen a traves de comandos
  errorManage();    // Revisa si hay errores e imprime de haberlos
}