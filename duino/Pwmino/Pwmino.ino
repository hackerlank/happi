/*

   CHANGELOG:
   2.1 waveDir>0 bug fix
   2.2 added set all command
   3.0 i2c
   TODO: pulse train
*/
// FIRMWARE
#define FW 			"3.1 Oct16"

// 1 for DRV, 0 for mosfet
#define DRV2603 	0

// wb
#define PIN_ENABLE 	4 		// enable motors
#define PIN_TYPE 	8		// toggle type

// all commands are <CMD>;<Par1>;<Par2>\r
#define CMD_SETVAL	"SET"	// set PWM value in % "SET;2;50"
#define CMD_SETALL	"SETA"	// set all PWM value in % "SETA;;50"

#define CMD_INFO 	"INFO"	// get firmware info "INFO"
#define CMD_ENABLE 	"EN"	// enable motors "EN;1"
#define CMD_SET_LRA "LRA"	// toggle LRA "LRA;1"
#define CMD_OK 		"OK"	// reply OK
#define CMD_ERROR 	"ERR"	// reply ERROR

#define CMD_SQ 		"SQ"	// set motor sequence "SQ;5;0,2,1,4,5"

#define WAVE_2P 	"W2P"	// set on-time [ms] and amp [%] "W2P;<t_on[ms]>;<amp[%]>" ... "W2P;60;50"
#define WAVE_EN 	"WEN" 	// wave direction +/-1 and 0 to disable "WEN;-1"

#define WAVE_PARAMS "WP"	// set all params "WP;<float>CSV" @ deprecated
#define WAVE_F0 	"WF0"
#define WAVE_FK 	"WFK"
#define WAVE_A0 	"WA0"
#define WAVE_AK 	"WAK"


//uint8_t pwmPins[6]={11,9,5,3,6,10};
uint8_t pwmPins	[6]={3,5,6,9,10,11};
uint8_t sequence[6]={3,4,0,5,2,1};
uint8_t seqLen=6;

// comm
uint8_t buff[64];
uint8_t res = 0;

// wave param
int wavetOff = 1; // ms
int wavetOn = 1;	// ms
int waveDir = 1;
int waveA = 50;	// % PWM
unsigned long timeExpire = 0;

// wave memory
bool waveRunning = false;
int mId = 0;
int mState = 0;
unsigned long time_1, sinceLastOnTrans;

// communication
boolean stringComplete = false;  // whether the string is complete
char inpoutStr[100];
int strLen=0;

/*If you change TCCR0B, it affects millis() and delay(). They will count time faster or slower than normal if you
 * change the TCCR0B settings. Below is the adjustment factor to maintain consistent behavior of these functions */
inline void fixedDelay(int millis) {delay(64*millis);}



void setPWM(uint8_t m, int val) {
	if (DRV2603)
		analogWrite(pwmPins[sequence[m]],127+val*128/100);
	else
		analogWrite(pwmPins[sequence[m]],	 val*255/100);
}

void allOff() {
	for (int i=0;i<6;i++)
		setPWM(i,0);
}

/* parse values, assumes first cell contains string, the rest contain ints */
int parseCMD(char * name, int * ints) {
  int count=0;
  char * pch;
  pch = strtok (inpoutStr,",;");
  while (pch != NULL && count < 11)  {
    if (!count)
        strcpy(name, pch);
    else
        ints[count-1]=atoi(pch);
    count++;
    pch = strtok (NULL, ",;");
  }
  return count;
}

