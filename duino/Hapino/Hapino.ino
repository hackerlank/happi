/*

   CHANGELOG:
   2.1 waveDir>0 bug fix
   2.2 added set all command
   3.0 i2c
   TODO: pulse train

   TODO:
   3.0 	migrate to floats (% to 0-1)
  		enable offsets and gains for each motor
 * */

#include <Arduino.h>
#include "DRV2605.h"
#include "I2C.h"

// FIRMWARE
#define FW 			"3.0"

// all commands are <CMD>;<Par1>;<Par2>\r
#define CMD_SETVAL	"SET"	// set PWM value in % "SET;2;50"
#define CMD_SETALL	"SETA"	// set all PWM value [0,1] "SETA;;50"

#define CMD_INFO 	"INFO"	// get firmware info "INFO"
#define CMD_ENABLE 	"EN"	// enable motors "EN;1"
#define CMD_SET_LRA "LRA"  // toggle LRA "LRA;1"

#define CMD_OK 		"OK"	// reply OK
#define CMD_ERROR 	"ERR"	// reply ERROR

#define CMD_SQ 	"SQ"	// set motor sequence "SQ;5;0,2,1,4,5"
#define CMD_SETOFS  "SETOFF" // analogue syntax
#define CMD_SETGAIN "SETGAIN"// analogue syntax

#define WAVE_2P 	"W2P"	// set on-time [s] and amp [0, 1]  "W2P;60;50"
#define WAVE_EN 	"WEN" 	// wave direction +/-1 and 0 to disable "WEN;-1;"


DRV2605 drv;

uint8_t hapAdr[] = {0x0A, 0x1A, 0x2A, 0x4A, 0x5A, 0x6A};

uint8_t seqLen = 6;
uint8_t sequence[] = {0, 1, 2, 3, 4, 5};
float hapOffs[32];
float hapGains[32];


// comm
uint8_t buff[64];
uint8_t res = 0;

// wave state
bool waveRunning = false;
int wavetOff = 1; // ms
int wavetOn = 1;	// ms
int waveDir = 1;
int waveA = 50;	// % PWM

// wave memory
int mId = 0;
int mState = 0;
unsigned long time;
unsigned long time_1, sinceLastOnTrans;

// communication
String inputString = "";         // a string to hold incoming data
String cmd = "";
String val1 = "";
String val2 = "";
boolean stringComplete = false;  // whether the string is complete

inline void fixedDelay(int millis) {
  delay(/*64* */millis);
}

void setPWM(uint8_t m, int val) {
  drv.setPWM(hapAdr[sequence[m]], val * 255 / 100);
}

void allOff() {
  for (int i = 0; i < 6; i++)
    setPWM(i, 0);
}

/* parse values */
String parseCmdSV(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = {0, -1  };
  int maxIndex = data.length() - 1;
  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void setup()  {
  Serial.begin(38400);//,SERIAL_8N1);
  I2c.begin();
  for (uint8_t j = 0; j < 6; j++) {
    uint8_t i=hapAdr[sequence[j]];
    uint8_t bat=drv.readRegister8(i,DRV2605_REG_VBAT);
    /*
    if (bat!=32) {
      drv.init(i, false);
      Serial.print(i,HEX);
      Serial.print(" ");
      Serial.println(bat);
      drv.setPWM(i, 40);
      delay(400);
      drv.setPWM(i, 0);
    }*/
    delay(10);
    
  }
  inputString.reserve(30);
  

}

int applyCmd() {
  cmd =  parseCmdSV(inputString, ';', 0);
  val1 = parseCmdSV(inputString, ';', 1);
  val2 = parseCmdSV(inputString, ';', 2);

  // GENERAL
  if (cmd == CMD_SETVAL) {
    setPWM(val1.toInt(), val2.toInt());
  } else if (cmd == CMD_SETALL) {
    for (int i = 0; i < 6; i++)
      setPWM(i, val2.toInt());
  } else if (cmd == CMD_INFO) {
    Serial.print("FW: ");
    Serial.print(FW);
    Serial.print("  DRV2605l ");
    //Serial.print("  SEQ: ");
    //Serial.print(sequence);
    Serial.print(" * ");
  } else if (cmd == CMD_ENABLE) {
    return -1; // NOT IMPLEMENTED
  } else if (cmd == CMD_SET_LRA) {
    return -1; // NOT IMPLEMENTED
  } else if (cmd == CMD_SQ) {
    seqLen = val1.toInt();
    if (seqLen > 6) return -1;
    for (int i = 0; i < seqLen; i++) {
      sequence[i] = parseCmdSV(val2, ',', i).toInt();
    }
  } else

    // WAVE
    if (cmd == WAVE_2P) {
      wavetOff = 1;
      wavetOn = val1.toInt();
      waveDir = 0;
      waveA = val2.toInt();
    } else if (cmd == WAVE_EN) {
      waveDir = val1.toInt();
      waveRunning = waveDir != 0;
      if (!waveRunning)
        allOff();
    } else
      return -1;
  return 1;
}

void readLine() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    inputString += inChar;
    if (inChar == '\r') {
      stringComplete = true;
      return;
    }
  }
}

void loop()  {
  // process commands
  readLine();
  if (stringComplete) {
    //Serial.println(inputString);
    if (applyCmd() > 0)
      Serial.println(CMD_OK);
    else
      Serial.println(CMD_ERROR);
    inputString = "";
    stringComplete = false;
  }

  // wave mode
  if (waveRunning) {
    time = (millis());
    sinceLastOnTrans = time - time_1;

    // turn off
    if (sinceLastOnTrans > wavetOn && mState == 1) {
      mState = 0;
      setPWM(mId, 0);
      if (waveDir > 0)
        mId < seqLen - 1 ? mId++ : mId = 0;
      else
        mId > 0 		? mId-- : mId = seqLen - 1;
    }
    if ((sinceLastOnTrans > wavetOff + wavetOn) && mState == 0) {
      mState = 1;
      setPWM(mId, waveA);
      time_1 = time;
    }
  }
}



