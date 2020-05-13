// LIBRERIAS -------------------------------
#include <LiquidCrystal_I2C.h> // LCD sin uso, se puede descartar esto
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 2 // Define pin de entrada de la señal digital One Wire
#include <DS3231.h>
#include <AltSoftSerial.h>
// ------------------------------------------

AltSoftSerial serialBlue; // Usa por defecto los pines 11 y 10 como RX y TX, respectivamente.
OneWire oneWire(ONE_WIRE_BUS); // Define la función de la entrada One Wire 
DallasTemperature sensors(&oneWire);

//   VARIABLES  -----------------------------------------------
int Rele = 7; // Salida del rele en pin 7
float tempActual;
DS3231  rtc(SDA, SCL);
Time  t; // Inicia la variable del tiempo
char c=' ';
String estadoActual; // Estado de la resistencia
int setPoint = 50; // Temperatura de corte limite superior
int tempInicio = 45; // Temperatura de corte limite inferior
int estadoApp = 0; // Estado de todo el sistema
//------------------------------------------------------------------


void setup()
{ Serial.begin(9600); // Inicia el Serial del Arduino (esto creo que se puede sacar terminado el prototipo)
  serialBlue.begin(9600); // Inicia Serial del HM-10 para la conexión Bluetooth
  sensors.begin(); // Inicia sensor de temperatura
  rtc.begin(); // Inicia reloj RTC
  pinMode(Rele, OUTPUT); // Define el relé como output
  digitalWrite(Rele, LOW); // Arranca siempre con el relé apagado
}

void loop()
{
  leerBlue(); // Lee la info del Bluetooth Low Energy
  tempInicio = setPoint - 5; // Define la temperatura limite inferior
  datosSensores(); // Recibe los datos de los diferentes sensores
  if( estadoApp == 1) { // Analiza que el sistema esté "ON" según instrucción del usuario
  if (t.hour > 5 && t.hour < 22) {activarResistencia();}   // Analiza que se encuentre dentro de los parámetros horarios (esto despues hacerlo para que lo pueda modificar el usuario desde la App)
  else {digitalWrite(Rele, LOW);} // Fuera de horario resistencia apagada
  } else {digitalWrite(Rele, LOW);} // Si sistema OFF, resistencia apagada
  if (digitalRead(Rele) == HIGH) {estadoActual = "1";} else {estadoActual = "0";} // Analiza si el relé está prendido o ensendido, quizás se puede hacer más simple para sacar el digitalRead
  escribirBlue(); // Al final del proceso envía la información al Android mediante Bluetooth
  Serial.print(tempActual);
  delay(2000); 
}

void escribirBlue() {
  String currentValue = String(tempActual); // Transforma el float de tempActual en un string que pueda leer el BLE
  serialBlue.print(estadoApp); // Imprime los diferentes valores del proceso, esto se puede hacer todo directamente en una lista y ahorrar bytes
  serialBlue.print("!"); // Imprime ! ya que el programa de Android separa en elementos de lista después de cada !
  serialBlue.print(estadoActual);
  serialBlue.print("!");
  serialBlue.print(currentValue);
  serialBlue.print("!");
  serialBlue.print(setPoint);
  serialBlue.print("!");
  serialBlue.print(t.hour);
  serialBlue.print(":");
  if(t.min < 10) { // Agrega un cero para que el tiempo siempre aparezca como HH:MM
    serialBlue.print("0");
    serialBlue.print(t.min);}
    else {serialBlue.print(t.min);}
}

void leerBlue() { 
  while (serialBlue.available()) // Analiza si hay data llegando desde el BLE, la lee y analiza.
    { c = serialBlue.read();
  switch (c) {
    case 48: estadoApp = 0;   break; // Si lee 0 significa que el sistema pasa a estar OFF
    case 49: estadoApp = 1;   break; // Si lee 1 significa que el sistema pasa a estar ON
    case 51: setPoint++;    break; // Si lee 3 significa que el Set Point sube 1 grado
    case 52: setPoint--;    break; // Si lee 4 significa que el Set Point baja 1 grado
  }
}
}
void datosSensores() {
  sensors.requestTemperatures(); // Recibe el valor de la temperatura en Celcius
  tempActual = sensors.getTempCByIndex(0);
  t = rtc.getTime(); // Recibe tiempo del RTC
}

void imprimirSerial() { // Actualmente sin uso, solo está para el prototipo
  Serial.println("----------------------------------------------------------------");
  Serial.print("La hora actual es: ");
  Serial.print(t.hour);
  Serial.print("La temperatura actual es: ");
  Serial.println(tempActual);
  Serial.print("La resistencia se encuentra: ");
  Serial.println(estadoActual);
  Serial.print("----------------------------------------------------------------");
}

void activarResistencia() { // Analiza la temperatura y actúa acorde. Averigüar si es mas eficiente cambiar a PID y como sería. 
  if (tempActual < tempInicio) {
    digitalWrite(Rele, HIGH); 
  }
  if (tempActual > setPoint) {
    digitalWrite(Rele, LOW); 
  }
}
