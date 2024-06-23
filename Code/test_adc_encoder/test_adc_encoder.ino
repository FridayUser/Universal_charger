#include <stdio.h>

//------------------------- Pin definitions ------------------------

#define DQ_PSU_OFF 2
#define DQ_ONE_WIRE 4
#define DQ_LOAD_EN 8
#define DQ_BATP_ON 10

#define AQ_PSU_SETV 3
#define AQ_PSU_SETI 6
#define AQ_BAL_BALV 7
#define AQ_LOAD_SETI 9

#define DI_ENC_CLK 11
#define DI_ENC_DT 12
#define DI_ENC_SW 13
#define DI_BTN2 14
#define DI_BTN1 15

#define SPI_BL 22
#define SPI_RST 21
#define SPI_DC 20
#define SPI_MOSI 19
#define SPI_SCK 18
#define SPI_CS 17
#define SPI_MISO 16

#define MUX_A 27
#define MUX_B 28
#define MUX_C 5

#define MUX_ADC 26

//------------------------- Basic variables ------------------------

int encoderPos = 0;
int adcValue = 0;

bool PsuOff = false;
bool LoadEn = false;
bool BatPOn = false;
int OneWireData;

int PsuSetV = 0;
int PsuSetI = 0;
int BalV = 0;
int LoadSetI = 0;

int AdcReadout = 0;
int Bat1V = 0;
int Bat2V = 0;
int Bat3V = 0;
int Bat4V = 0;
int Bat5V = 0;
int Bat6V = 0;
int OutV = 0;
int OutI = 0;

//------------------------- Setup ------------------------

void setup() {
  
  pinMode(DQ_PSU_OFF, OUTPUT);
  pinMode(DQ_ONE_WIRE, OUTPUT);
  pinMode(DQ_LOAD_EN, OUTPUT);
  pinMode(DQ_BATP_ON, OUTPUT);

  pinMode(AQ_PSU_SETV, OUTPUT);
  pinMode(AQ_PSU_SETI, OUTPUT);
  pinMode(AQ_BAL_BALV, OUTPUT);
  pinMode(AQ_LOAD_SETI, OUTPUT);

  pinMode(DI_ENC_CLK, INPUT);
  pinMode(DI_ENC_DT, INPUT);
  pinMode(DI_ENC_SW, INPUT);
  pinMode(DI_BTN2, INPUT);
  pinMode(DI_BTN1, INPUT);

  pinMode(SPI_BL, OUTPUT);
  pinMode(SPI_RST, OUTPUT);
  pinMode(SPI_DC, OUTPUT);
  pinMode(SPI_MOSI, OUTPUT);
  pinMode(SPI_SCK, OUTPUT);
  pinMode(SPI_CS, OUTPUT);
  pinMode(SPI_MISO, INPUT);

  pinMode(MUX_A, OUTPUT);
  pinMode(MUX_B, OUTPUT);
  pinMode(MUX_C, OUTPUT);

  pinMode(MUX_ADC, INPUT);
  analogReadResolution(12);

  pinMode(25, OUTPUT);
  digitalWrite(25, HIGH);

  attachInterrupt(digitalPinToInterrupt(DI_ENC_CLK), updateEncoder, RISING);
  attachInterrupt(digitalPinToInterrupt(DI_ENC_SW), switchEncPress, RISING);
  attachInterrupt(digitalPinToInterrupt(DI_BTN2), switch1Press, RISING);
  attachInterrupt(digitalPinToInterrupt(DI_BTN1), switch2Press, RISING);

  Serial.begin(9600);

  // ---------------------- Testing ------------------------
  // --------- Section Analog Out ------------ 
  analogWriteFreq(100000);
  analogWriteResolution(12);
  //analogWrite(AQ_PSU_SETV, (4096/3.3)*1); //*10
  //digitalWrite(AQ_PSU_SETV, LOW);
  analogWrite(AQ_PSU_SETI, (4096/3.3)*2);
  analogWrite(AQ_BAL_BALV, (4096/3.3)*3);
  analogWrite(AQ_LOAD_SETI, (4096/3.3)*2.5);
  //analogWrite(0, 2000);
  //analogWrite(1, 2000);
  //analogWrite(4, 2000);
  
  // -------- Section Analog In --------------
  digitalWrite(MUX_A, LOW);
  digitalWrite(MUX_B, HIGH);
  digitalWrite(MUX_C, HIGH);

  // -------- Section Digital Out ------------
  digitalWrite(DQ_PSU_OFF, HIGH);
  digitalWrite(DQ_LOAD_EN, HIGH);
  digitalWrite(DQ_BATP_ON, HIGH);
}

//------------------------- Main ------------------------

void loop() {
  
  AdcReadout = adcReadout(1);
  Serial.println(3.3/4096 * AdcReadout);
  delay(500);
}

//------------------------- Methods ------------------------

int adcReadout(int chNum){
  switch (chNum){
    case 0:
      digitalWrite(MUX_A, LOW);
      digitalWrite(MUX_B, LOW);
      digitalWrite(MUX_C, LOW);
      break; 
    case 1: 
      digitalWrite(MUX_A, HIGH);
      digitalWrite(MUX_B, LOW);
      digitalWrite(MUX_C, LOW);
      break;
    case 2:
      digitalWrite(MUX_A, LOW);
      digitalWrite(MUX_B, HIGH);
      digitalWrite(MUX_C, LOW);
      break;
    case 3:
      digitalWrite(MUX_A, HIGH);
      digitalWrite(MUX_B, HIGH);
      digitalWrite(MUX_C, LOW);
      break;
    case 4:
      digitalWrite(MUX_A, LOW);
      digitalWrite(MUX_B, LOW);
      digitalWrite(MUX_C, HIGH);
      break; 
    case 5:
      digitalWrite(MUX_A, HIGH);
      digitalWrite(MUX_B, LOW);
      digitalWrite(MUX_C, HIGH);
      break;
    case 6:
      digitalWrite(MUX_A, LOW);
      digitalWrite(MUX_B, HIGH);
      digitalWrite(MUX_C, HIGH);
      break;
    case 7:
      digitalWrite(MUX_A, HIGH);
      digitalWrite(MUX_B, HIGH);
      digitalWrite(MUX_C, HIGH);
      break;
  }
  //delay
  return analogRead(MUX_ADC);
}

//------------------------- Interrupts ------------------------

void updateEncoder() {
  bool data = digitalRead(DI_ENC_DT);
  bool clk = digitalRead(DI_ENC_CLK);
  if(data == clk){
    encoderPos++;
    digitalWrite(25, HIGH);
  }
  else{
    encoderPos--;
    digitalWrite(25, LOW);
  }
}

void switchEncPress(){
    
}

void switch1Press(){
    
}

void switch2Press(){
    
}