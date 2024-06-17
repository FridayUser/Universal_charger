/*
TODO:
- Dopisać wyświetlanie pozostałych wartości odczytanych z ADC-ka
- Dopisać funkcję nastawy wartości
- Do poprawy logika stanów (Sub i Val menu kiedy true a kiedy false):
    - Main: Sub:F, Val:F
    - Submenu: Sub:T, Val:F
    - Val: Sub:T, Val:T
- W CLKInterrupt zmienić stałe od infinite scrolla na nowe zmienne
- Dopisać w menuZaznaczonym nowe pozycje z głównego menu
- Czy da się jakoś uprościć podział na menu i zaznaczone menu do jednej funkcji?

DONE:
- Szkielet do zmiennych kontroli położenia ibiektów w menu
- Logika stanów przejścia 
- encSW_Interrupt i switch1Press skrócone, nie wiem czy to o to chodziło w logice stanów
- Dokończyć wymianę stałychna zmienne w pozostałych pozycjach  menu
- trochę zoptymalizowany kod, jak się nie spodoba to możemy wrócić do poprzedniej wersji jak dla mnie bardziej czytelny

*/


/*******************************************************************************
 * Start of Arduino_GFX setting
 *
 * Arduino_GFX try to find the settings depends on selected board in Arduino IDE
 * Or you can define the display dev kit not in the board list
 * Defalult pin list for non display dev kit:

 * Raspberry Pi Pico dev board : CS: 17, DC: 20, RST: 21, BL: 22, SCK: 18, MOSI: 19, MISO: 16
 ******************************************************************************/
#include <Arduino_GFX_Library.h>

//Changed line 195, 197 in Arduino_GFX_Library!!!

#define GFX_BL DF_GFX_BL // default backlight pin, you may replace DF_GFX_BL to actual backlight pin

/* More dev device declaration: https://github.com/moononournation/Arduino_GFX/wiki/Dev-Device-Declaration */
#if defined(DISPLAY_DEV_KIT)
Arduino_GFX *gfx = create_default_Arduino_GFX();
#else /* !defined(DISPLAY_DEV_KIT) */

/* More data bus class: https://github.com/moononournation/Arduino_GFX/wiki/Data-Bus-Class */
Arduino_DataBus *bus = new Arduino_RPiPicoSPI(20 /* DC */, 17 /* CS */, 18 /* SCK */, 19 /* MOSI */, 16 /* MISO */, spi0 /* spi */);

/* More display class: https://github.com/moononournation/Arduino_GFX/wiki/Display-Class */
//Arduino_GFX *gfx = new Arduino_ILI9341(bus, DF_GFX_RST, 0 /* rotation */, false /* IPS */);
Arduino_GFX *gfx = new Arduino_ST7789(bus, DF_GFX_RST, 1 /* rotation */, false /* IPS */);

#endif /* !defined(DISPLAY_DEV_KIT) */
/*******************************************************************************
 * End of Arduino_GFX setting
 ******************************************************************************/

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

#define MUX_A 27
#define MUX_B 28
#define MUX_C 5

#define MUX_ADC 26

const int menusAmount = 5;
const int screenWidth= gfx->height();
const int screenHeight = gfx->width();

int currentMenu = 1;
int currentSubMenu = 1;
bool inSubMenu = false;
bool inValueMenu = false;

int ch0Val, ch1Val, ch2Val, ch3Val, ch4Val, ch5Val, ch6Val, ch7Val;

int PsuSetV = 0;
int PsuSetI = 0;
int BalV = 0;
bool balanceOn = false;
bool chargeOn = false;

int loadISmall = 0;
int loafILarge = 0;

