/*
Author: Błażej Chodorowski 263671
Last update: 23.06.2024
Project: Universal charger

This program implements the GUI for a universal charger using an Arduino with a Raspberry Pi Pico dev board. 
The GUI allows configuration and control of various parameters including voltages, currents, and reading parameters. 
It uses a single encoder and two buttons to navigate through menus and adjust settings.

Main Features:
- Displays a graphical menu system on a display connected to the Raspberry Pi Pico.
- Controls various functions of the charger including setting and reading voltages and currents.
- Utilizes interrupt-driven input from the encoder and buttons for navigation and parameter adjustment.

Hardware Configuration:
- The display is initialized using the Arduino_GFX_Library.
- Various pins are defined for output control (e.g., DQ_PSU_OFF, AQ_PSU_SETV).
- Multiplexer (MUX) configuration is used for reading multiple analog values.

TODO:
(- Fix scaling to all values (The corelation between set value and output value: cofigure correct scalers)
- Add auto update every x seconds to value readout page (in while loop do a menu redraw every x seconds)
- Configure and test electronic load settings, current ones are placeholder
- Probably a lot more tbh.
- Maybe rewrite everything so it doesn't suck to work with lol)

- Correct scaling for value settings and output correlation.
- Implement auto-update feature for readout values.
- Configure and test electronic load settings.
- Consider a code refactor for better maintainability.

Pin Definitions:
- DQ_PSU_OFF: 2, DQ_ONE_WIRE: 4, DQ_LOAD_EN: 8, DQ_BATP_ON: 10
- AQ_PSU_SETV: 3, AQ_PSU_SETI: 6, AQ_BAL_BALV: 7, AQ_LOAD_SETI: 9
- DI_ENC_CLK: 11, DI_ENC_DT: 12, DI_ENC_SW: 13, DI_BTN2: 14, DI_BTN1: 15
- MUX_A: 27, MUX_B: 28, MUX_C: 5, MUX_ADC: 26

Constants:
- Defined scalers and max values for voltage, current, and balancing voltages.
- Example: `psuSetVScler`, `psuSetVMax`, `loadIScaler`, `loadIMax`.

Functions:
- `setup()`: Initializes serial communication, display, and pins.
- `loop()`: Measures parameters and includes a placeholder for auto-update functionality.
- `setParameters()`: Configures the output parameters based on user input.
- `adcReadout(int chNum)`: Reads analog values from the specified channel.
- `measureParameters()`: Reads and updates all channel values.
- `drawMenu()`, `drawMainMenu()`, `drawSubMenu1()`, `drawSubMenu2()`, `drawSubMenu3()`: Functions to render the various menu screens.
- `drawMenuSelected()`: Highlights the selected menu item.
- Interrupt service routines (`encCLK_Interrupt()`, `encSW_Interrupt()`, `switch1Press()`, `switch2Press()`): Handle encoder and button input for menu navigation and parameter adjustment.

Notes:
- This code is work-in-progress and may require additional refinement and testing.
- Ensure proper scaling and calibration of voltage and current settings to achieve accurate outputs.
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
bool batteryOn = false;
bool chargeOn = false;
bool loadOn = false;

int loadISmall = 0;
int loadILarge = 0;

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
  gfx->println("I'm not envious of the preson maintaining this code xD, it's terrible");

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

  analogWrite(AQ_PSU_SETV, 1000);
  analogWrite(AQ_PSU_SETI, 2000);
  analogWrite(AQ_BAL_BALV, 3000);
  analogWrite(AQ_LOAD_SETI,0);

  digitalWrite(DQ_PSU_OFF, HIGH);
  digitalWrite(DQ_LOAD_EN, LOW);
  digitalWrite(DQ_BATP_ON, LOW);

  analogWriteFreq(1000);
  analogWriteResolution(12);

  #ifdef GFX_BL
    pinMode(GFX_BL, OUTPUT);
    digitalWrite(GFX_BL, HIGH);
  #endif

  pinMode(DI_ENC_CLK, INPUT); //Software pullup not used due to an incident (yes Rico, kaboom)
  pinMode(DI_ENC_DT, INPUT); 
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
  drawMenuSelsected();
}

//int initialTime;
//int currentTime;

void loop()
{
  measureParameters();
  delay(1);  //Sleep well little prince


  // TODO: Auto update of readout values
  // initialTime = millis();

  // if (inSubMenu && currentMenu == 1){
  //   const int timeDelta = 60;
  //   currentTime = millis();
  //   if(currentTime - initialTime > timeDelta){
  //     drawMenu();
  //     drawMenuSelsected();
  //   }
  // }
}

//TODO: Configures scalers and maxes correctly, psu setV is somewhat working but not 
//very accurate set<->output due to rounding probably because step of 12 gives change 
//of 0,09667 and not 0.1 i guess
// rest of the scalers and maxes are untested and set randomly

const int psuSetVScler = 12;
const int psuSetIScaler = 12;
const int balVScaler = 128;
const int loadIScaler = 128;

const float psuSetVMax = 33.0;
const float psuSetIMax = 3.3;
const float balVMax = 5.55;

const float loadIMax  = 16.5;

void setParameters(){
  //(Nmax/Vmax * Vset/10 where Vset is stored as Nset so needs to be scaled with scaler, stupid, i know)
  analogWrite(AQ_PSU_SETV, (4096/psuSetVMax)*(PsuSetV/(10*psuSetVScler)));
  analogWrite(AQ_PSU_SETI, (4096/psuSetIMax)*(PsuSetI/(10*psuSetIScaler)));
  analogWrite(AQ_BAL_BALV, (4096/balVMax)*(BalV/(10*psuSetIScaler)));

  digitalWrite(DQ_PSU_OFF, !chargeOn);
  digitalWrite(DQ_LOAD_EN, loadOn);
  digitalWrite(DQ_BATP_ON, batteryOn);
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

void measureParameters(){
  const int measTime = 1;
  
  ch0Val = adcReadout(0);
  delay(measTime);  //MUX channel switch time

  ch1Val = adcReadout(1); delay(measTime);
  ch2Val = adcReadout(2); delay(measTime);
  ch3Val = adcReadout(3); delay(measTime);
  ch4Val = adcReadout(4); delay(measTime);
  ch5Val = adcReadout(5); delay(measTime);
  ch6Val = adcReadout(6); delay(measTime);
  ch7Val = adcReadout(7); delay(measTime);
}


const int menuMainElementCnt = 3; //Number of elements in main menu
const int menuSub1ElementCnt = 8; //Number of elements inside the sub menu 1, not including menu title 
const int menuSub2ElementCnt = 5; //...
const int menuSub3ElementCnt = 3; //...

const int titleOffset = 20;   //Offset between title and first menu element
const int elementOffset = 20; //Offset between menu elements
const int leftOffset = 10;    //Offest from left screen edge

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

void drawMainMenu(){
    gfx->setCursor(leftOffset, 10);
    gfx->println("== Very good charger ==");

    gfx->setCursor(leftOffset, titleOffset + elementOffset);
    gfx->println(" Measure voltages -> ");

    gfx->setCursor(leftOffset, titleOffset + 2 * elementOffset);
    gfx->println(" Control PSU -> ");

    gfx->setCursor(leftOffset, titleOffset + 3 * elementOffset);
    gfx->println(" Control Load -> ");
}

void drawSubMenu1(){
  gfx->setCursor(leftOffset, 10);
  gfx->println(".../ Measure voltages ");

  gfx->setCursor(leftOffset, titleOffset+elementOffset);
  gfx->println(" Voltage?: " + String(psuSetVMax/4096 * ch6Val) + "V");

  gfx->setCursor(leftOffset, titleOffset+2*elementOffset);
  gfx->println(" Current?: " + String(psuSetIMax/4096 * ch7Val) + "A");

  gfx->setCursor(leftOffset, titleOffset+3*elementOffset);
  gfx->println(" Bat1 V: " + String(balVMax/4096 * ch0Val) + "V");

  gfx->setCursor(leftOffset, titleOffset+4*elementOffset);
  gfx->println(" Bat2 V: " + String(balVMax/4096 * ch1Val) + "V");

  gfx->setCursor(leftOffset, titleOffset+5*elementOffset);
  gfx->println(" Bat3 V: " + String(balVMax/4096 * ch2Val) + "V");

  gfx->setCursor(leftOffset, titleOffset+6*elementOffset);
  gfx->println(" Bat4 V: " + String(balVMax/4096 * ch3Val) + "V");

  gfx->setCursor(leftOffset, titleOffset+7*elementOffset);
  gfx->println(" Bat5 V: " + String(balVMax/4096 * ch4Val) + "V");

  gfx->setCursor(leftOffset, titleOffset+8*elementOffset);
  gfx->println(" Bat6 V: " + String(balVMax/4096 * ch5Val) + "V");
}

void drawSubMenu2(){
  if(inValueMenu){
    switch(currentSubMenu){
      case 1:
        gfx->setCursor(leftOffset, screenHeight/2);
        gfx->println(" PSU Voltage: " + String(psuSetVMax/4096 * PsuSetV));
      break;
      case 2:
        gfx->setCursor(leftOffset, screenHeight/2);
        gfx->println(" PSU Current: " + String(psuSetIMax/4096 * PsuSetI));
      break;
      case 3: //Balancing voltage
        gfx->setCursor(leftOffset, screenHeight/2);
        gfx->println(" Balancing votage: " + String(balVMax/4096 * BalV));
      break;
    }
  } else {
    gfx->setCursor(leftOffset,10 );
    gfx->println(".../ Control PSU ");

    //1
    gfx->setCursor(leftOffset, titleOffset + elementOffset);
    gfx->println(" PSU Voltage: " + String(psuSetVMax/4096 * PsuSetV));
    //2
    gfx->setCursor(leftOffset, titleOffset + 2 * elementOffset);
    gfx->println(" PSU Current: " + String(psuSetIMax/4096 * PsuSetI));
    //3
    gfx->setCursor(leftOffset, titleOffset + 3 * elementOffset);
    gfx->println(" Balancing votage: " + String(balVMax/4096 * BalV));
    //4
    gfx->setCursor(leftOffset, titleOffset + 4 * elementOffset);
    gfx->println(" Battery output (1/0): " + String(batteryOn));
    //5
    gfx->setCursor(leftOffset, titleOffset + 5 * elementOffset);
    gfx->println(" Start charging (1/0): " + String(chargeOn));
  }
}

void drawSubMenu3(){
    if(inValueMenu){
    switch(currentSubMenu){
      case 1:
        gfx->setCursor(leftOffset, screenHeight/2);
        gfx->println(" Small current: "  + String(loadIMax/4096 * loadISmall));
      break;
      case 2:
        gfx->setCursor(leftOffset, screenHeight/2);
        gfx->println(" Large current: "  + String(loadIMax/4096 * loadILarge));
      break;
      case 3: //Balancing voltage
        gfx->setCursor(leftOffset, screenHeight/2);
        gfx->println(" Enable load (1/0): ");
      break;
    }
  } else {
    gfx->setCursor(leftOffset, 10);
    gfx->println(".../ Control Load ");

    gfx->setCursor(leftOffset, titleOffset + elementOffset);
    gfx->println(" Small current: " + String(loadIMax/4096 * loadISmall));

    gfx->setCursor(leftOffset, titleOffset + 2 * elementOffset);
    gfx->println(" Large current: " + String(loadIMax/4096 * loadILarge));

    gfx->setCursor(leftOffset, titleOffset + 3 * elementOffset);
    gfx->println(" Enable load (0/1): ");
  }
}


void drawMenuSelsected(){
  gfx->setTextColor(BLACK,WHITE);

  switch(currentMenu){
    case 1:   
      if(inSubMenu){
        switch(currentSubMenu){
          case 1:
            gfx->setCursor(leftOffset, titleOffset+elementOffset);
            gfx->println(" Voltage?: " + String(psuSetVMax/4096 * ch6Val) + "V");
          break;
          case 2:
            gfx->setCursor(leftOffset, titleOffset+2*elementOffset);
            gfx->println(" Current?: " + String(psuSetIMax/4096 * ch7Val) + "A");
          break;
          case 3:
            gfx->setCursor(leftOffset, titleOffset+3*elementOffset);
            gfx->println(" Bat1 V: " + String(balVMax/4096 * ch0Val) + "V");
          break;
          case 4:
            gfx->setCursor(leftOffset, titleOffset+4*elementOffset);
            gfx->println(" Bat2 V: " + String(balVMax/4096 * ch1Val) + "V");
          break;
          case 5:
            gfx->setCursor(leftOffset, titleOffset+5*elementOffset);
            gfx->println(" Bat3 V: " + String(balVMax/4096 * ch2Val) + "V");
          break;
          case 6:
            gfx->setCursor(leftOffset, titleOffset+6*elementOffset);
            gfx->println(" Bat4 V: " + String(balVMax/4096 * ch3Val) + "V");
          break;
          case 7:
            gfx->setCursor(leftOffset, titleOffset+7*elementOffset);
            gfx->println(" Bat5 V: " + String(balVMax/4096 * ch4Val) + "V");
          break;
          case 8:
            gfx->setCursor(leftOffset, titleOffset+8*elementOffset);
            gfx->println(" Bat6 V: " + String(balVMax/4096 * ch5Val) + "V");
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
            gfx->println(" PSU Voltage: " + String(psuSetVMax/4096 * PsuSetV));
          break;
          case 2:
            gfx->setCursor(leftOffset, titleOffset + 2 * elementOffset);
            gfx->println(" PSU Current: " + String(psuSetIMax/4096 * PsuSetI));
          break;
          case 3:
            gfx->setCursor(leftOffset, titleOffset + 3 * elementOffset);
            gfx->println(" Balancing votage: " + String(balVMax/4096 *BalV));
          break;
          case 4:
            gfx->setCursor(leftOffset, titleOffset + 4 * elementOffset);
            gfx->println(" Battery output (1/0): " + String(batteryOn));
          break;
          case 5:
            gfx->setCursor(leftOffset, titleOffset + 5 * elementOffset);
            gfx->println(" Start charging (1/0): " + String(chargeOn));
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
            gfx->println(" Small current: " + String(loadIMax/4096 * 5/loadISmall));
          break;
          case 2:
            gfx->setCursor(leftOffset, titleOffset + 2 * elementOffset);
            gfx->println(" Large current: " + String(loadIMax/4096 * 5/loadILarge));
          break;
          case 3:
            gfx->setCursor(leftOffset, titleOffset + 3 * elementOffset);
            gfx->println(" Enable load (1/0): ");
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

  if(!digitalRead(DI_ENC_DT)){    //Rotation CW
    if(inValueMenu){
      if(currentMenu == 2){
        switch(currentSubMenu){
        case 1: PsuSetV += psuSetVScler; break;
        case 2: PsuSetI += psuSetIScaler; break;
        case 3: BalV += balVScaler;    break;
        }
      } else if(currentMenu == 3){
        switch(currentSubMenu){
        case 1: loadISmall += loadIScaler; break;
        case 2: loadILarge += loadIScaler; break;
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
          PsuSetV -= psuSetVScler;
          if (PsuSetV < 0)
            PsuSetV = 0;
        break;
        case 2:
          PsuSetI -= psuSetIScaler;
          if (PsuSetI < 0)
            PsuSetI = 0;
        break;
        case 3: 
          BalV -= balVScaler;
          if (BalV < 0)
            BalV = 0;
        break;
        }
      } else if(currentMenu == 3){
        switch(currentSubMenu){
        case 1:
          loadISmall -= loadIScaler;
          if (loadISmall < 0)
            loadISmall = 0;
        break;
        case 2:
          loadILarge -= loadIScaler;
          if (loadILarge < 0)
            loadILarge = 0;
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

  if (inValueMenu){
    inSubMenu = true;
    inValueMenu = false;
  }
  else if (inSubMenu){
    inSubMenu = false;
    inValueMenu = false;
  } 
  drawMenu();
  drawMenuSelsected();
  setParameters();
}

void switch2Press(){  //The "Shut it down" button/output enable
  if(inValueMenu){
    if(currentMenu == 2){
      if(currentSubMenu == 4)
        batteryOn = !batteryOn;
      else if (currentSubMenu == 5)
        chargeOn = !chargeOn;
    }
  }
  drawMenu();
  drawMenuSelsected();
  setParameters();
}