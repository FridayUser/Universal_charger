#include <stdio.h>

#define PIN_CLK 3
#define PIN_DT 2
#define PIN_SW 4

#define PIN_ADC 27

int encoderPos = 0;
int adcValue = 0;

void setup() {
  pinMode(PIN_ADC, INPUT);
  //Zwiększenie rozdzielczości ADC 10->12 bit
  analogReadResolution(12);

  pinMode(PIN_CLK, INPUT_PULLUP);
  pinMode(PIN_DT, INPUT_PULLUP);

  //Dioda builtin jako sygnalizator włączenia
  pinMode(25, OUTPUT);
  digitalWrite(25, HIGH);

  //Pin 23 -> HIGH poprawia jakość napięcia ze źródła odniesienia
  // pinMode(23, OUTPUT);
  // digitalWrite(23, HIGH);

  attachInterrupt(digitalPinToInterrupt(PIN_CLK), updateEncoder, RISING);
  attachInterrupt(digitalPinToInterrupt(PIN_SW), switchPress, RISING);

  Serial.begin(9600);
}

void loop() {
  adcValue = analogRead(PIN_ADC);
  Serial.println(3.3/4096 * adcValue);
  Serial.println(encoderPos);
  delay(500);
}

void updateEncoder() {

  bool data = digitalRead(PIN_DT);

  if(data == HIGH){
    encoderPos++;
    digitalWrite(25, HIGH);
  }
  else{
    encoderPos--;
    digitalWrite(25, LOW);
  }
}

void switchPress(){
    Serial.println("Sw press");
}