void setup(void)
{
  Serial.begin(9600);
  // Serial.setDebugOutput(true);
  // while(!Serial);

  // Init Display
  if (!gfx->begin())
  {
    Serial.println("gfx->begin() failed!");
  }
  gfx->fillScreen(BLACK);
  gfx->setCursor(gfx->width()/2, gfx->height()/2);
  gfx->setTextColor(random(0xffff), random(0xffff));
  gfx->setTextSize(2,2);
  gfx->println("Hello world!");

  pinMode(DQ_PSU_OFF, OUTPUT);
  pinMode(DQ_ONE_WIRE, OUTPUT);
  pinMode(DQ_LOAD_EN, OUTPUT);
  pinMode(DQ_BATP_ON, OUTPUT);

  pinMode(AQ_PSU_SETV, OUTPUT);
  pinMode(AQ_PSU_SETI, OUTPUT);
  pinMode(AQ_BAL_BALV, OUTPUT);
  pinMode(AQ_LOAD_SETI, OUTPUT);

  pinMode(MUX_A, OUTPUT);
  pinMode(MUX_B, OUTPUT);
  pinMode(MUX_C, OUTPUT);

  pinMode(MUX_ADC, INPUT);
  analogReadResolution(12);

  pinMode(25, OUTPUT);  //led on
  digitalWrite(25, HIGH);

  analogWrite(AQ_PSU_SETV, (4096/3.3)*0); //*10
  analogWrite(AQ_PSU_SETI, (4096/3.3)*0);
  analogWrite(AQ_BAL_BALV, (4096/3.3)*0);
  analogWrite(AQ_LOAD_SETI,(4096/3.3)*0);

  digitalWrite(DQ_PSU_OFF, HIGH);
  digitalWrite(DQ_LOAD_EN, LOW);
  digitalWrite(DQ_BATP_ON, LOW);

  analogWriteFreq(100000);
  analogWriteResolution(12);

  #ifdef GFX_BL
    pinMode(GFX_BL, OUTPUT);
    digitalWrite(GFX_BL, HIGH);
  #endif

  pinMode(DI_ENC_CLK, INPUT_PULLUP);
  pinMode(DI_ENC_DT, INPUT_PULLUP);
  pinMode(DI_ENC_SW, INPUT_PULLUP);
  pinMode(DI_BTN2, INPUT_PULLUP);
  pinMode(DI_BTN1, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(DI_ENC_CLK), encCLK_Interrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(DI_ENC_SW), encSW_Interrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(DI_BTN2), switch1Press, FALLING);
  attachInterrupt(digitalPinToInterrupt(DI_BTN1), switch2Press, FALLING);

  delay(200);  //Cosmetic delay, can delete

  gfx->fillScreen(BLACK);
  drawMenu();
}

void loop()
{
  measureParameters(ch0Val, ch1Val, ch2Val, ch3Val, ch4Val, ch5Val, ch6Val, ch7Val);
  setParameters();
  delay(100);  //Sleep well little prince
}

void setParameters(){
  analogWrite(AQ_PSU_SETV, (4096/3.3)*PsuSetV); //*10
  analogWrite(AQ_PSU_SETI, (4096/3.3)*PsuSetI);
  analogWrite(AQ_BAL_BALV, (4096/3.3)*BalV);
}

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

void measureParameters(int &ch0Val, int &ch1Val, int &ch2Val, int &ch3Val, int &ch4Val, int &ch5Val, int &ch6Val, int &ch7Val){
  ch0Val = adcReadout(0);
  delay(10);  //MUX channel switch time
  ch1Val = adcReadout(1);
  delay(10);
  ch2Val = adcReadout(2);
  delay(10);
  ch3Val = adcReadout(3);
  delay(10);
  ch4Val = adcReadout(4);
  delay(10);
  ch5Val = adcReadout(5);
  delay(10);
  ch6Val = adcReadout(6);
  delay(10);
  ch7Val = adcReadout(7);
  delay(10);
}

const int menuMainElementCnt = 3; //Number of elements in main menu
const int menuSub1ElementCnt = 6; //Number of elements inside the sub menu 1, not including menu title 
const int menuSub2ElementCnt = 5; //...
const int menuSub3ElementCnt = 2; //...

