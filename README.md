# PMXAS5600N12
Encoder Absoluto basado en el integrado AS5600 con 12 neopixels, configurable a través de COM Serial

PMXAS5600N12 version 1.0 - 28/02/2019
PROYECTIL MX - HAZLO REAL

Modulo de Medicion Angular Absoluta Magnetica, programable mediante comandos UART

Esta provisto de un sensor de la marca AMS, el AS5600, que es un sensor
magnetico rotatorio de posicion con una resolucion de 12 bits, para un
rango por hardware de 4095°, el programa provee de otros 4095° para un
rango maximo total de -4095 a 4095°. Por defecto, el sitema viene en un
rango de 0 a 360°.

La placa contiene un microcontrolador maestro, basado en un ATSAMD21E18
de 32 bits, para realizar todas las mediciones y calculos a una velocidad 
de 96MHz, con este rapido sistema se logran obtener mediciones de hasta
6000RPM con 100% de presicion, tanto en revoluciones como en angulo.
La placa tambien consta de varios pines de salida para obtener funcionalidades
diversas:

PA27 - PA18:
Salidas programables en rangos angulares, programables en nivel alto o
bajo dentro de los mismos. Por defecto vienen con los siguientes limites:
PA27 -> de 0° a 90°
PA22 -> de 90° a 180°
PA19 -> de 180° a 270°
PA18 -> de 270° a 360°.
Con estas salidas se puede emular un registro de corrimiento de 4 bits o
activar subsistemas dentro de cierto angulo.

PA14, PA09:
Salidas que generan un pulso de 10us con cada revolucion. 
PA14 -> genera un pulso en el sentido de las manecillas del reloj. 
PA09 -> genera un pulso en el sentido contrario a las manecillas del reloj.

PA05:
Genera una señal PWM con una frecuencia variable entre 1 KHz y 4MHz, con un
ciclo de trabajo configurable entre 0 - 100%, dependiendo de la posicion angular.

PA02:
Genera una señal analogica de 10 bits, de entre 0 y 3.3v aproximadamente, 
dependiendo de la posicion angular.

PA07, PA06, PA04, PA03, PA01, PA00.
Salidas/Entradas: 
Estos pines solamente pueden ser entradas o salidas digitales.

Las funciones de la placa que se encuentran disponibles para el usuraio son:

void pmxas5600n12_configure(void);    - Configura todos los perifericos de la placa
void pmxas5600n12_exe(void);          - Ejecuta todas las acciones de control pertinentes a la placa

#Instrucciones

Hay 3 comandos principales:
set  - Escribe en la memoria de la tarjeta un nuevo valor
get  - Lee desde la memoria de la tarjeta un valor existente
save - salva un valor en memoria
Para que se considere un comando valido, debe terminar con nueva linea (\n)

Comandos GET Para lectura de valores:
get.registro
get.registro.subregistro
donde: 'registro'    es la variable a leer.
       'subregistro' puede ser un subregistro de registro.
       
Comandos SET para cambiar la configuracion:
set.registro
set.registro.valor
set.registro.subregistro.valor
donde: 'registro'    es la variable a configurar.
       'subregistro' puede ser un subregistro de registro.
       'valor'       es el valor que se escribira en ese registro.
                       y puede tomar valores positivos y negativos o tener pseudonimos.
                       
Ejemplo de comandos set:
set.angle.min.-180\n -> configura el angulo minimo en -180
set.angle.max.180\n  -> configura el angulo maximo en 180
set.turn.10\n        -> reinicia el contador de vueltas a 10
set.pa27.max.100\n  -> configura el angulo maximo de PA27 en 100

Lista de comandos get:
get.version     -> devuelve la version del firmware
get.angle       -> devuelve el valor del angulo actual.
get.angle.max   -> devuelve el valor del angulo maximo.
get.angle.min   -> devuelve el valor del angulo minimo.
get.angle.full  -> devuelve el valor de angulo total (max - min)
get.turn        -> devuelve el numero de vueltas que ha dado el dial (positivo o negativo)
get.pa27.min    -> devuelve el valor del angulo minimo para la salida PA27.
get.pa27.max    -> devuelve el valor del angulo maximo para la salida PA27.
get.pa22.min    -> devuelve el valor del angulo minimo para la salida PA27.
get.pa22.max    -> devuelve el valor del angulo maximo para la salida PA27.
get.pa19.min    -> devuelve el valor del angulo minimo para la salida PA27.
get.pa19.max    -> devuelve el valor del angulo maximo para la salida PA27.
get.pa18.min    -> devuelve el valor del angulo minimo para la salida PA27.
get.pa18.max    -> devuelve el valor del angulo maximo para la salida PA27.
get.dir         -> devuelve la direccion de incremento al girar el dial (CW o CCW)
get.baud        -> devuelve la velocidad de comunicacion en baudios.

