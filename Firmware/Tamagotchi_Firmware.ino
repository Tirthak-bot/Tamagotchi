#include <Wire.h>
#include <U8g2lib.h>

// OLED setup (SSD1306, I2C, full buffer)
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/7, /* data=*/6);

#define BTN1 9
#define BTN2 8
#define BTN3 20
#define BTN4 21
#define BTN5 5
#define BTN6 4

#define SDA_PIN 6
#define SCL_PIN 7
#define BUZZER 10

#define BTN_UP BTN1
#define BTN_LEFT BTN2
#define BTN_DOWN BTN3
#define BTN_RIGHT BTN4
#define BTN_TOP BTN5
#define BTN_BOTTOM BTN6
#define BUZZER_PIN BUZZER

struct Pet {
  int hunger;
  int happiness;
  int energy;
  unsigned long age;
};
Pet pet;

enum Screen {
  SCREEN_MAIN,
  SCREEN_FEED,
  SCREEN_PLAY,
  SCREEN_SLEEP
};
Screen currentScreen = SCREEN_MAIN;

unsigned long lastUpdate = 0;
unsigned long lastButtonPress = 0;

// --- Bitmaps/emojis (16x16) ---
const unsigned char petHappy[] PROGMEM = {
  0x00,0x00,0x1F,0xF8,0x20,0x04,0x40,0x02,
  0x4C,0x32,0x4C,0x32,0x40,0x02,0x40,0x02,
  0x44,0x22,0x43,0xC2,0x40,0x02,0x20,0x04,
  0x1F,0xF8,0x00,0x00,0x00,0x00,0x00,0x00
};
const unsigned char petSad[] PROGMEM = {
  0x00,0x00,0x1F,0xF8,0x20,0x04,0x40,0x02,
  0x4C,0x32,0x4C,0x32,0x40,0x02,0x40,0x02,
  0x40,0x02,0x43,0xC2,0x44,0x22,0x20,0x04,
  0x1F,0xF8,0x00,0x00,0x00,0x00,0x00,0x00
};
const unsigned char petNeutral[] PROGMEM = {
  0x00,0x00,0x1F,0xF8,0x20,0x04,0x40,0x02,
  0x4C,0x32,0x4C,0x32,0x40,0x02,0x40,0x02,
  0x47,0xE2,0x40,0x02,0x40,0x02,0x20,0x04,
  0x1F,0xF8,0x00,0x00,0x00,0x00,0x00,0x00
};
const unsigned char petSleep[] PROGMEM = {
  0x00,0x00,0x1F,0xF8,0x20,0x04,0x40,0x02,
  0x4F,0x72,0x40,0x02,0x40,0x02,0x40,0x02,
  0x43,0xC2,0x40,0x02,0x40,0x02,0x20,0x04,
  0x1F,0xF8,0x00,0x00,0x00,0x00,0x00,0x00
};

void setup() {
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_TOP, INPUT_PULLUP);
  pinMode(BTN_BOTTOM, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);

  u8g2.begin();

  pet.hunger = 80;
  pet.happiness = 80;
  pet.energy = 80;
  pet.age = 0;
}

void updatePet() {
  if (millis() - lastUpdate > 5000) {
    pet.hunger = max(0, pet.hunger - 1);
    pet.happiness = max(0, pet.happiness - 1);
    pet.energy = max(0, pet.energy - 1);
    pet.age += 5;
    lastUpdate = millis();
  }
}

void checkButtons() {
  if (millis() - lastButtonPress < 200) return;

  if (digitalRead(BTN_LEFT) == LOW) { currentScreen = SCREEN_FEED; tone(BUZZER_PIN, 1000, 50); lastButtonPress = millis(); }
  else if (digitalRead(BTN_UP) == LOW) { currentScreen = SCREEN_PLAY; tone(BUZZER_PIN, 1200, 50); lastButtonPress = millis(); }
  else if (digitalRead(BTN_RIGHT) == LOW) { currentScreen = SCREEN_SLEEP; tone(BUZZER_PIN, 800, 50); lastButtonPress = millis(); }
  else if (digitalRead(BTN_DOWN) == LOW) { currentScreen = SCREEN_FEED; tone(BUZZER_PIN, 1200, 50); lastButtonPress = millis(); }
  else if (digitalRead(BTN_TOP) == LOW) { currentScreen = SCREEN_PLAY; tone(BUZZER_PIN, 1400, 50); lastButtonPress = millis(); }
  else if (digitalRead(BTN_BOTTOM) == LOW) { currentScreen = SCREEN_SLEEP; tone(BUZZER_PIN, 1000, 50); lastButtonPress = millis(); }
}

void handleScreenLogic() {
  switch(currentScreen) {
    case SCREEN_FEED: pet.hunger = min(100, pet.hunger + 10); currentScreen = SCREEN_MAIN; break;
    case SCREEN_PLAY: pet.happiness = min(100, pet.happiness + 10); pet.energy = max(0, pet.energy - 5); currentScreen = SCREEN_MAIN; break;
    case SCREEN_SLEEP: pet.energy = min(100, pet.energy + 15); currentScreen = SCREEN_MAIN; break;
    case SCREEN_MAIN: break;
  }
}

void drawBar(int x, int y, int value) {
  int barWidth = 100;
  int barHeight = 6;
  int fillWidth = map(value, 0, 100, 0, barWidth);
  u8g2.drawFrame(x, y, barWidth, barHeight);
  u8g2.drawBox(x, y, fillWidth, barHeight);
}

void render() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);

  const unsigned char* sprite;
  if (pet.energy < 30 || pet.hunger < 30 || pet.happiness < 30) sprite = petSad;
  else if (pet.hunger > 50 && pet.happiness > 50 && pet.energy > 50) sprite = petHappy;
  else sprite = petNeutral;

  u8g2.drawXBMP(56, 2, 16, 16, sprite);

  u8g2.setCursor(0, 24); u8g2.print("HUN "); drawBar(30, 20, pet.hunger);
  u8g2.setCursor(0, 36); u8g2.print("HAP "); drawBar(30, 32, pet.happiness);
  u8g2.setCursor(0, 48); u8g2.print("ENG "); drawBar(30, 44, pet.energy);

  u8g2.setCursor(0, 60); u8g2.print("[Feed] [Play] [Sleep]");

  u8g2.sendBuffer();
}

void loop() {
  checkButtons();
  updatePet();
  handleScreenLogic();
  render();
  delay(100);
}