const int titleOffset = 20;   //Offset between title and first menu element
const int elementOffset = 20; //Offset between menu elements
const int leftOffset = 10;    //Offest from left screen edge

void drawMainMenu(){
    gfx->setCursor(leftOffset, 10);
    gfx->println(" Very good charger ");

    gfx->setCursor(leftOffset, titleOffset + elementOffset);
    gfx->println(" Measure voltages ");

    gfx->setCursor(leftOffset, titleOffset + 2 * elementOffset);
    gfx->println(" Control PSU ");

    gfx->setCursor(leftOffset, titleOffset + 3 * elementOffset);
    gfx->println(" Control Load ");
}

void drawSubMenu1(){
  gfx->setCursor(leftOffset, 10);
  gfx->println(".../ Measure voltages ");

  gfx->setCursor(leftOffset, titleOffset+1*elementOffset);
  gfx->println(" Bat1 V: " + String(3.3/4096 * ch0Val) + "V");

  gfx->setCursor(leftOffset, titleOffset+2*elementOffset);
  gfx->println(" Bat2 V: " + String(3.3/4096 * ch1Val) + "V");

  gfx->setCursor(leftOffset, titleOffset+3*elementOffset);
  gfx->println(" Bat3 V: " + String(3.3/4096 * ch2Val) + "V");

  gfx->setCursor(leftOffset, titleOffset+4*elementOffset);
  gfx->println(" Bat4 V: " + String(3.3/4096 * ch3Val) + "V");

  gfx->setCursor(leftOffset, titleOffset+5*elementOffset);
  gfx->println(" Bat5 V: " + String(3.3/4096 * ch4Val) + "V");

  gfx->setCursor(leftOffset, titleOffset+6*elementOffset);
  gfx->println(" Bat6 V: " + String(3.3/4096 * ch5Val) + "V");
}

void drawSubMenu2(){
  if(inValueMenu){
    switch(currentSubMenu){
      case 1:
        gfx->setCursor(leftOffset, screenHeight/2);
        gfx->println(" PSU Voltage: " + String(3.3/4096 *PsuSetV));
      break;
      case 2:
        gfx->setCursor(leftOffset, screenHeight/2);
        gfx->println(" PSU Current: " + String(3.3/4096 *PsuSetI));
      break;
      case 3: //Balancing voltage
        gfx->setCursor(leftOffset, screenHeight/2);
        gfx->println(" Balancing votage: " + String(BalV));
      break;
    }
  } else {
    gfx->setCursor(leftOffset,10 );
    gfx->println(".../ Control PSU ");

    //1
    gfx->setCursor(leftOffset, titleOffset + elementOffset);
    gfx->println(" PSU Voltage: " + String(3.3/4096 *PsuSetV));
    //2
    gfx->setCursor(leftOffset, titleOffset + 2 * elementOffset);
    gfx->println(" PSU Current: " + String(3.3/4096 *PsuSetI));
    //3
    gfx->setCursor(leftOffset, titleOffset + 3 * elementOffset);
    gfx->println(" Balancing votage: " + String(3.3/4096 *BalV));
    //4
    gfx->setCursor(leftOffset, titleOffset + 4 * elementOffset);
    gfx->println(" Balancing (ON/OFF)?: " + String(balanceOn));
    //5
    gfx->setCursor(leftOffset, titleOffset + 5 * elementOffset);
    gfx->println(" Start charging (ON/OFF): " + String(chargeOn));
  }
}

