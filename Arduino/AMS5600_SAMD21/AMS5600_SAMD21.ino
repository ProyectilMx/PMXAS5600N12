
/*
 * PMXAS5600N12 - Version 1.1 - 28/02/2019
 * Modulo de Medicion Angular Absoluta Magnetica, programable mediante comandos UART
 * La velocidad por defecto es de 9600 baudios
 * 
 * Esta provisto de un sensor de la marca AMS, el AS5600, que es un sensor
 * magnetico rotatorio de posicion con una resolucion de 12 bits, para un
 * rango por hardware de 4095°, el programa provee de otros 4095° para un
 * rango maximo total de -4095 a 4095°. Por defecto, el sitema viene en un
 * rango de 0 a 360°.
 * 
 * La placa contiene un microcontrolador maestro, basado en un ATSAMD21E18
 * de 32 bits, para realizar todas las mediciones y calculos a una velocidad 
 * de 96MHz, con este rapido sistema se logran obtener mediciones de hasta
 * 6000RPM con 100% de presicion, tanto en revoluciones como en angulo.
 * La placa tambien consta de varios pines de salida para obtener funcionalidades
 * diversas:
 * 
 * PA27 - PA18:
 * Salidas programables en rangos angulares, programables en nivel alto o
 * bajo dentro de los mismos. Por defecto vienen con los siguientes limites:
 * PA27 -> de 0° a 90°
 * PA22 -> de 90° a 180°
 * PA19 -> de 180° a 270°
 * PA18 -> de 270° a 360°.
 * Con estas salidas se pueden activar subsistemas dentro de cierto angulo.
 * 
 * PA14, PA09:
 * Salidas que generan un pulso de 10us con cada revolucion. 
 * PA14 -> genera un pulso en el sentido de las manecillas del reloj. 
 * PA09 -> genera un pulso en el sentido contrario a las manecillas del reloj.
 * 
 * PA07:
 * Se puede configurar para activarse dentro de un rango determinado de vueltas. 
 * Por ejemplo, si se configura que PA07 este en HIGH cuando se este entre 
 * 0 y 500 vueltas, el pulso cambiara a LOW si el numero de vueltas esta fuera 
 * de ese intervalo.
 * 
 * PA06:
 * Se puede configurar para que al llegar a un multiplo de vueltas, por ejemplo, 15,
 * cambie a un estado bajo o alto, por lo tanto este pulso sucederia durante la vuelta 
 * 0, 15, 30, 45, 60, 75, etc.
 * 
 * PA05:
 * Genera una señal PWM con una frecuencia de 2.5KHz, con un ciclo de trabajo 
 * configurable entre 0 - 100%, que dependera de la posicion angular.
 * 
 * PA02:
 * Genera una señal analogica de 10 bits, de entre 0 y 3.3v, cuyo valor dependera 
 * de la posicion angular.
 * 
 * PA04, PA03, PA01, PA00.
 * Salidas/Entradas: 
 * Estos pines solamente pueden ser entradas o salidas digitales.
 * 
 * Las funciones de la placa que se encuentran disponibles para el programa principal son:
 * 
 * void pmxas5600n12_configure(void);    - Configura todos los perifericos de la placa
 * void pmxas5600n12_exe(void);          - Ejecuta todas las acciones de control pertinentes a la placa
 * 
 * Se pueden utilizar las siguiente variables en el programa principal para interactuar con la placa:
 * 
 * user_turn       -  Reporta el numero de vueltas actual 
 * user_angle      -  Reporta el angulo actual en 12 bits (0 - 4095)
 */

#include "PMXAS5600N12.h"

void setup() {
  pmxas5600n12_configure();   // Configura todos los perifericos de la placa
  
  /* Si deseas utilizar los pines PA04, PA03, PA01, PA00, deberas descomentarlos y
   * declararlos como entradas o salidas a continuacion
  
  pinMode(PA00, OUTPUT);
  pinMode(PA01, INPUT);
  pinMode(PA03, INPUT);
  pinMode(PA04, INPUT);*/
}

void loop() {
  pmxas5600n12_exe();   // Ejecuta todas las acciones de control pertinentes a la placa

  /* esta linea imprime angulo y vueltas, asegurate que la placa no este reportando nada por 
   * serial antes de descomentarla. Puedes desactivar el envio constante con set,update,call;
   */
  //Serial.println("Vueltas: " + String(user_turn) + ", Angulo: " + String(user_angle));
  
  /*
   * Tu codigo aqui
   */
}