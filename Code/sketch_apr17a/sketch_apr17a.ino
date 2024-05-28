#include <Arduino_GFX_Library.h>

#define GFX_BL 28 // Dostosuj zgodnie ze swoim pinem podświetlenia

#define BACKGROUND BLACK
#define TEXT_COLOR WHITE
#define HIGHLIGHT_COLOR BLUE

// Enkoder
#define ENCODER_PIN_A 2
#define ENCODER_PIN_B 3
#define ENCODER_BUTTON_PIN 4

volatile int encoderPos = 0;
int lastEncoderPosition = -1;
int menuIndex = 0;

const char* menuOptions[] = {"Opcja1", "Opcja2", "Opcja3", "Opcja4"};
const int menuLength = sizeof(menuOptions) / sizeof(menuOptions[0]);

Arduino_DataBus *bus = create_default_Arduino_DataBus();
Arduino_GFX *gfx = new Arduino_ST7789(bus, GFX_BL, 0 /* rotation */, false /* IPS */);

void IRAM_ATTR updateEncoder() {
  bool data = digitalRead(ENCODER_PIN_B);
  bool clk = digitalRead(ENCODER_PIN_A);
  if (data == clk) {
    encoderPos++;
  } else {
    encoderPos--;
  }
}

void setup() {
  Serial.begin(115200);
  
  if (!gfx->begin()) {
    Serial.println("gfx->begin() failed!");
    while (1);
  }
  gfx->fillScreen(BACKGROUND);
  gfx->setTextColor(TEXT_COLOR);

  pinMode(ENCODER_PIN_A, INPUT_PULLUP);
  pinMode(ENCODER_PIN_B, INPUT_PULLUP);
  pinMode(ENCODER_BUTTON_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B), updateEncoder, CHANGE);

  drawMenu();
}

void loop() {
  int newEncoderPosition = encoderPos / 4;
  if (newEncoderPosition != lastEncoderPosition) {
    lastEncoderPosition = newEncoderPosition;
    menuIndex = (newEncoderPosition % menuLength + menuLength) % menuLength;
    drawMenu();
  }

  if (digitalRead(ENCODER_BUTTON_PIN) == LOW) {
    delay(50); // Debounce delay
    if (digitalRead(ENCODER_BUTTON_PIN) == LOW) {
      handleMenuSelection();
      while (digitalRead(ENCODER_BUTTON_PIN) == LOW); // Wait for button release
    }
  }
}

void drawMenu() {
  gfx->fillScreen(BACKGROUND);
  gfx->setCursor(0, 0);
  for (int i = 0; i < menuLength; i++) {
    if (i == menuIndex) {
      gfx->setTextColor(HIGHLIGHT_COLOR);
    } else {
      gfx->setTextColor(TEXT_COLOR);
    }
    gfx->println(menuOptions[i]);
  }
}

void handleMenuSelection() {
  gfx->fillScreen(BACKGROUND);
  gfx->setTextColor(TEXT_COLOR);
  gfx->setCursor(0, 0);
  gfx->println(menuOptions[menuIndex]);
  gfx->println("Opcje dodatkowe:");
  // Dodaj tutaj kod do wyświetlania dodatkowych opcji dla wybranej opcji menu.
}

void updateEncoder() {
  bool data = digitalRead(ENCODER_PIN_A);
  bool clk = digitalRead(ENCODER_PIN_B);
  if(data == clk){
    encoderPos++;
    digitalWrite(25, HIGH);
  }
  else{
    encoderPos--;
    digitalWrite(25, LOW);
  }
}
