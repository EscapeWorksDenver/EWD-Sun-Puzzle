//громкость в строке 122

#include <SoftwareSerial.h>
#include <EEPROM.h>
#define BEEP_PIN 7
#define LED_PIN 11
#define BUT_PIN A0
#define ENC_1_PIN A4
#define ENC_2_PIN A5

byte Comb[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
byte RightComb[10] = { 7, 6, 5, 4, 3, 2, 1, 0, 0, 0 };

int lenComb = 10;
bool flag = 0;
int step = 0;
uint8_t vol = 25;
int encoder = 0;
int encoderOld = 1;

unsigned long stepStartTime = 0;
const int stepDelay = 3000;  // 3 seconds delay

bool flagEnc = 0;
bool encA = 0;
bool encB = 0;
bool encAOld = 0;
bool encBOld = 0;
bool dir = 0;
bool dirOld = 0;

SoftwareSerial mySerial(99, 2);
void (*resetFunc)(void) = 0;

void setup() {  ////////////////////////////////////////////////////////////////////////////////////////////////
  Serial.begin(9600);
  mySerial.begin(9600);
  pinMode(BUT_PIN, INPUT_PULLUP);
  pinMode(ENC_1_PIN, INPUT_PULLUP);
  pinMode(ENC_2_PIN, INPUT_PULLUP);
  pinMode(BEEP_PIN, OUTPUT);
  digitalWrite(BEEP_PIN, LOW);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  Serial.println();
  Serial.print("RIGHT COMBINATION = ");

  for (int i = 0; i < 10; i++) {
    RightComb[i] = (char)EEPROM.read(i);  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    Serial.print(RightComb[i]);
    Serial.print("  ");
  }

  if (RightComb[0] == 0) SetNewComb();
  Serial.print("Volume = ");
  Serial.println(vol);
  DF_vol(vol);
  Serial.println("Start...2330");
  DF_play(1);
  beep(200);
}

void loop() {
  if (digitalRead(BUT_PIN) == 0) {
    delay(300);
    if (digitalRead(BUT_PIN) == 0) SetNewComb();
  }
  
  GetEncoder();
  
  if (encoder != encoderOld) {
    Serial.print("Step =  ");
    Serial.print(step);
    Serial.print(";  Dir = ");
    Serial.print(dir);
    Serial.print(";   Comb[Step] = ");
    Serial.print(RightComb[step]);
    Serial.print(";   ENCODER = ");
    Serial.println(encoder);

    if ((step == 0 || step == 2 || step == 4 || step == 6 || step == 8) && dir == 0) encoder = 0;
    if ((step == 1 || step == 3 || step == 5 || step == 7 || step == 9) && dir == 1) encoder = 0;
    encoderOld = encoder;
    dirOld = dir;
    
    // Record the start time of the step
    stepStartTime = millis();
  }
  
  // Check if 3 seconds have passed since the start of the step
  if (millis() - stepStartTime >= stepDelay) {
    //************************* If we change directions, move on to the next step
    if (RightComb[step] == encoder) {
      DF_play(1);
      //!!!!!!!!!!!!!!!!!!!!!!!!!!! after the sound, check that the gap has not been exceeded (2 encoder steps)
      while (encoder >= encoderOld && encoder <= encoderOld + 4) {
        GetEncoder();
      }
      if (dirOld != dir) {
        step++;
        //******************** If the next correct value is RightComb[N]==0, then everything has passed. Victory!
        if (RightComb[step] == 0) WIN();
        encoder = 0;
      } else {
        step = 0;
        encoder = 0;
      }
    }
  }
}

void WIN() {
  Serial.println("<<<<<<<<<<<<<<<<  Win >>>>>>>>>>>>>>>>>> ");
  DF_play(2);

  for (int i = 0; i < 250; i++) {
    analogWrite(LED_PIN, i);
    delay(10);
  }

  for (int j = 0; j < 10; j++) {
    for (int i = 250; i > 50; i--) {
      analogWrite(LED_PIN, i);
      delay(10);
    }

    for (int i = 50; i < 250; i++) {
      analogWrite(LED_PIN, i);
      delay(10);
    }
  }

    for (int i = 250; i >= 0; i--) {
      analogWrite(LED_PIN, i);
      delay(20);
    }

    digitalWrite(LED_PIN, LOW);
  resetFunc();
}

int beep(int del) {
  digitalWrite(BEEP_PIN, HIGH);
  delay(del);
  digitalWrite(BEEP_PIN, LOW);
}

int SetNewComb() {
  beep(800);
  while (digitalRead(BUT_PIN) == 0) delay(100);
  step = 0;
  encoder = 0;
  Serial.println();

  for (int i = 0; i < 10; i++) Comb[i] = 0;
  Serial.println("Enter a new combination ");

  //Spin the disk
  while (digitalRead(BUT_PIN) == 1) {
    GetEncoder();

    if (dirOld != dir) {
      step++;
      beep(100);

      if (step > 0) {
        Comb[step - 2] = encoder - 1;
        encoder = 0;

        for (int i = 0; i < 10; i++) {
          Serial.print(Comb[i]);
          Serial.print(" ");
        }
        Serial.println(" ");
      }
    }

    if (encoder != encoderOld) {

      if (step == 0 && dir == 0) {
        encoder = 0;

        for (int i = 0; i < 10; i++) {
          beep(7);
          delay(7);
        }
      }

      else {
        beep(15);
        Serial.print("Step =  ");
        Serial.print(step);
        Serial.print(";  Dir = ");
        Serial.print(dir);
        Serial.print(";   ENCODER = ");
        Serial.println(encoder);
      }
    }
    //************************* if we change direction, move on to the next step
    encoderOld = encoder;
    dirOld = dir;
  }

  delay(500);
  if (Comb[0] == 0) resetFunc();
  beep(500);
  Serial.println("EEPROM.write...  ");
  // Write to EEPROM...

  for (int i = 0; i < 10; i++) {
    Serial.print(Comb[i]);
    Serial.print(" ");
    delay(100);
    EEPROM.write(i, Comb[i]);
    delay(200);
  }
  Serial.println(" ");
  resetFunc();
}

void GetEncoder() {
  if (analogRead(A4) < 300) encA = 0;
  if (analogRead(A4) > 320) encA = 1;
  if (analogRead(A5) < 300) encB = 0;
  if (analogRead(A5) > 320) encB = 1;

  if (encA == 1 && encB == 1 && encAOld == 0 && encBOld == 1) {
    encoder++;
    dir = 0;
  }

  if (encA == 1 && encB == 1 && encAOld == 1 && encBOld == 0) {
    encoder++;
    dir = 1;
]  }

  encAOld = encA;
  encBOld = encB;
}
/////////////////////////player library
uint8_t buffer[10] = { 0x7E, 0xFF, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEF };

void setBuff(uint8_t _command, uint16_t _option) {
  uint16_t crc = 0x00;
  buffer[0x03] = _command;
  buffer[0x05] = (uint8_t)(_option >> 0x08);
  buffer[0x06] = (uint8_t)_option;

  for (uint8_t i = 0x01; i < 0x07; i++) {
    crc += buffer[i];
  }

  crc = (0xFFFF - crc + 0x01);
  buffer[0x07] = (uint8_t)(crc >> 0x08);
  buffer[0x08] = (uint8_t)crc;
}

void DF_play(uint16_t _option) {
  setBuff(0x03, _option);
  mySerial.write(buffer, sizeof(buffer));
}

void DF_vol(uint16_t _option) {
  setBuff(0x06, _option);
  mySerial.write(buffer, sizeof(buffer));
  delay(200);
}

void DF_stop() {
  setBuff(0x16, 0);
  mySerial.write(buffer, sizeof(buffer));
}
