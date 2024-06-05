
/* TODO: Wymyślić sposób na wyświetlanie menu w odpowiedni sposób. Myślę żeby zrobić to tak:
* Główne menu to liczby w case od 1-9, kręcenie enkoderem zmienia o 1
* Menu pomocnicze to liczby od 1x-9x, wciścnięcie przycisku mnoży razy 10
* przytrzmanie przycisku dzieli przez 10.
* Jedna metoda do rysowania i obsługi menu i wszystkich funkcji
* 
* Dodać bity które będą określać w którym tierze menu się znajduję
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

const int screenHeight = 240;
const int screenWidth = 320;
const int menusAmount = 5;

int currentMenu = 1;


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

  #ifdef GFX_BL
    pinMode(GFX_BL, OUTPUT);
    digitalWrite(GFX_BL, HIGH);
  #endif

  pinMode(DI_ENC_CLK, INPUT_PULLUP);
  pinMode(DI_ENC_DT, INPUT_PULLUP);
  pinMode(DI_ENC_SW, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(DI_ENC_CLK), encCLK_Interrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(DI_ENC_SW), encSW_Interrupt, CHANGE);

  delay(2000);

  gfx->fillScreen(BLACK);
  drawMenu();
}

void loop()
{
  delay(10);  //Sleep well little prince
}

void drawMenu(){  
  gfx->setCursor(10, 0*screenHeight/menusAmount);
  gfx->setTextColor(WHITE,BLACK);
  gfx->println(" Measure voltages ");

  gfx->setCursor(10, 1*screenHeight/menusAmount);
  gfx->setTextColor(WHITE,BLACK);
  gfx->println(" Control PSU ");

  gfx->setCursor(10, 2*screenHeight/menusAmount);
  gfx->setTextColor(WHITE,BLACK);
  gfx->println(" Set Balancer ");

  gfx->setCursor(10, 3*screenHeight/menusAmount);
  gfx->setTextColor(WHITE,BLACK);
  gfx->println(" Control Load ");

  gfx->setCursor(10, 4*screenHeight/menusAmount);
  gfx->setTextColor(WHITE,BLACK);
  gfx->println(" Enable Bat Power ");
}

void drawSelsectedMenu(){
  switch (currentMenu){
    case 1:
      drawMenu();
      gfx->setCursor(10, 0*screenHeight/menusAmount);
      gfx->setTextColor(BLACK,WHITE);
      gfx->println(" Measure voltages ");
    break;
    case 2:
      drawMenu();
      gfx->setCursor(10, 1*screenHeight/menusAmount);
      gfx->setTextColor(BLACK,WHITE);
      gfx->println(" Control PSU ");
    break;
    case 3:
      drawMenu();
      gfx->setCursor(10, 2*screenHeight/menusAmount);
      gfx->setTextColor(BLACK,WHITE);
      gfx->println(" Set Balancer ");
    break;
    case 4:
      drawMenu();
      gfx->setCursor(10, 3*screenHeight/menusAmount);
      gfx->setTextColor(BLACK,WHITE);
      gfx->println(" Control Load ");
    break;
    case 5:
      drawMenu();
      gfx->setCursor(10, 4*screenHeight/menusAmount);
      gfx->setTextColor(BLACK,WHITE);
      gfx->println(" Enable Bat Power ");
    break;
  }
}

void drawSubMenu1 () {  
  gfx->setCursor(10, 0*screenHeight/menusAmount);
  gfx->setTextColor(WHITE,BLACK);
  gfx->println(" yes ");

  gfx->setCursor(10, 1*screenHeight/menusAmount);
  gfx->setTextColor(WHITE,BLACK);
  gfx->println(" yes ");

  gfx->setCursor(10, 2*screenHeight/menusAmount);
  gfx->setTextColor(WHITE,BLACK);
  gfx->println(" yes ");

  gfx->setCursor(10, 3*screenHeight/menusAmount);
  gfx->setTextColor(WHITE,BLACK);
  gfx->println(" yes ");

  gfx->setCursor(10, 4*screenHeight/menusAmount);
  gfx->setTextColor(WHITE,BLACK);
  gfx->println(" no ");
}

void drawSelectedSubMenu1(){
  switch (currentMenu){
    case 11:
      gfx->setCursor(10, 0*screenHeight/menusAmount);
      gfx->setTextColor(BLACK,WHITE);
      gfx->println(" yes ");
    break;
    case 12:
      gfx->setCursor(10, 1*screenHeight/menusAmount);
      gfx->setTextColor(BLACK,WHITE);
      gfx->println(" yes ");
    break;
    case 13:
      gfx->setCursor(10, 2*screenHeight/menusAmount);
      gfx->setTextColor(BLACK,WHITE);
      gfx->println(" yes ");
    break;
    case 14:
      gfx->setCursor(10, 3*screenHeight/menusAmount);
      gfx->setTextColor(BLACK,WHITE);
      gfx->println(" yes ");
    break;
    case 15:
      gfx->setCursor(10, 4*screenHeight/menusAmount);
      gfx->setTextColor(BLACK,WHITE);
      gfx->println(" yes ");
    break;
  }
}

void encCLK_Interrupt (){

  bool encDT = digitalRead(DI_ENC_DT);

  if(encDT){
    currentMenu++;
    if(currentMenu > menusAmount)
      currentMenu = 1;
  }
  else {
    currentMenu--;
    if(currentMenu < 1)
      currentMenu = menusAmount;
  }
  drawSelsectedMenu();

}

void encSW_Interrupt(){
  // If the button is pressed it generates a risig and falling edge. We chceck witch edge it is. 
  // If it was the rising edge, we count the time to the falling edge. If it was shorter than 2 seconds
  // the button was short pressed and we go into submenu. Else we return form submenu.

  bool encSW = digitalRead(DI_ENC_SW);
  Serial.println("Button is press");
  int currentTime = millis();
  int pressTime; 

  if(!encSW){
    Serial.println("Time is check");
    pressTime = currentTime;
  } else {
    if (currentTime - pressTime < 2000){
        Serial.println("Time is smoll");
        gfx->fillScreen(RED);
        currentTime = millis();
        delay(1);
    }
    if(currentTime - pressTime > 2000){
      //currentMenu = currentMenu/10;
      Serial.println("Time is big");
      gfx->fillScreen(BLUE);
    }
  }
}









