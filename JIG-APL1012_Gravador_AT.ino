/* INCLUDES =============================================== */
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);  // set the LCD address to 0x27 for a 16 chars and 2 line display (SCL A5, ..A4)
SoftwareSerial mySerial(2, 3);       // RX/TX (2, 3)(3, 2 gravação traseira)

/* DEFINES ================================================ */
#define EEPROM_ADR 5  //Endereço fixo, escolhido aleatoriamente so pra não colocar 0.
#define SINGLE_PRESSED_TIME 600
#define BLINK_ARROW_TIME 500

struct player {
  String flash;
  String menuChar;
  String name;
} p;


typedef enum {
  None,
  slowBlink,
  slowBlinkBoth,
  slowBlinkAlt,
  FastBlink,
  on,
  off,
} ledState_t;


/* PIN VARIABLES ============================================ */

uint8_t BotaoPin = 6;
uint8_t ledRed = 7;
uint8_t ledGreen = 8;

uint8_t menuButton = 10;
uint8_t rightButton = 11;
uint8_t leftButton = 12;

/* GLOBAL VARIABLES ========================================= */
ledState_t ledGreenState = off;
ledState_t ledRedState = off;

uint8_t selectedName = 1;
uint8_t startButton = 0;

uint32_t testTimer = 0;
uint32_t ledSlowTime = 500;
uint32_t ledFastTime = 75;
uint32_t buttonPressedTimer = 0;

bool start_flag = 0;

String a = "", b = "";

/* FUNCTIONS PROTOTYPE ========================================= */
void ledBlink(void);
void error_func(void);
void readEEPROM(void);
void writeEEPROM(void);
void Menu(void);
struct player setPlayer(uint8_t i);

/* SETUP ========================================================= */
void setup(void) {

  pinMode(BotaoPin, INPUT_PULLUP);
  pinMode(menuButton, INPUT_PULLUP);
  pinMode(leftButton, INPUT_PULLUP);
  pinMode(rightButton, INPUT_PULLUP);

  Serial.begin(9600);
  mySerial.begin(9600);

  readEEPROM();

  lcd.init();  // initialize the lcd
  lcd.clear();
  lcd.backlight();

  lcd.setCursor(0, 0);  //column 1 (0), row 2 (1)
  lcd.print("Nome de gravacao:       ");
  lcd.setCursor(0, 1);
  lcd.print(setPlayer(selectedName).name);

  readEEPROM();

  pinMode(ledRed, OUTPUT);
  pinMode(ledGreen, OUTPUT);

  ledGreenState = slowBlinkAlt;
  ledRedState = None;
  b = "";
}

/* LOOP ====================================================== */
void loop(void) {

  if ((millis() - buttonPressedTimer > SINGLE_PRESSED_TIME) && (digitalRead(menuButton) == 0)) {
    buttonPressedTimer = millis();

    ledRedState = on;
    ledGreenState = on;
    Menu();
  }

  startButton = digitalRead(BotaoPin);
  ledBlink();

  if (start_flag == 0) {
    b = "";
    if (startButton == LOW) {
      ledGreenState = slowBlinkAlt;
      ledRedState = None;
      start_flag = 1;
      testTimer = millis();
    }
  }

  else if (start_flag == 1) {
    lcd.setCursor(0, 0);  //column 1 (0), row 2 (1)
    lcd.print("    GRAVANDO    ");
    lcd.setCursor(0, 1);
    lcd.print(setPlayer(selectedName).name);


    a = (setPlayer(selectedName).name);
    a.trim();
    a = String("+NAMA=") + a;
    ledGreenState = FastBlink;
    ledRedState = None;

    mySerial.println(setPlayer(selectedName).flash);
    delay(100);
    mySerial.println("AT+ROLE0");
    delay(100);

    if ((millis() - testTimer) > 4000) {
          Serial.println("a=");
          Serial.println(a);


      start_flag = 0;
      digitalWrite(ledGreen, LOW);
      digitalWrite(ledRed, LOW);

      mySerial.readString();
      mySerial.println("AT+NAMA");
      delay(40);

      b = mySerial.readString();
      b.trim();
      Serial.println("b=");
      Serial.println(b);

      if (b == a) {

        lcd.setCursor(0, 0);  //column 1 (0), row 2 (1)
        lcd.print("   !GRAVACAO!   ");
        lcd.setCursor(0, 1);
        lcd.print("   !COMPLETA!   ");


        digitalWrite(ledGreen, LOW);
        digitalWrite(ledRed, LOW);
        delay(100);
        digitalWrite(ledGreen, HIGH);
        delay(2000);
        digitalWrite(ledGreen, LOW);
        ledGreenState = slowBlinkAlt;
        ledRedState = None;

        lcd.setCursor(0, 0);  //column 1 (0), row 2 (1)
        lcd.print("Nome de gravacao:       ");
        lcd.setCursor(0, 1);
        lcd.print(setPlayer(selectedName).name);
      }

      else {
        error_func();
      }
    }
  }
}



