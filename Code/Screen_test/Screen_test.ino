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
-Dokończyć wymianę stałych na zmienne w pozostałych pozycjach menu
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

void setup(void)
{
  Serial.begin(9600);
  // Serial.setDebugOutput(true);
  // while(!Serial);
  Serial.println("Arduino_GFX Hello World example");

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

  pinMode(MUX_A, OUTPUT);
  pinMode(MUX_B, OUTPUT);
  pinMode(MUX_C, OUTPUT);

  pinMode(MUX_ADC, INPUT);
  analogReadResolution(12);

  pinMode(25, OUTPUT);
  digitalWrite(25, HIGH);

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

  delay(2000);  //Cosmetic delay, can delete

  gfx->fillScreen(BLACK);
  drawMenu();
}

void loop()
{
  measureParameters(ch0Val, ch1Val, ch2Val, ch3Val, ch4Val, ch5Val, ch6Val, ch7Val);
  delay(10);  //Sleep well little prince
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

int menuMainElementCnt; //Number of elements in main menu
int menuSub1ElementCnt; //Number of elements inside the sub menu 1, not including menu title 
int menuSub2ElementCnt;
int menuSub3ElementCnt;

int titleOffset = 20;   //Offset between title and first menu element
int elementOffset = 20; //Offset between menu elements
int leftOffset = 10;    //Offest from left screen edge

void drawMenu(){  
  gfx->fillScreen(BLACK);
  gfx->setTextColor(WHITE,BLACK);

 if (inSubMenu) {
    switch (currentMenu) {
      case 1:
        menuSub1ElementCnt = 6;    //Update with current element count
        gfx->setCursor(leftOffset, 10);
        gfx->println(" Measure voltages ");

        for (int i = 0; i < menuSub1ElementCnt; i++) {
          int channelValue = 0;
          switch (i) {
            case 0: channelValue = ch0Val; break;
            case 1: channelValue = ch1Val; break;
            case 2: channelValue = ch2Val; break;
            case 3: channelValue = ch3Val; break;
            case 4: channelValue = ch4Val; break;
            case 5: channelValue = ch5Val; break;
          }
          gfx->setCursor(leftOffset, titleOffset + (i + 1) * elementOffset);
          gfx->println(" Bat" + String(i + 1) + " V: ");
          gfx->setCursor(50, titleOffset + (i + 1) * elementOffset);
          gfx->println(channelValue);
        }
        break;  
      case 2:
        menuSub2ElementCnt = 4; 
        if(inValueMenu){
        switch(currentSubMenu){
        case 1:
          gfx->setCursor(leftOffset, screenHeight/2);
          gfx->println(" PSU Voltage: ");
          gfx->setCursor(100, screenHeight/2);
          gfx->println(PsuSetV);  //Natsawiane z przerwania od onkoder clk
        break;
        case 2:
          gfx->setCursor(leftOffset, screenHeight/2);
          gfx->println(" PSU Current: ");
          gfx->setCursor(100, screenHeight/2);
          gfx->println(PsuSetI);
        break;
        case 3: //Balancing voltage
        break;
        }}
           //Update with current element count

        gfx->setCursor(leftOffset,10 );
        gfx->println(" Control PSU ");

        gfx->setCursor(leftOffset, titleOffset + elementOffset);
        gfx->println(" PSU Voltage: ");
        gfx->setCursor(100, screenHeight/2);
        gfx->println(PsuSetV);

        gfx->setCursor(leftOffset, titleOffset + 2 * elementOffset);
        gfx->println(" PSU Current: ");
        gfx->setCursor(100, screenHeight/2);
        gfx->println(PsuSetI);

        gfx->setCursor(leftOffset, titleOffset + 3 * elementOffset);
        gfx->println(" Balancing (ON/OFF)?: ");

        gfx->setCursor(leftOffset, titleOffset + 4 * elementOffset);
        gfx->println(" Start charging (ON/OFF): ");
        break;   
      case 3:
        menuSub3ElementCnt = 2;    //Update with current element count
        gfx->setCursor(leftOffset, 10);
        gfx->println(" Control Load ");

        gfx->setCursor(leftOffset, titleOffset + elementOffset);
        gfx->println(" Load current setting: ");

        gfx->setCursor(leftOffset, titleOffset + 2 * elementOffset);
        gfx->println(" Enable load: ");
        break;
    }
  } else {
    menuMainElementCnt  = 3;    //Update with current element count
    gfx->setCursor(leftOffset, 10);
    gfx->println(" Measure voltages ");

    gfx->setCursor(leftOffset, titleOffset + elementOffset);
    gfx->println(" Control PSU ");

    gfx->setCursor(leftOffset, titleOffset + 2 * elementOffset);
    gfx->println(" Control Load ");
  }
}

void drawMenuSelsected(){
  gfx->setTextColor(BLACK,WHITE);
  switch(currentMenu){
    case 1:
      if(inSubMenu){
        switch(currentSubMenu){
          case 1:
       
           for (int i = 0; i < 5; i++) {
            int channelValue = 0;
            switch (i) {
              case 0: channelValue = ch0Val; break;
              case 1: channelValue = ch1Val; break;
              case 2: channelValue = ch2Val; break;
              case 3: channelValue = ch3Val; break;
              case 4: channelValue = ch4Val; break;
            }
            gfx->setCursor(leftOffset, titleOffset + (i + 1) * elementOffset);
            gfx->println(" Bat" + String(i + 1) + " V: ");
            gfx->setCursor(50, titleOffset + (i + 1) * elementOffset);
            gfx->println(channelValue);
           }
        break;  
        }
      } else {
        gfx->setCursor(leftOffset, 10);
        gfx->println(" Measure voltages ");
      }
    break;
    case 2:
      if(inSubMenu){
        switch(currentSubMenu){
          case 1:
            gfx->setCursor(leftOffset, titleOffset + elementOffset);
            gfx->println(" Single cell max voltage: ");
          break;
          case 2:
            gfx->setCursor(leftOffset, titleOffset + 2 * elementOffset);
            gfx->println(" Single cell max current: ");
          break;
        }
      } else {
        gfx->setCursor(leftOffset, titleOffset + elementOffset);
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
        gfx->setCursor(leftOffset, titleOffset + 2 * elementOffset);
        gfx->println(" Control Load ");
      }
    break;
  }
}

void encCLK_Interrupt (){
  bool encDT = digitalRead(DI_ENC_DT);
  if(encDT){
    if(inValueMenu){
      if(currentMenu == 2)
      switch(currentSubMenu){
        case 1:
          PsuSetV--;
          if (PsuSetV < 0)
            PsuSetV = 0;
        break;
        case 2:
          PsuSetI--;
          if (PsuSetI < 0)
            PsuSetI = 0;
        break;
        case 3: //Balancing voltage
        break;

      }
    } else if(inSubMenu){
      currentSubMenu--;
      if(currentSubMenu > 5 && currentMenu == 1){
        currentSubMenu = 1;
      }
      else if(currentSubMenu > 2 && currentMenu == 2){
        currentSubMenu = 1;
      }
      else if(currentSubMenu > 2 && currentMenu == 3){
        currentSubMenu = 1;
      }
    } else {
        currentMenu--;
        if(currentMenu < 1)
          currentMenu = 3;
      } 
  } else {
    if(inSubMenu){
      currentSubMenu++;
      if(currentSubMenu < 1){
        switch(currentMenu){
          case 1:
            currentSubMenu = 5;
          break;
          case 2:
            currentSubMenu = 2;
          break;
          case 3:
            currentSubMenu = 2;
          break;
        }
      }
    } else {
      currentMenu++;
      if(currentMenu > 3)
        currentMenu = 1;
    }
  }
  drawMenu();
  drawMenuSelsected();
}

void encSW_Interrupt(){ //The "Go In" button
  if (!inSubMenu){
    inValueMenu = false;
  } 
  else if (!inValueMenu){
    inSubMenu = true;
  }
  drawMenu();
  drawMenuSelsected();
}

void switch1Press(){  //The "Go Back" button
  if (inSubMenu){
    inValueMenu = false;
  } 
  else if (inValueMenu){
    inSubMenu = true;
  }
  drawMenu();
  drawMenuSelsected();
}

void switch2Press(){  //The "Shut it down" button/output enable
  //Sprawdź czy jesteś w submenu od łądowania żeby potwirdzić wartości, jeżeli tak i wciśniesz przycisk to załącz wyjście
  delay(1);
}







