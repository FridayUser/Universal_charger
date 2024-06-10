
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

const int menusAmount = 5;
const int screenWidth= gfx->height();
const int screenHeight = gfx->width();

int currentMenu = 1;
int currentSubMenu = 1;
bool inSubMenu = false;
bool inValueMenu = false;

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

  pinMode(25, OUTPUT);
  digitalWrite(25, HIGH);

  #ifdef GFX_BL
    pinMode(GFX_BL, OUTPUT);
    digitalWrite(GFX_BL, HIGH);
  #endif

  pinMode(DI_ENC_CLK, INPUT_PULLUP);
  pinMode(DI_ENC_DT, INPUT_PULLUP);
  pinMode(DI_ENC_SW, INPUT_PULLUP);
  pinMode(DI_BTN2, INPUT_PULLUP);
  pinMode(DI_BTN1, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(DI_ENC_CLK), encCLK_Interrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(DI_ENC_SW), encSW_Interrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(DI_BTN2), switch1Press, RISING);
  attachInterrupt(digitalPinToInterrupt(DI_BTN1), switch2Press, RISING);

  delay(2000);

  gfx->fillScreen(BLACK);
  drawMenu();
}

void loop()
{
  delay(10);  //Sleep well little prince
}

void drawMenu(){  
  gfx->fillScreen(BLACK);
  gfx->setTextColor(WHITE,BLACK);

  if(inSubMenu && currentMenu == 1){
    gfx->setCursor(10, 10);
    gfx->println(" Measure voltages ");

    gfx->setCursor(10, 30);
    gfx->println(" Bat1 V: ");

    gfx->setCursor(10, 50);
    gfx->println(" Bat2 V: ");

    gfx->setCursor(10, 70);
    gfx->println(" Bat3 V: ");

    gfx->setCursor(10, 90);
    gfx->println(" Bat4 V: ");

    gfx->setCursor(10, 110);
    gfx->println(" Bat5 V: ");

  } else if (inSubMenu && currentMenu == 2) {
    gfx->setCursor(10, 10);
    gfx->println(" Control PSU ");

    gfx->setCursor(10, 30);
    gfx->println(" Single cell max voltage: ");

    gfx->setCursor(10, 50);
    gfx->println(" Single cell max current: ");

  } else if (inSubMenu && currentMenu == 3){
    gfx->setCursor(10, 10);
    gfx->println(" Control Load ");

    gfx->setCursor(10, 30);
    gfx->println(" Load current setting: ");

    gfx->setCursor(10, 70);
    gfx->println(" Enable load: ");

  } else {
    gfx->setCursor(10, 10);
    gfx->println(" Measure voltages ");

    gfx->setCursor(10, 30);
    gfx->println(" Control PSU ");

    gfx->setCursor(10, 50);
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
            gfx->setCursor(10, 30);
            gfx->println(" Bat1 V: ");
          break;
          case 2:
            gfx->setCursor(10, 50);
            gfx->println(" Bat2 V: ");
          break;
          case 3:
            gfx->setCursor(10, 70);
            gfx->println(" Bat3 V: ");
          break;
          case 4:
            gfx->setCursor(10, 90);
            gfx->println(" Bat4 V: ");
          break;
          case 5:
            gfx->setCursor(10, 110);
            gfx->println(" Bat5 V: ");
          break;
        }
      } else {
        gfx->setCursor(10, 10);
        gfx->println(" Measure voltages ");
      }
    break;
    case 2:
      if(inSubMenu){
        switch(currentSubMenu){
          case 1:
            gfx->setCursor(10, 30);
            gfx->println(" Single cell max voltage: ");
          break;
          case 2:
            gfx->setCursor(10, 50);
            gfx->println(" Single cell max current: ");
          break;
        }
      } else {
        gfx->setCursor(10, 30);
        gfx->println(" Control PSU ");
      }
    break;
    case 3:
      if(inSubMenu){
        switch(currentSubMenu){
          case 1:
            gfx->setCursor(10, 30);
            gfx->println(" Load current setting: ");
          break;
          case 2:
            gfx->setCursor(10, 70);
            gfx->println(" Enable load: ");
          break;
        } 
      } else {
        gfx->setCursor(10, 50);
        gfx->println(" Control Load ");
      }
    break;
  }
}

void encCLK_Interrupt (){
  bool encDT = digitalRead(DI_ENC_DT);
  if(encDT){
    if(inSubMenu){
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
        if(currentMenu > 3)
          currentMenu = 1;
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
      if(currentMenu < 1)
        currentMenu = 3;
    }
  }
  drawMenu();
  drawMenuSelsected();
}

void encSW_Interrupt(){
  inSubMenu = true;
  drawMenu();
  drawMenuSelsected();
}

void switch1Press(){
  inSubMenu = false;
  drawMenu();
  drawMenuSelsected();
}

void switch2Press(){
  delay(1);
}

// int startTime;
// int passedTime;

// void encSW_Interrupt(){
//   // If the button is pressed it generates a risig and falling edge. We chceck witch edge it is. 
//   // If it was the rising edge, we count the time to the falling edge. If it was shorter than 2 seconds
//   // the button was short pressed and we go into submenu. Else we return form submenu.

//   bool encSW = digitalRead(DI_ENC_SW);
//   Serial.println("Button is press");
  

//   if(!encSW){
//     Serial.println("Time start is check: ");
//     startTime = millis();
//     Serial.println(startTime);

//   } else {
//     Serial.println("Time stop is check");
//     passedTime = millis();
//     Serial.println(passedTime);

//     if (passedTime - startTime < 1000){
//       Serial.println("Time is smoll");
//       gfx->fillScreen(RED);
//     }
//     if(passedTime - startTime > 1000){
//       //currentMenu = currentMenu/10;
//       Serial.println("Time is big");
//       gfx->fillScreen(BLUE);
//     }
//   }
// }







