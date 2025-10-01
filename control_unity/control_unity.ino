#include <Wire.h>
#include <max6675.h>

#define USE_SERIAL_2004_LCD

#include "LCDBigNumbers.hpp"
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

#define RPM_PIN 14
#define COMBUSTIVEL_PIN 4
#define SENSOR_VELOCIDADE_PIN 23 
#define LED_TEMP 12
#define LED_COMB 13
int thermoCLK = 18;
int thermoCS  = 5;
int thermoDO  = 19;

unsigned long data2send = 0;
volatile unsigned long revolutions = 0;
unsigned long rpm = 0;

volatile unsigned long contVelocidade = 0;
unsigned long lastTimeVelocidade;
unsigned long velocidade = 0.0;
float R = 0.05175; // Raio da roda (m)
#define PI 3.1415926535897932384626433832795

MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

#define LCD_I2C_ADDRESS 0x27
LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, 20, 4);
LCDBigNumbers big_number(&lcd, BIG_NUMBERS_FONT_3_COLUMN_3_ROWS_VARIANT_1);

void setup() {
  Serial.begin(115200);

  // Sensor de RPM
  pinMode(RPM_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RPM_PIN), rpmISR, RISING);

  // Sensor de Velocidade
  pinMode(SENSOR_VELOCIDADE_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(SENSOR_VELOCIDADE_PIN), velocidadeISR, RISING);
  lastTimeVelocidade = millis();

  // Sensor de Combustível e Temperatura
  pinMode(COMBUSTIVEL_PIN, INPUT);
  pinMode(LED_COMB, OUTPUT);
  pinMode(LED_TEMP, OUTPUT);

  lcd.init();
  lcd.clear();
  lcd.backlight();

  big_number.begin();
  lcd.setCursor(0, 0);
  delay(1000);
  lcd.clear();
}

// ========== LOOP PRINCIPAL ==========
void loop() {
  data2send = 0;
  big_number.setBigNumberCursor(0, 0);

  lerRPM();
  lerVelocidade();
  lerCombustivel();
  lerTemperatura();

  char number[4];
  unsigned int temp = ((data2send & 0x000ff000) >> 12);
  snprintf(number, 4, "%d", temp);
  

  lcd.clear();
  delay(32);

  if (temp < 10) {
    big_number.setBigNumberCursor(8, 1);
    big_number.print(number[0]);
  } else {
    big_number.setBigNumberCursor(11, 1);
    big_number.print(number[1]);
    big_number.setBigNumberCursor(6, 1);
    big_number.print(number[0]);
  }
  
  Serial.printf("%#010x\n", data2send);
  delay(1000);
}

// --- RPM ---
void rpmISR() {
  revolutions++;
}

void lerRPM() {
  noInterrupts();
  unsigned long revs = revolutions;
  revolutions = 0;
  interrupts();

  rpm = revs * 60; // revoluções por segundo * 60
  data2send |= (0xfff00000 & (rpm << 20));
}

// --- Velocidade ---
void velocidadeISR() {
  contVelocidade++;
}

void lerVelocidade() {
  // if (millis() - lastTimeVelocidade >= 1000) {
    long currentMillis = millis();
    long delta = currentMillis - lastTimeVelocidade;
    lastTimeVelocidade = currentMillis;

    long cont = contVelocidade / 3; // 3 furos
    contVelocidade = 0;

    float velocidade_m_s = (2 * PI * R * cont) / (delta / 1000.0);
    velocidade = velocidade_m_s * 3.6;
  //}

  data2send |= (0x000ff000 & (velocidade << 12));
}

// --- Combustível ---
void lerCombustivel() {
  int estado = digitalRead(COMBUSTIVEL_PIN);
  if (estado == HIGH) {
    data2send |=  (0x0000000f & 0x1);
    digitalWrite(LED_COMB, LOW);
  } else {
    data2send |= (0x00000000f & 0x0);
    digitalWrite(LED_COMB, HIGH);
  }
}

// --- Temperatura ---
void lerTemperatura() {
  delay(32);

  double temp = thermocouple.readCelsius();

  if (isnan(temp)) {
    data2send |= 0x00000ff0;
  } else {
    unsigned long tmp = temp;
    data2send |= (0x00000ff0 & (tmp << 4));

    if(temp > 80)
      digitalWrite(LED_TEMP, HIGH);
    else
      digitalWrite(LED_TEMP, LOW);
  }

  delay(256);
}
