// CÓDIGO DEL G8 - Irina G.B., Sophie T., Amelie J. y Tiago R.

#include <WiFi.h> // Biblioteca que permite conectar el ESP32 a una red WiFi.
#include <ESP32Time.h> // Biblioteca para manejar el tiempo en el ESP32.
#include <NTPClient.h> // Biblioteca para obtener la hora actual desde un servidor NTP (Network Time Protocol).
#include <WiFiUdp.h>  // Biblioteca para enviar y recibir datos a través de UDP, usado por el cliente NTP.
#include <Adafruit_GFX.h>  // Biblioteca gráfica para manejar pantallas como la OLED.
#include <Adafruit_SSD1306.h>  // Biblioteca para controlar pantallas OLED con el chip SSD1306.
#include <Adafruit_Sensor.h> // Biblioteca común para todos los sensores Adafruit.
#include <DHT.h>  // Biblioteca para trabajar con el sensor DHT.
#include <DHT_U.h>  // Extensión de la biblioteca DHT que facilita la integración con otros sensores.


//WiFi
const char* ssid     = ""; //ponemos dsps
const char* password = "";//ponemos dsps

//pantalla OLED
#define SCREEN_WIDTH 128       
#define SCREEN_HEIGHT 64 
#define SDA_PIN 21 
#define SCL_PIN 22 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);  // Inicialización de la pantalla OLED

// Config DHT
#define DHTPIN 23       
#define DHTTYPE DHT11 
DHT_Unified dht(DHTPIN, DHTTYPE); 


#define SW1_PIN 34  
#define SW2_PIN 35  

//Config NTP y reloj
WiFiUDP ntpUDP;  // UDP para la comunicación NTP
int gmtOffset = -3; // Desfase horario en horas (ajustable)
NTPClient timeClient(ntpUDP, "pool.ntp.org", gmtOffset * 3600); // Cliente NTP para obtener la hora
ESP32Time rtc; // Reloj en tiempo real basado en ESP32

//Mmáquina de estados para controlar las pantallas
enum Estado {
  ESTADO_INICIO,              
  ESTADO_MOSTRAR_PANTALLA_1, 
  ESTADO_MOSTRAR_PANTALLA_2 
};
Estado estado = ESTADO_INICIO; 

// variables para los botones y temporización
bool sw1Anterior = HIGH;  // Estado previo del botón 1
bool sw2Anterior = HIGH;// Estado previo del botón 2
unsigned long tiempoBotonesAnterior = 0; // Control de tiempos de los botones
unsigned long intervaloBotones = 200; // Intervalo de tiempo entre lecturas de botones
int temperaturaActual = 0; // Variable para la temperatura

void setup() {
  Serial.begin(9600);            
  dht.begin();                

  pinMode(SW1_PIN, INPUT_PULLUP);
  pinMode(SW2_PIN, INPUT_PULLUP);

  Wire.begin(SDA_PIN, SCL_PIN);  // Iniciar comunicación I2C con la pantalla OLED

  // Inicializar la pantalla OLED y mostrar mensaje si hay error
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Error al iniciar la pantalla");
    while (true); // Detener ejecución si no se puede iniciar la pantalla
  }

  displayMensaje("Conectando WiFi...");  
  WiFi.begin(ssid, password);        
  while (WiFi.status() != WL_CONNECTED) {  // Esperar hasta que se conecte
    delay(500);
    Serial.print(".");
  }

  displayMensaje("WiFi conectado."); // Mostrar mensaje de que se logro conecta
  timeClient.begin(); // Iniciar cliente NTP
  timeClient.update(); // Obtener la hora
  rtc.setTime(timeClient.getEpochTime()); // Config el reloj con la hora NTP

  estado = ESTADO_MOSTRAR_PANTALLA_1; 
}

