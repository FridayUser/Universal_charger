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

bool encCLK;
bool encDT;

const int screenHeight = 240;
const int screenWidth = 320;
const int menusAmount = 5;

int currentMenu = 1;


void setup(void)
{
  Serial.begin(115200);
  // Serial.setDebugOutput(true);
  // while(!Serial);
  Serial.println("Arduino_GFX Hello World example");

#ifdef GFX_EXTRA_PRE_INIT
  GFX_EXTRA_PRE_INIT();
#endif

  // Init Display
  if (!gfx->begin())
  {
    Serial.println("gfx->begin() failed!");
  }
  gfx->fillScreen(BLACK);

#ifdef GFX_BL
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);
#endif

  drawMenu();

  pinMode(DI_ENC_CLK, INPUT_PULLUP);
  pinMode(DI_ENC_DT, INPUT_PULLUP);
  pinMode(DI_ENC_SW, INPUT_PULLUP);

  encCLK = false;
  encDT = false;

  attachInterrupt(digitalPinToInterrupt(DI_ENC_CLK), encCLK_Interrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(DI_ENC_DT), encDT_Interrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(DI_ENC_SW), encSW_Interrupt, RISING);

  delay(5000); // 5 seconds
}

void loop()
{

  drawMenu();
  
  switch (currentMenu){
    case 1:
      gfx->setCursor(10, 0*screenHeight/menusAmount);
      gfx->setTextColor(BLACK,WHITE);
      gfx->println(" Measure voltages ");
    break;
    case 2:
      gfx->setCursor(10, 1*screenHeight/menusAmount);
      gfx->setTextColor(BLACK,WHITE);
      gfx->println(" Control PSU ");
    break;
    case 3:
      gfx->setCursor(10, 2*screenHeight/menusAmount);
      gfx->setTextColor(BLACK,WHITE);
      gfx->println(" Set Balancer ");
    break;
    case 4:
      gfx->setCursor(10, 3*screenHeight/menusAmount);
      gfx->setTextColor(BLACK,WHITE);
      gfx->println(" Control Load ");
    break;
    case 5:
      gfx->setCursor(10, 4*screenHeight/menusAmount);
      gfx->setTextColor(BLACK,WHITE);
      gfx->println(" Enable Bat Power ");
    break;
  }

  // gfx->setCursor(random(gfx->width()), random(gfx->height()));
  // gfx->setTextColor(random(0xffff), random(0xffff));
  // gfx->setTextSize(random(6) /* x scale */, random(6) /* y scale */, random(2) /* pixel_margin */);
  // gfx->println("Hello world!");

  delay(1000); // 1 second
}

void drawMenu () {
  gfx->setTextSize(2,2);
  
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

void encCLK_Interrupt (){
  delay(30);
  encCLK = true;

  if(encCLK == encDT){
    currentMenu++;
    if(currentMenu > menusAmount)
      currentMenu = 1;
  }
  else {
    currentMenu--;
    if(currentMenu < 1)
      currentMenu = menusAmount;
  }

  encDT = false;
}

void encDT_Interrupt (){
  delay(30);
  encDT = true;

  // if(encCLK == encDT){
  //   currentMenu++;
  //   if(currentMenu > menusAmount)
  //     currentMenu = 1;
  // }
  // else {
  //   currentMenu--;
  //   if(currentMenu < 1)
  //     currentMenu = menusAmount;
  // }

  encCLK = false;
}

void encSW_Interrupt(){

}