/* FUNCTIONS =============================================================== */
void ledBlink(void) {
  static uint32_t timerLEDGreen = 0;
  static uint32_t timerLEDRed = 0;

  switch (ledGreenState) {
    case off:
      digitalWrite(ledGreen, LOW);
      break;

    case on:
      digitalWrite(ledGreen, HIGH);
      break;

    case slowBlinkAlt:

      if ((millis() - timerLEDGreen) > ledSlowTime) {
        digitalWrite(ledGreen, !digitalRead(ledGreen));
        digitalWrite(ledRed, !digitalRead(ledGreen));

        timerLEDGreen = millis();
      }
      break;

    case slowBlinkBoth:
      if ((millis() - timerLEDGreen) > 200) {
        digitalWrite(ledGreen, !digitalRead(ledGreen));
        digitalWrite(ledRed, digitalRead(ledGreen));
        timerLEDGreen = millis();
      }
      break;

    case FastBlink:
      if ((millis() - timerLEDGreen) > ledFastTime) {
        digitalWrite(ledGreen, !digitalRead(ledGreen));
        digitalWrite(ledRed, !digitalRead(ledGreen));

        timerLEDGreen = millis();
      }
      break;
  }

  switch (ledRedState) {
    case off:
      digitalWrite(ledRed, LOW);
      break;

    case on:
      digitalWrite(ledRed, HIGH);
      break;

    case slowBlink:
      if ((millis() - timerLEDRed) > ledSlowTime) {
        digitalWrite(ledRed, !digitalRead(ledRed));
        timerLEDRed = millis();
      }
      break;

    case FastBlink:
      if ((millis() - timerLEDRed) > ledFastTime) {
        digitalWrite(ledRed, !digitalRead(ledRed));
        timerLEDRed = millis();
      }
      break;

    case None:
      break;
  }
}

void error_func(void) {

  lcd.setCursor(0, 0);  //column 1 (0), row 2 (1)
  lcd.print("     !ERRO!     ");
  lcd.setCursor(0, 1);
  lcd.print("     !ERRO!     ");

  digitalWrite(ledGreen, LOW);
  digitalWrite(ledRed, LOW);
  delay(100);
  digitalWrite(ledRed, HIGH);
  delay(3000);
  digitalWrite(ledRed, LOW);
  ledGreenState = slowBlinkAlt;
  ledRedState = None;

  lcd.setCursor(0, 0);  //column 1 (0), row 2 (1)
  lcd.print("Nome de gravacao:       ");
  lcd.setCursor(0, 1);
  lcd.print(setPlayer(selectedName).name);
}

