//Guzzi, Judcovsky, Tovbein, Resnik

#include <U8g2lib.h>
#include "DHT.h"
#include <ESP32Time.h>
/* ----------------------------------------------------------------------------------------------------- */

// Definicion de constructores y variables globales

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

/* ----------------------------------------------------------------------------------------------------- */
void printBMP_OLED(void );
void printBMP_OLED2(void) ;
#define BOTON1 34
#define BOTON2 35
#define P1 0
#define P2 1
#define RST 20
#define ESPERA1 2
#define ESPERA2 3
#define AUMENTARM 4
#define AUMENTARH 5
int estado = RST;
#define LED 25
#define DHTPIN 23
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
float temp;
int millis_valor;
int millis_actual;

ESP32Time rtc;

int minutos;
int hora_24;


void setup() {
  pinMode(LED, OUTPUT);
  pinMode(BOTON1, INPUT_PULLUP);
  pinMode(BOTON2, INPUT_PULLUP);
  Serial.begin(9600);
  Serial.println(F("DHTxx test!"));
  u8g2.begin();
  dht.begin();
  rtc.setTime(15, 8, 10, 25, 4, 2025);

  // Obtener los valores de la hora
    minutos = rtc.getMinute();
    hora_24 = rtc.getHour(true);

}

void loop() {
  millis_actual = millis();
  if (millis_actual - millis_valor >= 2000) {
    temp = dht.readTemperature();
    if (isnan(temp)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }

    
    switch (estado) {
      case RST:
        millis_valor = millis();
        estado = P1;
        break;

      case P1:
        printBMP_OLED();
        if (digitalRead(BOTON1) == LOW && digitalRead(BOTON2) == LOW) {
          estado = ESPERA1;
        }
        break;

      case ESPERA1:
        if (digitalRead(BOTON1) == HIGH && digitalRead(BOTON2) == HIGH) {
          estado = P2;
        }
        break;

      case P2:
        printBMP_OLED2();
        if (digitalRead(BOTON1) == LOW) {
          estado = AUMENTARM;
        }
        if (digitalRead(BOTON2) == LOW) {
          estado = AUMENTARH;
        }
        if (digitalRead(BOTON1) == LOW && digitalRead(BOTON2) == LOW) {
          estado = ESPERA2;
        }
        if (hora_24 >= 24){
          hora_24 = 0;
        }
        if (minutos >= 60){
          minutos = 0;
          hora_24 = hora_24 + 1;
        }
        break;

      case ESPERA2:
        if (digitalRead(BOTON1) == HIGH && digitalRead(BOTON2) == HIGH) {
          estado = P1;
        }
        break;

      case AUMENTARM:
        if (digitalRead(BOTON1) == HIGH) {
          minutos = minutos + 1;
          estado = P2;
        }
        break;

      case AUMENTARH:
        if (digitalRead(BOTON2) == HIGH) {
          hora_24 = hora_24 +1;
          estado = P2;
        }
        break;
    }
  }
}

void printBMP_OLED(void) {
  char stringtemp[5];
  u8g2.clearBuffer();          // Clear the internal memory
  u8g2.setFont(u8g2_font_t0_11b_tr); // Choose a suitable font
  sprintf(stringtemp, "%.2f", temp); // Convert float to string
  u8g2.drawStr(0, 20, "T. Actual:");
  u8g2.drawStr(70, 20, stringtemp);
  u8g2.drawStr(100, 20, "°C");

  char timeStr[10];
  sprintf(timeStr, "%02d:%02d", hora_24, minutos);
  u8g2.drawStr(0, 40, "Hora Actual:");
  u8g2.drawStr(75, 40, timeStr);
  u8g2.sendBuffer();          // Transfer internal memory to the display
}

void printBMP_OLED2(void) {
  u8g2.clearBuffer();          // Clear the internal memory
  u8g2.setFont(u8g2_font_t0_11b_tr); // Choose a suitable font

  char timeStr[10];
  sprintf(timeStr, "%02d:%02d", hora_24, minutos);
  u8g2.drawStr(0, 20, "Hora Actual:");
  u8g2.drawStr(75, 20, timeStr);
  u8g2.sendBuffer();          // Transfer internal memory to the display
}
