// CÓDIGO DEL G8 - Irina G.B., Sophie T., Amelie J. y Tiago R.

#include <Wire.h>  // Biblioteca que permite la comunicación I2C  para interactuar con la pantalla OLED
#include <Adafruit_GFX.h>  // Biblioteca gráfica de Adafruit para mostrar texto 
#include <Adafruit_SSD1306.h>  // Biblioteca específica para pantallas OLED con controlador SSD1306
#include <Adafruit_Sensor.h>  // Biblioteca para sensores Adafruit
#include <DHT.h>  // Biblioteca para utilizar sensores de temperatura y humedad como el DHT11
#include <DHT_U.h> // Versión unificada del DHT que permite mayor compatibilidad con otras bibliotecas

#define SCREEN_WIDTH 128 //ancho de la pantala OLED en ppíxeles
#define SCREEN_HEIGHT 64 //altura dela pantalla OLED en píxeles
#define SDA_PIN 21 //pin para línea de datos del protocolo I2C
#define SCL_PIN 22 //pin para la línea de reloj del protocolo I2C

#define DHTPIN 23 //pin digital al que se conecta el sensor de temperatura y humedad
#define DHTTYPE DHT11 //definición del tipo de sensor DHT que se utiliza: el DHT11

#define SW1_PIN 34 
#define SW2_PIN 35 


//objetos para el sensor de temperatura y la pantalla OLED
DHT_Unified dht(DHTPIN, DHTTYPE); //inicialización del sensor DHT11
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); //inicialización de la pantalla OLED con las dimensiones indicadas y comunicación I2C

//variables globales
uint32_t delayMS; //tiempo mínimo entre lecturas del sensor, proporcionado por la biblioteca
int temperaturaActual = 0;   //valor de temperatura leído del sensor
int hora = 12; // hora actual mostrada en pantalla
int minuto = 0;  //minuto actual mostrado en pantaual = ESTADO_PANTALLA_1;
Estado estadoAnterior = ESTADO_PANTALLA_1; // estado anterior para controlar transiciones

// variables utilizadas para controlar el tiempo de lectura de temperatura y lectura de botones
unsigned long tiempoAnterior = 0; // tiempo de la última lectura de temperatura
unsigned long intervalo = 1000; // intervalo de actualización para la temperatura (en milisegundos)

unsigned long tiempoBotonesAnterior = 0; //tiemo de la última lectura de los botones
unsigned long intervaloBotones = 200; //tiempo minimmo entre lecturas de botones para evitar rebotes

bool sw1Anterior = HIGH; //estado anterior del botón 1
bool sw2Anterior = HIGH; //estado anterior del botón 2

void setup() {
  Serial.begin(9600);                     
  dht.begin(); //inicialización del sensor de temperatura y humedad
  pinMode(SW1_PIN, INPUT_PULLUP); 
  pinMode(SW2_PIN, INPUT_PULLUP); 

  //config del sensor y atraso mínimo entre lecturas
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  delayMS = sensor.min_delay / 1000;  //se convierte el atraso de microsegundos a milisegundos

  Wire.begin(SDA_PIN, SCL_PIN);  //se inicia la comunicación I2C con los pines

  // inicialización de la pantalla OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed")); // muestra mensaje si falla la pantalla
    for (;;); // queda en bucle infinito si hay error con la pantalla
  }

  display.clearDisplay(); // limpia la pantalla
  display.setTextSize(1); // tamaño de letra chico
  display.setTextColor(SSD1306_WHITE); // color blanco
  display.setCursor(0, 0); // comienza a escribir en esquina superior izquierda
  display.println("Inicializando..."); // muestra mensaje de inicio
  display.display(); // actualiza la pantalla con el contenido
  delay(1000); // espera un segundo antes de empezar
}

void loop() {
  unsigned long tiempoActual = millis(); // obtiene el tiempo actual en milisegundos

  if (tiempoActual - tiempoAnterior >= intervalo) { // si pasó el tiempo suficiente
    tiempoAnterior = tiempoActual; // actualiza el último tiempo

    sensors_event_t event; // crea objeto para evento del sensor
    dht.temperature().getEvent(&event); // lee la temperatura
    if (!isnan(event.temperature)) { // si la lectura es válida
      temperaturaActual = event.temperature; 
    } else {
      Serial.println(F("Error al leer la temperatura")); // muestra error si no pudo leer
    }
  }

  if (tiempoActual - tiempoBotonesAnterior >= intervaloBotones) { // control de rebote para botones
    tiempoBotonesAnterior = tiempoActual; // actualiza el tiempo de botones

    bool sw1 = digitalRead(SW1_PIN);
    bool sw2 = digitalRead(SW2_PIN);

    if (sw1 == LOW && sw1Anterior == HIGH) { //si se presionó el botón 1
      if (estadoActual == ESTADO_PANTALLA_1) {
        estadoActual = ESTADO_PANTALLA_2; //cambia al modo de configuración
      } else {
        estadoActual = ESTADO_PANTALLA_1; //vuelve al modo normal
      }
    }

    if (sw2 == LOW && sw2Anterior == HIGH && estadoActual == ESTADO_PANTALLA_2) {
      minuto += 1; // aumenta los minutos
      if (minuto >= 60) { //si pasa de 59
        minuto = 0; //reinicia minutos
        hora = (hora + 1) % 24; //suma una hora y reinicia si llega a 24
      }
    }

    sw1Anterior = sw1; //guarda el estado actual del botón 1
    sw2Anterior = sw2; //guarda el estado actual del botón 2
  }

  if (estadoActual != estadoAnterior || tiempoActual - tiempoAnterior < 100) {
    display.clearDisplay(); // limpia pantalla
    display.setCursor(0, 0); //comienza a escribir desde arriba

    if (estadoActual == ESTADO_PANTALLA_1) {
      display.println("Hora y Temp:");
      display.print("Hora: ");
      if (hora < 10) display.print("0"); //agrega 0 si la hora es menor a 10
      display.print(hora);
      display.print(":");
      if (minuto < 10) display.print("0"); // agrega 0 si los minutos son menores a 10
      display.println(minuto);
      display.print("Temp: ");
      display.print(temperaturaActual);
      display.println(" C"); 

    } else if (estadoActual == ESTADO_PANTALLA_2) {
      display.println("Configurar Hora:"); //pantalla de configuración
      display.print("Hora: ");
      if (hora < 10) display.print("0");
      display.print(hora);
      display.print(":");
      if (minuto < 10) display.print("0");
      display.println(minuto);
      display.println("SW2 para ajustar"); //aclaración
    }

    display.display(); //muestra todo en pantalla
    estadoAnterior = estadoActual; //actualiza el estado anterior
  }
}

  }
