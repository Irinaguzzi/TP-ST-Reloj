// Guzzi, Judcovski, Tovbein, Resnik

#include <U8g2lib.h>
#include <WiFi.h>
#include <DHT.h>
#include <ESP32Time.h>
//wifi
const char* ssid = "ORT-IoT";
const char* password = "NuevaIOT$25";
const char* ntpServer = "pool.ntp.org";
int gmtOffset = -3;
// DHT
#define DHTPIN 23
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Pantalla OLED
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);

// Pines
#define BOTON1 34
#define BOTON2 35
#define LED 25

// Estados
#define P1 0
#define P2 1
#define RST 2
#define ESPERA1 3
#define ESPERA2 4
#define AJUSTAR_GMT 5
int estado = RST;

// Variables
float temp;
int ultimoTiempo = 0;
int millis_actual;
int millis_valor;
int intervalo = 1000;

ESP32Time rtc;

void sincronizarHora() {
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    rtc.setTimeStruct(timeinfo);
  } else {
    Serial.println("Error al sincronizar hora");
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(LED, OUTPUT);
  pinMode(BOTON1, INPUT_PULLUP);
  pinMode(BOTON2, INPUT_PULLUP);
  u8g2.begin();
  dht.begin();

  // Conexión WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado");

  configTime(gmtOffset * 3600, 0, ntpServer);
  sincronizarHora();
}

void loop() {
  millis_actual = millis();
  if (millis_actual - millis_valor >= 2000) {
    temp = dht.readTemperature();
    if (isnan(temp)) {
      Serial.println(F("Error leyendo del sensor DHT!"));
      return;
    }

    struct tm timeinfo = rtc.getTimeStruct();
    int hora = timeinfo.tm_hour;
    int minuto = timeinfo.tm_min;

    switch (estado) {
      case RST:
        millis_valor = millis();
        estado = P1;
        break;

      case P1:
        mostrarPantalla(hora, minuto, temp, gmtOffset);
        if (digitalRead(BOTON1) == LOW && digitalRead(BOTON2) == LOW) {
          estado = ESPERA1;
        }
        break;

      case ESPERA1:
        if (digitalRead(BOTON1) == HIGH && digitalRead(BOTON2) == HIGH) {
          estado = AJUSTAR_GMT;
        }
        break;

      case AJUSTAR_GMT:
        mostrarConfigGMT(gmtOffset);
        if (digitalRead(BOTON1) == LOW &&(millis_actual - ultimoTiempo > intervalo)) {
          gmtOffset++;
          ultimoTiempo = millis_actual;
          
          if (gmtOffset > 12) gmtOffset = -12;
          configTime(gmtOffset * 3600, 0, ntpServer);
          sincronizarHora();
        }
        if (digitalRead(BOTON2) == LOW && (millis_actual - ultimoTiempo > intervalo)) {
          gmtOffset--;
          ultimoTiempo = millis_actual;
           if (gmtOffset < -12) gmtOffset = 12;
          configTime(gmtOffset * 3600, 0, ntpServer);
          sincronizarHora();
        }
        if (digitalRead(BOTON1) == LOW && digitalRead(BOTON2) == LOW) {
          estado = ESPERA2;
        }
        break;

      case ESPERA2:
        if (digitalRead(BOTON1) == HIGH && digitalRead(BOTON2) == HIGH) {
          estado = P1;
        }
        break;
    }
  }
}

void mostrarPantalla(int h, int m, float t, int gmt) {
  char strTemp[6];
  char strHora[6];
  sprintf(strTemp, "%.1f", t);
  sprintf(strHora, "%02d:%02d", h, m);
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_t0_11b_tr);
  u8g2.drawStr(0, 20, "Temp:");
  u8g2.drawStr(50, 20, strTemp);
  u8g2.drawStr(90, 20, "C");
  u8g2.drawStr(0, 40, "Hora:");
  u8g2.drawStr(50, 40, strHora);
  u8g2.drawStr(0, 60, "GMT:");
  u8g2.setCursor(50, 60);
  u8g2.print(gmt);
  u8g2.sendBuffer();
}

void mostrarConfigGMT(int gmt) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_t0_11b_tr);
  u8g2.drawStr(0, 20, "Configurar GMT:");
  u8g2.setCursor(0, 40);
  u8g2.print("Actual GMT: ");
  u8g2.print(gmt);
  u8g2.sendBuffer();
}