Lista de comandos set:
set.ack.none             -> Configura tipo de acknowledge a ninguno
set.ack.ok               -> Configura tipo de acknowledge a ok
set.ack.full             -> Configura tipo de acknowledge a detallado
set.ack.delay            -> Tiempo de espera despues del acknowledge
set.dir.cw               -> Direccion de incremento a favor de las manecillas
set.dir.ccw              -> Direccion de incremento en contra de las manecillas
set.update.fluid.angle   -> Impresion constante de angulo
set.update.fluid.turn    -> Impresion constante de numero de vuelta
set.update.fluid.both    -> Impresion constante de angulo y numero de vuelta
set.update.change.angle  -> Impresion de angulo en cuanto cambia de valor
set.update.change.turn   -> Impresion de numero de vuelta en cuanto cambia de valor
set.update.change.both   -> Impresion de ambas variables en cuanto camban de valor
set.update.call          -> Configura el modo de impresion bajo consulta
set.angle.min.value      -> Configura el angulo minimo (value = -4095 - 4094)
set.angle.max.value      -> Configura el angulo maximo (value = -4094 - 4095)
set.turn.value           -> Configura el valor actual de vueltas (value = -2147483648 - 2147483647)
set.turn.pulse.value     -> Configura la duracion del pulso por revolucion en PA14 y PA09 (value = 5uS - 100uS)
set.baud.value           -> configura la velocidad de comunicacion en baudios, value puede ser:
                            9600, 19200, 38400, 57600, 74880, 115200, 230400 ,250000 o 500000
set.pa27.min.value       -> configura el angulo minimo para la salida PA27 (value = minAngle - maxAngle)
set.pa27.max.value       -> configura el angulo maximo para la salida PA27 (value = minAngle - maxAngle)
set.pa27.high            -> configura en alto la zona de accion de PA27 entre dos angulos
set.pa27.low             -> configura en bajo la zona de accion de PA27 entre dos angulos
set.pa27.enable          -> habilita el PA27 para su uso acotado entre 2 angulos
set.pa27.disable         -> deshabilita el PA27 para su uso acotado entre 2 angulos
set.pa22.min.value       -> configura el angulo minimo para la salida PA22 (value = minAngle - maxAngle)
set.pa22.max.value       -> configura el angulo maximo para la salida PA22 (value = minAngle - maxAngle)
set.pa22.high            -> configura en alto la zona de accion de PA22 entre dos angulos
set.pa22.low             -> configura en bajo la zona de accion de PA22 entre dos angulos
set.pa22.enable          -> habilita el PA22 para su uso acotado entre 2 angulos
set.pa22.disable         -> deshabilita el PA22 para su uso acotado entre 2 angulos
set.pa19.min.value       -> configura el angulo minimo para la salida PA19 (value = minAngle - maxAngle)
set.pa19.max.value       -> configura el angulo maximo para la salida PA19 (value = minAngle - maxAngle)
set.pa19.high            -> configura en alto la zona de accion de PA19 entre dos angulos
set.pa19.low             -> configura en bajo la zona de accion de PA19 entre dos angulos
set.pa19.enable          -> habilita el PA19 para su uso acotado entre 2 angulos
set.pa19.disable         -> deshabilita el PA19 para su uso acotado entre 2 angulos
set.pa18.min.value       -> configura el angulo minimo para la salida PA18 (value = minAngle - maxAngle)
set.pa18.max.value       -> configura el angulo maximo para la salida PA18 (value = minAngle - maxAngle)
set.pa18.high            -> configura en alto la zona de accion de PA18 entre dos angulos
set.pa18.low             -> configura en bajo la zona de accion de PA18 entre dos angulos
set.pa18.enable          -> habilita el PA18 para su uso acotado entre 2 angulos
set.pa18.disable         -> deshabilita el PA18 para su uso acotado entre 2 angulos
set.pixel.bright.value   -> Configura el brillo de los neopixels (value = 0 - 255)
set.pixel.effect.single  -> Configura el efecto de los neopixels a un pixel a la vez
set.pixel.effect.rise    -> Los neopixels se iran encendiendo
set.pixel.effect.fall    -> Los neopixels se iran apagando
set.pixel.enable         -> Habilita los neopixels
set.pixel.disable        -> Desahibita los neopixels
set.pixel.color.value    -> Cambia el color de los neopixels, value puede ser:
                            red, green, blue, yellow, magenta, cyan, white
                            
Lista de comandos save:
save.baud                -> Guarda el valor de la velocidad de la comunicacion Serial
                            cuando se ocupa set.baud.value, solamente se actualiza baud.
                            Al utilizar save, la siguiente vez que se energice, permanecera