void loop() {
  unsigned long ahora = millis(); // Obtener el tiempo actual desde el inicio del programa
  bool sw1 = digitalRead(SW1_PIN);
  bool sw2 = digitalRead(SW2_PIN);  

  // solo leer los botones después de un intervalo de tiempo
  if (ahora - tiempoBotonesAnterior >= intervaloBotones) {
    tiempoBotonesAnterior = ahora;

    // cambiar entre las pantallas si ambos botones están apretados
    if (sw1 == LOW && sw2 == LOW && (sw1Anterior == HIGH || sw2Anterior == HIGH)) {
      estado = (estado == ESTADO_MOSTRAR_PANTALLA_1) ? ESTADO_MOSTRAR_PANTALLA_2 : ESTADO_MOSTRAR_PANTALLA_1;
    }

    //ajstar el GMT si estamos en la pantalla de configuración (Pantalla 2)
    if (estado == ESTADO_MOSTRAR_PANTALLA_2) {
      if (sw1Anterior == LOW && sw1 == HIGH && gmtOffset < 12) { //Aumentar el GMT
        gmtOffset++;
        actualizarNTP();  // Actualizar la hora NTP con el nuevo GMT
      }
      if (sw2Anterior == LOW && sw2 == HIGH && gmtOffset > -12) { //Disminuir el GMT
        gmtOffset--;
        actualizarNTP();  // Actualizar la hora NTP con el nuevo GMT
      }
    }

    //guardar los estados anteriores de los botones para el siguiente ciclo
    sw1Anterior = sw1;
    sw2Anterior = sw2;
  }

  //obotener la temperatura actual del sensor DHT
  temperaturaActual = obtenerTemperatura();

  //Máquina de estados para manejar las pantallas
  switch (estado) {
    case ESTADO_MOSTRAR_PANTALLA_1: {  
      display.clearDisplay(); // Limpia
      display.setCursor(0, 0); // Establece la posición del cursoor
      display.println("Hora y Temp"); // Título de la pantalla

      struct tm timeinfo = rtc.getTimeStruct(); // Obtiene la hora actual desde el RTC
      display.setCursor(0, 20);
      if (timeinfo.tm_hour < 10) display.print("0"); // Formatear la hora
      display.print(timeinfo.tm_hour);
      display.print(":");
      if (timeinfo.tm_min < 10) display.print("0"); // Formatear los minutos
      display.println(timeinfo.tm_min);

      display.print("Temp: "); // Mostrar la temperatura
      display.print(temperaturaActual);
      display.println(" C");
      display.display();// Actualizar la pantalla
      break;
    }

    case ESTADO_MOSTRAR_PANTALLA_2: { // mostrar configuracion de GMT
      display.clearDisplay(); // limpiar la pantalla
      display.setCursor(0, 0);
      display.setTextSize(1);
      display.println("Config GMT");

      display.setCursor(0, 20);
      display.print("GMT: ");
      if (gmtOffset >= 0) display.print("+"); // mostrar GMT con signo positivo o negativo
      display.println(gmtOffset);

      display.println("SW1: +1 GMT"); // instrucciones para el usuario
      display.println("SW2: -1 GMT");
      display.display();// actualizar la pantalla
      break;
    }

    default: {
      break;
    }
  }
}


// mmostrar un mensaje en la pantalla OLED
void displayMensaje(const char* msg) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println(msg); // Mostrar el mensaje recibido
  display.display();
}

//actualizar la hora utilizando el servidor NTP
void actualizarNTP() {
  timeClient.setTimeOffset(gmtOffset * 3600); // Actualizar el desfase horario
  timeClient.update(); // Obtener la hora actualizada
  rtc.setTime(timeClient.getEpochTime()); // Configurar el RTC con la nueva hora
}

// oobtener la temperatura del sensor DHT
int obtenerTemperatura() {
  sensors_event_t event;
  dht.temperature().getEvent(&event);  // Obtener evento de temperatura
  return isnan(event.temperature) ? 0 : event.temperature; // Devolver temperatura o 0 si hay error
