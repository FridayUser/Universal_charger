#include <stdio.h>

#define PIN_CLK 3
#define PIN_DT 2
#define PIN_SW 4

#define PIN_ADC 27

int encoderPos = 0;
int adcValue = 0;

void setup() {
  pinMode(PIN_ADC, INPUT);
  analogReadResolution(12);

  pinMode(PIN_CLK, INPUT);
  pinMode(PIN_DT, INPUT);

  pinMode(25, OUTPUT);
  digitalWrite(25, HIGH);

  attachInterrupt(digitalPinToInterrupt(PIN_CLK), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_SW), switchPress, CHANGE);

  Serial.begin(9600);
}

void loop() {
  adcValue = analogRead(PIN_ADC);
  Serial.println(3.3/4096 * adcValue);
  delay(500);
}

void updateEncoder() {
  bool data = digitalRead(PIN_DT);
  bool clk = digitalRead(PIN_CLK);
  if(data == clk){
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