void Menu(void) {
  uint8_t lastPrinted = 0;
  uint32_t wordBlink = 0;
  bool blinked = 0;

  lcd.setCursor(0, 0);  //column 1 (0), row 2 (1)
  lcd.print("Escolha o nome:       ");

  while (1) {
    ledBlink();

    if ((millis() - buttonPressedTimer > SINGLE_PRESSED_TIME) && (digitalRead(rightButton) == 0)) {
      buttonPressedTimer = millis();
      blinked = 0;
      selectedName++;
    }

    if ((millis() - buttonPressedTimer > SINGLE_PRESSED_TIME) && (digitalRead(leftButton) == 0)) {
      buttonPressedTimer = millis();
      blinked = 0;
      selectedName--;
    }

    if (selectedName > 3) selectedName = 1;
    else if (selectedName < 1) selectedName = 3;

    switch (selectedName) {
      case 1:

        if (millis() - wordBlink > BLINK_ARROW_TIME) {  //Faz a seta piscar
          if (blinked == 0) {
            lcd.setCursor(0, 1);
            lcd.print(">");
            wordBlink = millis();
            blinked = 1;
          } else {
            lcd.setCursor(0, 1);
            lcd.print(" ");
            wordBlink = millis();
            blinked = 0;
          }
        }

        if (lastPrinted != selectedName) {
          lcd.setCursor(1, 1);
          lcd.print(setPlayer(1).menuChar + " ");

          lcd.setCursor(5, 1);
          lcd.print(" " + setPlayer(2).menuChar + "  ");

          lcd.setCursor(10, 1);
          lcd.print(" " + setPlayer(3).menuChar + " ");
          lastPrinted = selectedName;
        }
        break;

      case 2:

        if (millis() - wordBlink > BLINK_ARROW_TIME) {  //Faz a seta piscar
          if (blinked == 0) {
            lcd.setCursor(5, 1);
            lcd.print(">");
            wordBlink = millis();
            blinked = 1;
          } else {
            lcd.setCursor(5, 1);
            lcd.print(" ");
            wordBlink = millis();
            blinked = 0;
          }
        }

        if (lastPrinted != selectedName) {
          lcd.setCursor(6, 1);
          lcd.print(setPlayer(2).menuChar + " ");

          lcd.setCursor(0, 1);
          lcd.print(" " + setPlayer(1).menuChar + " ");

          lcd.setCursor(10, 1);
          lcd.print(" " + setPlayer(3).menuChar + "  ");
          lastPrinted = selectedName;
        }
        break;

      case 3:
        if (millis() - wordBlink > BLINK_ARROW_TIME) {  //Faz a seta piscar
          if (blinked == 0) {
            lcd.setCursor(10, 1);
            lcd.print(">");
            wordBlink = millis();
            blinked = 1;
          } else {
            lcd.setCursor(10, 1);
            lcd.print(" ");
            wordBlink = millis();
            blinked = 0;
          }
        }

        if (lastPrinted != selectedName) {
          lcd.setCursor(11, 1);
          lcd.print(setPlayer(3).menuChar + "  ");

          lcd.setCursor(0, 1);
          lcd.print(" " + setPlayer(1).menuChar + " ");

          lcd.setCursor(5, 1);
          lcd.print(" " + setPlayer(2).menuChar + " ");

          lastPrinted = selectedName;
        }
        break;
    }


    if ((millis() - buttonPressedTimer > SINGLE_PRESSED_TIME) && (digitalRead(menuButton) == 0)) {
      buttonPressedTimer = millis();

      writeEEPROM();

      lcd.setCursor(0, 0);  //column 1 (0), row 2 (1)
      lcd.print("Nome de gravacao:       ");
      lcd.setCursor(0, 1);
      lcd.print(setPlayer(selectedName).name);
      ledGreenState = slowBlinkAlt;
      ledRedState = None;
      return;
    }
  }
}


struct player setPlayer(uint8_t i) {
  switch (i) {
    case 1:
      p = { "AT+NAMAAPL-1012", "STD", "    APL-1012         " };
      return p;
      break;

    case 2:
      p = { "AT+NAMAAPL-1012 FORCE", "FRC", " APL-1012 FORCE      " };
      return p;
      break;

    case 3:
      p = { "AT+NAMAACTIVEBOX BT", "AMP", "  ACTIVEBOX BT  " };
      return p;
      break;
  }


   /* case 3:
      p = { "AT+NAMAAPL-1012 LIGHTS-ON", "LGO", "APL-1012 LIGHTS-ON" };
      return p;
      break;
  }*/
  return;
}


void readEEPROM(void) {
  EEPROM.get(EEPROM_ADR, selectedName);

  if (isnan(selectedName)) {
    selectedName = 1;  //se for NaN, seta pra 1;
    writeEEPROM();
  }

  if ((selectedName <= 0) || (selectedName >= 4)) {
    selectedName = 1;
    writeEEPROM();
  }
}

void writeEEPROM(void) {
  EEPROM.put(EEPROM_ADR, selectedName);
}