void drawSubMenu3(){
    if(inValueMenu){
    switch(currentSubMenu){
      case 1:
        gfx->setCursor(leftOffset, screenHeight/2);
        gfx->println(" Load current setting: ");
      break;
      case 2: //Balancing voltage
        gfx->setCursor(leftOffset, screenHeight/2);
        gfx->println(" Enable load: ");
      break;
    }
  }

  gfx->setCursor(leftOffset, 10);
  gfx->println(".../ Control Load ");

  gfx->setCursor(leftOffset, titleOffset + elementOffset);
  gfx->println(" Load current setting: ");

  gfx->setCursor(leftOffset, titleOffset + 2 * elementOffset);
  gfx->println(" Enable load: ");
}

void drawMenu(){  
  gfx->fillScreen(BLACK);
  gfx->setTextColor(WHITE,BLACK);

 if (inSubMenu) {
    switch (currentMenu) {
      case 1: drawSubMenu1(); break;  
      case 2: drawSubMenu2(); break;   
      case 3: drawSubMenu3(); break;
    }
  } else {
    drawMainMenu();
  }
}

void drawMenuSelsected(){
  gfx->setTextColor(BLACK,WHITE);

  switch(currentMenu){
    case 1:   
      if(inSubMenu){
        switch(currentSubMenu){
          case 1:
            gfx->setCursor(leftOffset, titleOffset+1*elementOffset);
            gfx->println(" Bat1 V: " + String(3.3/4096 * ch0Val) + "V");
          break;
          case 2:
            gfx->setCursor(leftOffset, titleOffset+2*elementOffset);
            gfx->println(" Bat2 V: " + String(3.3/4096 * ch1Val) + "V");
          break;
          case 3:
            gfx->setCursor(leftOffset, titleOffset+3*elementOffset);
            gfx->println(" Bat3 V: " + String(3.3/4096 * ch2Val) + "V");
          break;
          case 4:
            gfx->setCursor(leftOffset, titleOffset+4*elementOffset);
            gfx->println(" Bat4 V: " + String(3.3/4096 * ch3Val) + "V");
          break;
          case 5:
            gfx->setCursor(leftOffset, titleOffset+5*elementOffset);
            gfx->println(" Bat5 V: " + String(3.3/4096 * ch4Val) + "V");
          break;
          case 6:
            gfx->setCursor(leftOffset, titleOffset+6*elementOffset);
            gfx->println(" Bat6 V: " + String(3.3/4096 * ch5Val) + "V");
          break;
        }
      } else {
        gfx->setCursor(leftOffset, titleOffset + elementOffset);
        gfx->println(" Measure voltages ");
      }
    break;
    case 2:
      if(inSubMenu){
        switch(currentSubMenu){
          case 1:
            gfx->setCursor(leftOffset, titleOffset + elementOffset);
            gfx->println(" PSU Voltage: " + String(3.3/4096 *PsuSetV));
          break;
          case 2:
            gfx->setCursor(leftOffset, titleOffset + 2 * elementOffset);
            gfx->println(" PSU Current: " + String(3.3/4096 *PsuSetI));
          break;
          case 3:
            gfx->setCursor(leftOffset, titleOffset + 3 * elementOffset);
            gfx->println(" Balancing votage: " + String(3.3/4096 *BalV));
          break;
          case 4:
            gfx->setCursor(leftOffset, titleOffset + 4 * elementOffset);
            gfx->println(" Balancing (ON/OFF)?: " + String(balanceOn));
          break;
          case 5:
            gfx->setCursor(leftOffset, titleOffset + 5 * elementOffset);
            gfx->println(" Start charging (ON/OFF): " + String(chargeOn));
          break;
        }
      } else {
        gfx->setCursor(leftOffset, titleOffset + 2 * elementOffset);
        gfx->println(" Control PSU ");
      }
    break;
    case 3:
      if(inSubMenu){
        switch(currentSubMenu){
          case 1:
            gfx->setCursor(leftOffset, titleOffset + elementOffset);
            gfx->println(" Load current setting: ");
          break;
          case 2:
            gfx->setCursor(leftOffset, titleOffset + 3 * elementOffset);
            gfx->println(" Enable load: ");
          break;
        } 
      } else {
        gfx->setCursor(leftOffset, titleOffset + 3 * elementOffset);
        gfx->println(" Control Load ");
      }
    break;
  }
}