/* walk through cmds and execute */
int applyCmd() {
  char cmd[10];
  int ints[10];
  int cmdPlusInts=parseCMD(cmd, ints);
  /* 
  // debug
  Serial.println();
  Serial.println(cmd);
  for (int i=0;i<cmdPlusInts-1;i++) {
    Serial.println(ints[i]);  
  }
  return 1;
  */
  // SIMPLE
  if      (strcmp(cmd,CMD_SETVAL)==0) {
    setPWM(ints[0], ints[1]);
  } 
  else if (strcmp(cmd,CMD_SETALL)==0) {
    for (int i = 0; i < 6; i++)
      setPWM(i, ints[0]);
  } 
  else if (strcmp(cmd,CMD_CLICK)==0) {
    digitalWrite(13,HIGH);
    setPWM(ints[0], ints[1]);
    fixedDelay(ints[2]); 
    setPWM(ints[0], 0);
    digitalWrite(13,LOW);
  }
  else if (strcmp(cmd,CMD_CLICK_ALL)==0) {
    digitalWrite(13,HIGH);
    for (int i = 0; i < 6; i++)
      setPWM(i, ints[0]);
    fixedDelay(ints[1]); 
    allOff();
    digitalWrite(13,LOW);
  }

  // BASICS
  else if (strcmp(cmd,CMD_INFO)==0) {
    Serial.print("FW: ");
    Serial.print(FW);
    Serial.print("  DRV2605l ");
    Serial.print("  SEQ: ");
    for (int i=0;i<seqLen;i++){
      Serial.print(sequence[i]);
      Serial.print(' ');
    }
    Serial.print(" * ");
  } 
  else if (strcmp(cmd,CMD_TEST)==0) {
    for (uint8_t j = 0; j < 6; j++) {
      uint8_t i=hapAdr[sequence[j]];
      setPWM(j,20);
      fixedDelay(100);
      setPWM(j, 0);
      fixedDelay(100);
    }
  } 
  else if (strcmp(cmd,CMD_SCAN)==0) {
    digitalWrite(13,HIGH);
    int n=I2c.scan();
    digitalWrite(13,LOW);
    for (int i=0; i<n;i++) {
      digitalWrite(13,HIGH);
      fixedDelay(300);
      digitalWrite(13,LOW);
      fixedDelay(300);
    }
  } 
  else if (strcmp(cmd,CMD_RESET)==0) {
    initDrivers();
    digitalWrite(13,HIGH);
    fixedDelay(700);
    digitalWrite(13,LOW);
  } 
  else if (strcmp(cmd,CMD_ENABLE)==0) {
    return -1; // NOT IMPLEMENTED
  }   
  else if (strcmp(cmd,CMD_SET_LRA)==0) {
    return -1; // NOT IMPLEMENTED
  } 

  // WAVE
  else if (strcmp(cmd,CMD_SQ)==0) {
    if (ints[0] > (cmdPlusInts-2)) 
      return -1;
    seqLen = ints[0];
    for (int i = 0; i < seqLen; i++) {
      sequence[i] = ints[i+1];
    }
  }   
  else if (strcmp(cmd,WAVE_2P)==0) {
      wavetOff = 1;
      wavetOn = ints[0];
      waveDir = 0;
      waveA = ints[1];
  } 
  else if (strcmp(cmd,WAVE_EN)==0) {
      waveDir = ints[0];
      if (waveDir!=0 && !waveRunning)
        mId=random(seqLen);
      if (waveDir>1 || waveDir<-1)
        timeExpire=millis()/64+(unsigned long)(waveDir>0 ? waveDir : -waveDir);
      else timeExpire=0;
      waveRunning = waveDir != 0;
      if (!waveRunning)
        allOff();
  } else if (strcmp(cmd,WAVE)==0) {
      //WAV;Ton;Amplitude;Direction;Duration
      wavetOn =   ints[0];
      waveA =     ints[1];
      waveDir =   ints[2];
      if (waveDir!=0 && !waveRunning)
          mId=random(seqLen);
      timeExpire=ints[3] ? millis()/64+(unsigned long)ints[3] : 0;
      waveRunning = waveDir != 0;
      if (!waveRunning)
        allOff();
  } else
      return -1;
  return 1;
}

/* read all chars and raise flag if command complete (\r found) */
void readLine() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    inpoutStr[strLen] = inChar;
    strLen++;
    if (inChar == '\r') {
      stringComplete = true;
      return;
    }
  }
}

void initDrivers() {
  for (uint8_t j = 0; j < 6; j++) {
    uint8_t i=hapAdr[sequence[j]];
    drv.setAddress(i);
    drv.init(i,false);
    drv.setRealtimeValue(0);
    fixedDelay(10);
  }
}

void setup()  {
	// set PWM timer resolution highest
	TCCR0B = TCCR0B & 0b11111000 | 0x01;
	TCCR1B = TCCR1B & 0b11111000 | 0x01;
	TCCR2B = TCCR2B & 0b11111000 | 0x01;
	// config PWM outputs
	for (int i=0;i<6;i++) {
		pinMode(pwmPins[i], OUTPUT);
		setPWM(pwmPins[i], 0);
	}
	if (DRV2603) {
		// init enable pin
		pinMode(PIN_ENABLE, OUTPUT);
		digitalWrite(PIN_ENABLE, LOW);
		// init driver type to ERM
		pinMode(PIN_TYPE, OUTPUT);
		digitalWrite(PIN_TYPE, LOW);
	}
		inputString.reserve(30);
		Serial.begin(38400);//,SERIAL_8N1);

}
}

/* reads serial buffer, executes command if complete, controls the wave */
void loop()  {
  // read all chars in the buffer
  readLine();

  // execute command if complete and reset buffer
  if (stringComplete) {
    //Serial.println(inputString);
    if (applyCmd() > 0)
      Serial.println(CMD_OK);
    else
      Serial.println(CMD_ERROR);
    
    memset(inpoutStr, 0, strLen);
    strLen=0;
    stringComplete = false;
  }

  // wave mode
  if (waveRunning) {
    unsigned long t = millis()/64;
    if (timeExpire > 0)
      if (long(timeExpire-t) < 0){
        waveRunning=false;
        allOff();
        return;
    }  
    sinceLastOnTrans = t - time_1;
    // turn off
    if (sinceLastOnTrans > wavetOn && mState == 1) {
      mState = 0;
      setPWM(mId, 0);
      if (waveDir > 0)
        mId < seqLen - 1 ? mId++ : mId = 0;
      else
        mId > 0 		     ? mId-- : mId = seqLen - 1;
    }
    if ((sinceLastOnTrans > wavetOff + wavetOn) && mState == 0) {
      mState = 1;
      setPWM(mId, waveA);
      time_1 = t;
    }
  }
}