void encCLK_Interrupt (){

  if(digitalRead(DI_ENC_DT)){    //Rotation CW
    if(inValueMenu){
      if(currentMenu == 2){
        switch(currentSubMenu){
        case 1: PsuSetV += 128; break;
        case 2: PsuSetI += 128; break;
        case 3: BalV += 128;    break;
        }
      } else if(currentMenu == 3){
        switch(currentSubMenu){
        case 1: loadISmall += 128; break;
        case 2: loafILarge += 128; break;
        }
      }
    } else if(inSubMenu){
      currentSubMenu++;
      if(currentSubMenu > menuSub1ElementCnt && currentMenu == 1){
        currentSubMenu = 1;
      }
      else if(currentSubMenu > menuSub2ElementCnt && currentMenu == 2){
        currentSubMenu = 1;
      }
      else if(currentSubMenu > menuSub3ElementCnt && currentMenu == 3){
        currentSubMenu = 1;
      }
    } else {
        currentMenu++;
        if(currentMenu > menuMainElementCnt)
          currentMenu = 1;
      } 
  } else {    //Rotation CCW
    if(inValueMenu){
            if(currentMenu == 2){
        switch(currentSubMenu){
        case 1:
          PsuSetV -= 128;
          if (PsuSetV < 0)
            PsuSetV = 0;
        break;
        case 2:
          PsuSetI -= 128;
          if (PsuSetI < 0)
            PsuSetI = 0;
        break;
        case 3: 
          BalV -= 128;
          if (BalV < 0)
            BalV = 0;
        break;
        }
      } else if(currentMenu == 3){
        switch(currentSubMenu){
        case 1:
          loadISmall -= 128;
          if (loadISmall < 0)
            loadISmall = 0;
        break;
        case 2:
          loafILarge -= 128;
          if (loafILarge < 0)
            loafILarge = 0;
        break;
        }
      }
    } else if(inSubMenu){
      currentSubMenu--;
      if(currentSubMenu < 1){
        switch(currentMenu){
          case 1: currentSubMenu = menuSub1ElementCnt; break;
          case 2: currentSubMenu = menuSub2ElementCnt; break;
          case 3: currentSubMenu = menuSub3ElementCnt; break;
        }
      }
    } else {
      currentMenu--;
      if(currentMenu < 1)
        currentMenu = menuMainElementCnt;
    }
  }
  drawMenu();
  drawMenuSelsected();
}

void encSW_Interrupt(){ //The "Go In" button
  if (!inSubMenu){
    inSubMenu = true;
    inValueMenu = false;
  } 
  else if (!inValueMenu){
    inSubMenu = true;
    inValueMenu = true;
  }
  drawMenu();
  drawMenuSelsected();
}

void switch1Press(){  //The "Go Back" button

  /*
  void updateValues(){
    analogWrite(AQ_PSU_SETV, (4096/3.3)*0); //*10
    analogWrite(AQ_PSU_SETI, (4096/3.3)*0);
    analogWrite(AQ_BAL_BALV, (4096/3.3)*0);
    analogWrite(AQ_LOAD_SETI,(4096/3.3)*0);
  }
  */
  
  if (inSubMenu){
    inSubMenu = false;
    inValueMenu = false;
  } 
  else if (inValueMenu){
    inSubMenu = true;
    inValueMenu = false;
  }
  drawMenu();
  drawMenuSelsected();
}

void switch2Press(){  //The "Shut it down" button/output enable
  if(inValueMenu){
    if(currentMenu == 2){
      if(currentSubMenu == 4)
        balanceOn = !balanceOn;
      else if (currentSubMenu == 4)
       chargeOn = !chargeOn;
    }
  }
  delay(1);
}

