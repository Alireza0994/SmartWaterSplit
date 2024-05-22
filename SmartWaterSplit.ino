/*
 ===================== program by Alireza Salehi Zadeh ==================
 ================================ 1403 / 3 / 2    =======================
 ================================    5 / 22/ 2024 ==========================
 keyword for finding section : $  dollor sign
 each person record takes 26 bytes | maximum people can fit in uno eeprom : 39
 wiring shcematic can be use in the program direcory
 this version is sipmpler coding style then first version
    so I tried to write less methods then last version
  Copyright 2024 by Alireza Salehi Zadeh
  Thankyou | i am happy to use git

*/

#include <LiquidCrystal_I2C.h> // for lcd
#include <Keypad.h>            // for keypad
#include <EEPROM.h>            // for EEPROM
#include <SoftwareSerial.h>    // for Radio Freaquency IDentification Serial * RFID
#include "RTClib.h"            // for Real Time Clock
int PompTriggerPin = 10;
#define btnStop  A0 //push button
#define btnStart A1 //push button
///////////////////////// Important ! ///////////////////////////////////
#define Master 12944421//Change if you want to replace you master card
long MasterID = Master;

#define RECORD_SIZE 38  // bytes
#define MAX_RECORDS 107  // maximum number of records
int NotFound = 404;
// تعریف پین‌های LCD
LiquidCrystal_I2C lcd(0x27, 16, 2); // Defining The Lcd 16X2 I2C with 0x27 i2c id
//========== Lcd Custome ICONS ========== 
  byte Horiz[8] = {
  0b00000,
  0b01010,
  0b01110,
  0b10001,
  0b00000,
  0b00000,
  0b00000,
  0b00000
  };
  byte horiz2[8] = {
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b10001,
  0b01110,
  0b01010,
  0b00000
  };
  byte DoorOpen[8] {
  0b00000,
  0b01100,
  0b10010,
  0b10010,
  0b11110,
  0b11110,
  0b11110,
  0b00000
  };
  byte DoorClose[8] =
  {
  0b00000,
  0b01100,
  0b10010,
  0b10000,
  0b11110,
  0b11110,
  0b11110,
  0b00000
  };
  byte fan1[8] = {
  0b00000,
  0b00000,
  0b00100,
  0b01110,
  0b11111,
  0b01110,
  0b00100,
  0b00000
};
byte fan2[8] = {

  0b00000,
  0b00000,
  0b10001,
  0b01110,
  0b01110,
  0b01110,
  0b10001,
  0b00000
};
//
//rfid
  const int BUFFER_SIZE = 14; // RFID DATA FRAME FORMAT: 1byte head (value: 2), 10byte data (2byte version + 8byte tag), 2byte checksum, 1byte tail (value: 3)
  const int DATA_SIZE = 10; // 10byte data (2byte version + 8byte tag)
  const int DATA_VERSION_SIZE = 2; // 2byte version (actual meaning of these two bytes may vary)
  const int DATA_TAG_SIZE = 8; // 8byte tag
  const int CHECKSUM_SIZE = 2; // 2byte checksum
  long LastTag;
  unsigned long LastTagTime;
  SoftwareSerial ssrfid = SoftwareSerial(53,52); 

  uint8_t buffer[BUFFER_SIZE]; // used to store an incoming data frame 
  int buffer_index = 0;

//
// keypad
  const byte ROWS = 4;
  const byte COLS = 4;
  char keys[ROWS][COLS] = {
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
  };
  byte rowPins[ROWS] = {9, 8, 7, 6};
  byte colPins[COLS] = {4, 5, 3, 2};
  Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
//
//Real Time Clock
  

  RTC_DS1307 rtc;


// EEPerson
struct EEPerson {
    unsigned long rfid;         // 8 digits
    char phoneNumber[12];       // 12 digits
    int pinCode;                // 4 digits
    int dowLimit;               // 1 digit
    int limitStart;             // 4 digits
    int limitLength;            // 3 digits 
    unsigned long remainingTime;// 6 digits 
};
//flags and variables
  bool canShowMainForm = true;
  EEPerson CurrentPerson;
  bool programMode;
  unsigned long millisMainFormShowed = 0;
  bool PompState = false;
  bool isrunning = false;
  bool AnimationFlag;
  unsigned long millisAnimation = 0;
  DateTime now;
  bool UserStarts;
  unsigned long pastMinutes = 0;
  int lastMinute;
  bool FirstTimeCheck = true;
  int readResult = 0; // save the address of Current Person on EEPROM 
  bool BackupGetted = false;
  unsigned long Timer = 0;
  int QueueTimer = 120;
  bool QueueTimerState = false;
  int lastSecend = 0;
  bool HasTimer = false;
EEPerson getInput(EEPerson UpdatePerson , bool SaveState = false, int Address = -1);
void AddUser(EEPerson person , bool SaveState = false , int Address = -1);
void setup() {
  InitLcd();
  Serial.begin(9600);
  InitPins();
  InitRFID();
  initrtc();
}

void loop() {
  
  LoopRFID();
  //check for Stop The Process by end of Remaining Time or limitlength
  if(UserStarts)
    CheckTime();
  //time checking 
  if(UserStarts && lastMinute != now.minute()){
    if(!FirstTimeCheck){
      pastMinutes++;
      CurrentPerson.remainingTime--;
      if(HasTimer){ 
        Timer--;
        if(Timer == 0){
          HasTimer = false;
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("   Stoping...");
          lcd.setCursor(0,1);
          lcd.print("  Timer Ended");
          delay(700);
          tone(11 , 600 , 500);
          delay(100);
          tone(11 , 500 , 400);
          delay(100);
          tone(11 , 800 , 500);
          lcd.clear();
          lcd.setCursor(0 , 1);
          UserStarts = false;
          HasTimer = false;
          QueueTimer = 120;
          QueueTimerState = true;
          lastSecend = now.second();
          EEPROM.put(readResult , CurrentPerson);
        }
      }
      Serial.println("Adding Minutes");
    }
    else{
      FirstTimeCheck = false;
      Log("First Time Check Changed");
      Log(String(lastMinute));
      Log(String(now.minute()));
    }
    lastMinute = now.minute();
  }
  if(now.second() != lastSecend && QueueTimerState && !UserStarts){
    QueueTimer--;
    if(QueueTimer == 0){
      QueueTimerState = false;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("   Stoping...");
      delay(700);
      tone(11 , 600 , 500);
      delay(100);
      tone(11 , 500 , 400);
      delay(100);
      tone(11 , 800 , 500);
      digitalWrite(PompTriggerPin, HIGH);
      PompState = false;
      QueueTimer = 120;
      
    }
    lastSecend = now.second();
  }
  //check for save in eeprom
  if(pastMinutes % 60 == 0 && pastMinutes != 0 && !BackupGetted){// every 60 minutes we save to eeprom
    Log("Save to eeprom");
    EEPROM.put(readResult, CurrentPerson);
    BackupGetted = true;
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(" -Backup Stored-");
    delay(1000);
  }else{
    if(pastMinutes % 60 != 0)
      BackupGetted = false;
  }
  //btn press checks
  if(BtnCheck(btnStop)){
    Log("btn stop pressed");
    if(UserStarts){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("   Stoping...");
      delay(700);
      tone(11 , 600 , 500);
      delay(100);
      tone(11 , 500 , 400);
      delay(100);
      tone(11 , 800 , 500);
      lcd.clear();
      lcd.setCursor(0 , 1);
      UserStarts = false;
      HasTimer = false;
      QueueTimer = 120;
      QueueTimerState = true;
      lastSecend = now.second();
      EEPROM.put(readResult , CurrentPerson);
      //TODO Check Saf To Stop and Timer 2 minuts
    }
  }
  if(BtnCheck(btnStart)){
    Log("btn start pressed");
  }
  //millis checks
    if(millis() - millisMainFormShowed >= 999){
      if(canShowMainForm){ 
        ShowMainForm();
        millisMainFormShowed = millis();
      }
    }
  //
  //Keypad Button Checks
    char key = keypad.getKey();
    if(key){
      beep();
      if(programMode && key == 'C'){
        if(!PompState){
          digitalWrite(PompTriggerPin, LOW);
          Log("Opening");
          lcd.setCursor(0,1);
          lcd.print("Opening");
          PompState = true;
        }else{
          digitalWrite(PompTriggerPin, HIGH);
          Log("Closing Pomp");
          lcd.setCursor(0,1);
          lcd.print("Closing");
          PompState = false;
        }
        delay(1000);
        lcd.clear();
        
      }
    }
  //
  //Handeling Animations
    lcd.setCursor(7, 0);
    if (PompState == true) {
      if(millis() - millisAnimation >= 500){
        if (AnimationFlag == 1) {
          lcd.write(byte(4));
          AnimationFlag = 0;
        } else {
          lcd.write(byte(5));
          AnimationFlag = 1;
        }
        millisAnimation = millis();
      }
    } else {
      lcd.print(" ");
    }
  }
//RFID Codes
  void InitRFID(){
  ssrfid.begin(9600);
  ssrfid.listen(); 
  Log("RFID INIT DONE");
  }
  void LoopRFID(){
     if (ssrfid.available() > 0){
      bool call_extract_tag = false;

      int ssvalue = ssrfid.read(); // read 
      if (ssvalue == -1) { // no data was read
        return;
      }

      if (ssvalue == 2) { // RDM630/RDM6300 found a tag => tag incoming 
        buffer_index = 0;
      } else if (ssvalue == 3) { // tag has been fully transmitted       
        call_extract_tag = true; // extract tag at the end of the function call
      }

      if (buffer_index >= BUFFER_SIZE) { // checking for a buffer overflow (It's very unlikely that an buffer overflow comes up!)
        Serial.println("Error: Buffer overflow detected!");
        return;
      }

      buffer[buffer_index++] = ssvalue; // everything is alright => copy current value to buffer

      if (call_extract_tag == true) {
        if (buffer_index == BUFFER_SIZE) {
          unsigned tag = extract_tag();
        } else { // something is wrong... start again looking for preamble (value: 2)
          buffer_index = 0;
          return;
        }
      }    
    }    
  }
  unsigned extract_tag() {
      uint8_t msg_head = buffer[0];
      uint8_t *msg_data = buffer + 1; // 10 byte => data contains 2byte version + 8byte tag
      uint8_t *msg_data_version = msg_data;
      uint8_t *msg_data_tag = msg_data + 2;
      uint8_t *msg_checksum = buffer + 11; // 2 byte
      uint8_t msg_tail = buffer[13];

      long tag = hexstr_to_value(msg_data_tag, DATA_TAG_SIZE);
      if(millis() - LastTagTime <= 2000 && tag == LastTag) return;

      long checksum = 0;
      for (int i = 0; i < DATA_SIZE; i+= CHECKSUM_SIZE) {
        long val = hexstr_to_value(msg_data + i, CHECKSUM_SIZE);
        checksum ^= val;
      }
      // Serial.println(millis());
      // Serial.println(LastTagTime);
      // Serial.println("------");
      Log("tag is :");
      Log(String(tag));
      CheckID(tag);
      LastTag = tag;
      LastTagTime = millis();
        // Serial.print("Extracted Tag: ");
        // Serial.println(tag);
        return tag;

  }

  long hexstr_to_value(char *str, unsigned int length) { // converts a hexadecimal value (encoded as ASCII string) to a numeric value
    char* copy = malloc((sizeof(char) * length) + 1); 
    memcpy(copy, str, sizeof(char) * length);
    copy[length] = '\0'; 
    // the variable "copy" is a copy of the parameter "str". "copy" has an additional '\0' element to make sure that "str" is null-terminated.
    long value = strtol(copy, NULL, 16);  // strtol converts a null-terminated string to a long value
    free(copy); // clean up 
    return value;
  }
//EEPROM Codes
  //EEPROM Codes  : Checking ID
    void CheckID(unsigned long ID){
      Log("Checking ID");
      readResult = readPersonById(ID, CurrentPerson);
      Log("result is:");
      Log(String(readResult));
      Log(String(CurrentPerson.rfid));
      if(ID == MasterID){
        programMode = !programMode;
        Log("programmer mode is ");
        Log(String(programMode));
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("program mode ");
        if(programMode)
          lcd.print("on");
        else
          lcd.print("off");
        tone(11 , 500 , 300);
        delay(500);
        lcd.clear();
        lcd.print("Ready For Scan");
      }else{
        if(programMode){
          if (readResult == NotFound) {
            //adding to eeprom
            EEPerson AddingPerson;
            AddingPerson = getInput(AddingPerson);
            if(AddingPerson.rfid == 403){
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print("----Canceled----");
              delay(700);
              lcd.clear();
              beep();
              return;
            }
            AddingPerson.rfid = ID;
            AddUser(AddingPerson);
          }else{
            Log("User Found");
            EEPerson UpdatingPerson;
            UpdatingPerson = CurrentPerson;
            UpdatingPerson = getInput(UpdatingPerson , true , readResult);
            if(UpdatingPerson.rfid == 403){
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print("----Canceled----");
              delay(700);
              lcd.clear();
              return;
            }
            UpdatingPerson.rfid = ID;
            AddUser(UpdatingPerson , true , readResult);
          }
        }else{
          //check for start
          if(readResult != NotFound){
          //   //lcdtype("Enter Pass:");
            Log("Found");
            canShowMainForm = false;
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Enter Password:");
            lcd.setCursor(0,1);
            String InputString = "";
            while(InputString.length() != 4){
              char key = keypad.getKey();
              if(key){
                beep();
                if(key == 'B'){
                  InputString.remove(InputString.length() - 1);
                  lcd.setCursor(InputString.length(), 1);
                  lcd.print(" ");
                  lcd.setCursor(InputString.length(), 1);
                }else{
                  InputString += key;
                  lcd.print("*");
                }
              }
            }
            Log("Password is:");
            Log(InputString);
            Log("Correct password is :");
            Log(String(CurrentPerson.pinCode));
            lcd.clear();
            lcd.setCursor(0,0);
            if(InputString.toInt() == CurrentPerson.pinCode){
              Log("True You can use");
              lcd.print("Password Correct :)");
              lcd.setCursor(0,1);
              lcd.print("welcome ");
              lcd.print(CurrentPerson.rfid);
              delay(700);
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print("Total: |B :back");
              lcd.setCursor(0,1);
              lcd.print(String(CurrentPerson.remainingTime / 60));
              lcd.print(":");
              lcd.print(String(CurrentPerson.remainingTime % 60));
              lcd.print("|D :Timer");
              char key;
              while (true) {
                key = keypad.getKey();
                if(BtnCheck(btnStart)){
                  CheckForStart();
                  return;
                }
                if(key){
                  beep();
                  if(key == 'D' || key == 'B')
                    break;
                }
              }
              Log("Key Getted");
              if(key == 'B'){
                lcd.clear();
                lcd.setCursor(0,0);
                lcd.print("----Canceled----");
                delay(700);
                lcd.clear();
                canShowMainForm = true;
                return;
              }
              if(key == 'D'){
                //timer
                canShowMainForm = false;
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Enter Timer data:");
                lcd.setCursor(0, 1);
                lcd.print("     XXX:XX");
                lcd.setCursor(5, 1);
                lcd.blink();
                InputString = "";
                int i = 0;
                int Hours = 0;
                int Minuts = 0;
                int cx = 5;
                //getting hours
                while (i < 3) {
                    char key = keypad.getKey();
                    if (key && key != '#') {
                      beep();
                      if(key == 'B'){
                        InputString.remove(InputString.length()-1);
                        lcd.setCursor(--cx, 1);
                        lcd.print("X");
                        lcd.setCursor(cx, 1);
                        i--;
                      }else{
                        InputString += key;
                        lcd.print(key);
                        i++;
                        cx++;
                      }
                    }
                }
                Hours = InputString.toInt();
                InputString = "";
                lcd.setCursor(9 , 1);
                i = 0;
                cx = 9;
                //getting minutes
                while (i < 2) {
                    char key = keypad.getKey();
                    if (key && key != '#') {
                      beep();
                      if(key == 'B'){
                        InputString.remove(InputString.length()-1);
                        lcd.setCursor(--cx, 1);
                        lcd.print("X");
                        lcd.setCursor(cx, 1);
                        i--;
                      }else{
                        InputString += key;
                        lcd.print(key);
                        i++;
                        cx++;
                      }
                    }
                }
                Minuts = InputString.toInt();
                InputString = "";
                lcd.noBlink();
                Log(String(Hours));
                Log(":");
                Log(String(Minuts));
                lcd.clear();
                lcd.print("Press Start/Stop");
                lcd.setCursor(0, 1);
                lcd.print("================");
                while(true){
                  if(BtnCheck(btnStart)){
                    Timer = Hours * 60 + Minuts;
                    HasTimer = true;
                    CheckForStart();
                    return;
                  }
                  if(BtnCheck(btnStop)){
                    lcd.clear();
                    lcd.setCursor(0,0);
                    lcd.print("----Canceled----");
                    delay(700);
                    lcd.clear();
                    canShowMainForm = true;
                    return;
                  }
                }
              }
            }else{
              Log("False You can not use");
              lcd.print("Not Correct :(");
              canShowMainForm = true;
              delay(700);
              lcd.clear();
            }
          //   flags.NormalState = 1;
          //   flags.AddingState = 2;
          }
          //if readresult not 404 start timer or wait for confirm to start the timer
        }
      }
    }
  // EEPROM Codes : Checking for start pomp
    void CheckForStart(){
      int Time = now.hour() * 60 + now.minute();
      unsigned long fullTime = CurrentPerson.limitStart + CurrentPerson.limitLength * 60;
      // int from = CurrentPerson.limitStart;
      // int to = CurrentPerson.limitStart + CurrentPerson.limitLength;
      int fromDay = CurrentPerson.dowLimit;
      int toDay = fromDay + fromDay + (CurrentPerson.limitLength / 24 );
      int fromTime = CurrentPerson.limitStart;
      int toTime = (CurrentPerson.limitStart + (CurrentPerson.limitLength * 60)) - toDay * 24;

      //When the conditions are not ok we quit the method so when all is good blow codes will run
      if(CurrentPerson.remainingTime == 0){
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("   Total is 0");
        lcd.setCursor(0,1);
        lcd.print(" Can not start :(");
        delay(1500);
        lcd.clear();
        canShowMainForm = true;
        return;
      }
      Log("now time is :");
      Log(String(Time));
      Log("limits start is :");
      Log(String(CurrentPerson.limitStart));
      Log("limits end is :");
      Log(String(toTime));
      Log("converted dow is :");
      Log(String(now.dayOfTheWeek() + 1));
      Log("from day is ");
      Log(String(fromDay));
      Log("to day is ");
      Log(String(toDay));
      if(CurrentPerson.dowLimit != 9){
        if(Time >= CurrentPerson.limitStart && Time <= toTime && (now.dayOfTheWeek() + 1) >= fromDay && (now.dayOfTheWeek() + 1) <= toDay){ 
          startProccess();
          return;
        }
        errorStartProccess();
        return;
      }
      else{
        startProccess();
        return;
      }
    }
    int CheckForStop(){ //return 0 => Can resume using of pomp | 1 => Remain Time is = 0 | 2 = limit time reached
      int Time = now.hour() * 60 + now.minute();
      unsigned long fullTime = CurrentPerson.limitStart + CurrentPerson.limitLength * 60;
      // int from = CurrentPerson.limitStart;
      // int to = CurrentPerson.limitStart + CurrentPerson.limitLength;
      int fromDay = CurrentPerson.dowLimit;
      int toDay = fromDay + fromDay + (CurrentPerson.limitLength / 24 );
      int fromTime = CurrentPerson.limitStart;
      int toTime = (CurrentPerson.limitStart + (CurrentPerson.limitLength * 60)) - toDay * 24;

      //When the conditions are not ok we quit the method so when all is good blow codes will run
      if(CurrentPerson.remainingTime == 0){
        return 1;
      }
      if(CurrentPerson.dowLimit != 9){
        if(Time >= CurrentPerson.limitStart && Time <= toTime && (now.dayOfTheWeek() + 1) >= fromDay && (now.dayOfTheWeek() + 1) <= toDay){ 
          return 0;
        }
        return 2;
      }
      else{
        return 0;
      }
    }
    void errorStartProccess(){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("   You Can not use");
      lcd.setCursor(0,1);
      lcd.print("On This Time :(");
      delay(1500);
      lcd.clear();
      canShowMainForm = true;
    }
    void startProccess(){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("   Starting...");
      delay(700);
      tone(11 , 600 , 500);
      delay(100);
      tone(11 , 500 , 400);
      delay(100);
      tone(11 , 800 , 500);
      lcd.clear();
      canShowMainForm = true;
      digitalWrite(PompTriggerPin , LOW);
      lastMinute = now.minute();
      PompState = true;
      UserStarts = true;
      QueueTimerState = false;
    }
  // EEPROM Codes : Start Proccesing of adding or updating a person to eeprom
    EEPerson getInput(EEPerson UpdatePerson , bool SaveState = false, int Address = -1) { //Save state : false = adding / true = updating | Address -1 = adding / != 1 = Updating EEprom Address
    EEPerson person;
    if(SaveState)
      person = UpdatePerson;
    lcd.clear();
    lcd.println("     Please");
    lcd.print("   Insert Card");
    // Get RFID (Test value)
    person.rfid = 12345678;
    // Get PhoneNumber
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enter Phone:");
    if(SaveState){
      lcd.setCursor(0, 1);
      lcd.print(person.phoneNumber);
      lcd.blink();
    }
    lcd.setCursor(0, 1);
    char phoneStr[12];
    int i = 0;
    while (i < 11) {
        char key = keypad.getKey();
        if (key) {
          beep();
          if(key == 'B'){
            phoneStr[i] = ' ';
            i--;
            lcd.setCursor(i, 1);
            lcd.print(" ");
          }else{
            lcd.setCursor(i, 1);
            phoneStr[i++] = key;
            lcd.print(key);
          }
        }
    }
    phoneStr[11] = '\0';
    strcpy(person.phoneNumber , phoneStr);
    Log("phoneNumber Code");
    Log(String(person.phoneNumber));
    // Get pinCode
    lcd.clear();
    lcd.noBlink();
    lcd.setCursor(0, 0);
    lcd.print("Enter Pin:");
    if(SaveState){
      lcd.setCursor(0, 1);
      lcd.print(person.pinCode);
    }
    char pinStr[5];
    i = 0;
    while (i < 4) {
        char key = keypad.getKey();
        if (key) {
          beep();
          if(key == 'B'){
            pinStr[i] = ' ';
            i--;
            lcd.setCursor(i, 1);
            lcd.print(" ");
          }else{
            lcd.setCursor(i, 1);
            pinStr[i++] = key;
            lcd.print("*");
          }
        }
    }
    pinStr[4] = '\0';
    person.pinCode = atoi(pinStr);
    Log("pincode is ");
    Log(String(person.pinCode));
    // Get dowLimit
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enter dowLimit:");
    if(SaveState){
      lcd.setCursor(0, 1);
      dowChecker(person.dowLimit);
    }
    char dowKey = '\0';
    while (dowKey == '\0') {
        dowKey = keypad.getKey();
    }
    beep();
    lcd.clear();
    lcd.setCursor(0, 0);
    //Showing Day of Week
    String strDow = String(dowKey);
    dowChecker(strDow.toInt());
    delay(2000);
    person.dowLimit = dowKey - '0';
    unsigned long Hour = 0;  // use for limit and remaining both
    unsigned long Minute = 0;// use for limit and remaining both
    String InputString = "";// use for all getting data from now on
    int cx = 2; //cursor x
    // Get limitStart and limitLength
    if (person.dowLimit != 9) {
        lcd.clear();
        lcd.setCursor(0, 0);
        if(SaveState){
          String msg = "T=" + String(person.limitStart / 60) + ":" + String(person.limitStart % 60) + " | L:" + String(person.limitLength);
          lcd.print(msg);
        }
        else
          lcd.print("T=XX:XX | L:XXX");
        lcd.setCursor(2, 0);
        lcd.blink(); // Start blinking
        i = 0;
        //getting hours
        while (i < 2) {
            char key = keypad.getKey();
            if (key && key != '#') {
              beep();
              if(key == 'B'){
                InputString.remove(InputString.length()-1);
                lcd.setCursor(--cx, 0);
                lcd.print("X");
                lcd.setCursor(cx, 0);
                i--;
              }else{
                InputString += key;
                lcd.print(key);
                i++;
                cx++;
              }
            }
        }
        Hour = InputString.toInt();
        InputString = "";
        lcd.setCursor(5 , 0);
        i = 0;
        cx = 5;
        //getting minutes
        while (i < 2) {
            char key = keypad.getKey();
            if (key && key != '#') {
              beep();
              if(key == 'B'){
                InputString.remove(InputString.length()-1);
                lcd.setCursor(--cx, 0);
                lcd.print("X");
                lcd.setCursor(cx, 0);
                i--;
              }else{
                InputString += key;
                lcd.print(key);
                i++;
                cx++;
              }
            }
        }
        Minute = InputString.toInt();
        InputString = "";
        person.limitStart = Hour * 60 + Minute;
        lcd.setCursor(12, 0); // Move cursor to 'X' for limit length
        i = 0;
        cx = 12;
        //getting length
        while (i < 3) {
            char key = keypad.getKey();
            if (key) {
              beep();
              if(key == 'B'){
                InputString.remove(InputString.length()-1);
                lcd.setCursor(--cx, 0);
                lcd.print("X");
                lcd.setCursor(cx, 0);
                i--;
              }else{
                InputString += key; 
                lcd.print(key);
                i++;
                cx++;
              }
              
            }
        }
        person.limitLength = InputString.toInt();
        
    } else {
        person.limitStart = 0; // No limit
    }
    lcd.noBlink();
    lcd.clear();
    if(SaveState)
    {
      String msg = "H:" + String(person.remainingTime / 60) + " | M:" + String(person.remainingTime % 60);
      lcd.print(msg);
    }
    else
      lcd.print("H:XXXX | M:XX"); // Important cursor locations : 2 , 11
    lcd.setCursor(2,0);
    lcd.blink();
    i = 0;
    cx = 2;
    Hour = 0;
    Minute = 0;
    InputString = "";
    Serial.println("Getting Remaining Time");
    while(i < 4){
      char key = keypad.getKey();
      if(key){
        beep();
        if(key == 'B'){
          InputString.remove(InputString.length()-1);
          lcd.setCursor(--cx, 0);
          lcd.print("X");
          lcd.setCursor(cx, 0);
          i--;
        }else{
          InputString += key;
          lcd.print(key);
          i++;
          cx++;
        }
        Log("input string is :");
        Log(InputString);
      }
    }
    Hour = InputString.toInt();
    InputString = "";
    lcd.setCursor(11 , 0);
    i = 0;
    cx = 11;
    while(i < 2){
      char key = keypad.getKey();
      if(key){
        beep();
        if(key == 'B'){
          InputString.remove(InputString.length()-1);
          lcd.setCursor(--cx, 0);
          lcd.print("X");
          lcd.setCursor(cx, 0);
          i--;
        }else{
          InputString += key;
          lcd.print(key);
          i++;
          cx++;
        }
      }
    }
    Minute = InputString.toInt();
    Serial.println(Hour);
    Serial.println(Minute);
    person.remainingTime = (Hour * 60 + Minute);
    Serial.println(person.remainingTime);
    lcd.noBlink(); // Stop blinking
    lcd.clear();
    lcd.print("Confirm :Press D");
    lcd.setCursor(0,1);
    lcd.print("Cancel  :Press C");
    // Wait for confirmation (pressing D)
    char Key = '\0';
    bool pass = false;
    while (!pass) {
        Key = keypad.getKey();
        if(Key == 'C'){
          pass = true;
          EEPerson emptyPerson;
          emptyPerson.rfid = 403;//forbiden code | have not access to register data: canceled 
          return emptyPerson;
        }
        else if(Key == 'D'){
          pass = true;
        }
    }
    beep();

    return person;
    }
  // EEPROM Codes : Getting a number and writing the day of week
    void dowChecker(int dowKey){
      Serial.println(dowKey);
      switch (dowKey) {
            case 0:
                lcd.print("shanbe");
                break;
            case 1:
                lcd.print("yekshanbe");
                break;
            case 2:
                lcd.print("doshanbe");
                break;
            case 3:
                lcd.print("seshanbe");
                break;
            case 4:
                lcd.print("charshanbe");
                break;
            case 5:
                lcd.print("panjshanbe");
                break;
            case 6:
                lcd.print("jomee");
                break;
            case 9:
                lcd.print("No Limit");
                break;
            default:
                lcd.print("Invalid Input");
                break;
        }
        }
  //EEPROM Codes  : Actual EEprom Codes for adding updating finding free slot or person from eeprom
  
    void writePerson(const EEPerson& person) {
      for (int i = 0; i < EEPROM.length(); i += RECORD_SIZE) {
        EEPerson temp;
        EEPROM.get(i, temp);
        if (temp.rfid == 0) {
          EEPROM.put(i, person);
          return;
        }
      }
    }


    int readPersonById(unsigned long rfid, EEPerson& person) {  // returns address of found person and fills [person] vaiable
      for (int i = 0; i < EEPROM.length(); i += RECORD_SIZE) {
        EEPROM.get(i, person);
        if (person.rfid == rfid) {
          //person.EEPROM_Address = i;
          return i;  // Found the person => return Address of the person
        }
      }
      return NotFound;  // Person not found
    }

    int get_free_slot(EEPerson& person){
      for(int i = 0; i < EEPROM.length(); i += RECORD_SIZE){
        EEPROM.get(i, person);
        if(person.rfid == 0){
          Log("Free Slot is :");
          Log(String(i));
          return i;
        }
      }
    }
    //Add User To EEPROM
    void AddUser(EEPerson person , bool SaveState = false , int Address = -1 ){ // false :adding person / true : updating a person | address -1 : add
      Log("Adding Data to eeprom");
      EEPerson p;
      if(!SaveState){
        Address = get_free_slot(p);
      }
      // else{
      //   EEPerson emptyPerson;
      //   CurrentPerson = emptyPerson;
      // }
      EEPROM.put(Address, person);
      lcd.setCursor(0, 1);
      lcd.print("Added Successfully");

    }
//lcd methods section $
  void InitLcd(){
    lcd.begin();

  	// Turn on the blacklight and print a message.
    lcd.backlight();
  	lcd.print("by A.Salehi Zade");
    delay(1000);
    lcd.createChar(0, Horiz);
    lcd.createChar(1, horiz2);
    lcd.createChar(2, DoorOpen);
    lcd.createChar(3, DoorClose);
    lcd.createChar(4, fan1);
    lcd.createChar(5, fan2);
    Introduction();
    lcd.setCursor(0, 1);
    lcd.print("  Smart Split");
    delay(200);
    lcd.setCursor(3, 1);
    lcd.print("          ");
    delay(100);
    lcd.setCursor(0, 1);
    lcd.print("  Smart Split");
    delay(2500);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Initializing...");
  }
  void ShowMainForm(){
    lcd.setCursor(0,1);
    lcd.print("                ");
    lcd.setCursor(8,0);
    lcd.print("        ");
    lcd.setCursor(6,0);
    if(!PompState)
      lcd.write(byte(3));
    else
      lcd.write(byte(2));
    if(PompState){
      if(HasTimer){
        lcd.setCursor(10,0);
        char HourChar[4];
        char MinuteChar[3];
        sprintf(HourChar, "%03d", Timer / 60);
        sprintf(MinuteChar, "%02d", Timer % 60);
        String msg = String(HourChar) + ":" + String(MinuteChar);
        lcd.print(msg);
      }else{
        lcd.setCursor(9,0);
        lcd.print("working");
      }

      if(UserStarts){
        //showing the amount of usage
        char remainingHoursChar[5]; // show the remaining hours
        char remainingMinutesChar[3]; // show the remaining minutes
        char pastHoursChar[4]; // show the hours past 
        char pastMinutesChar[3]; // show the minuts past
        sprintf(remainingHoursChar, "%04d", CurrentPerson.remainingTime / 60);
        sprintf(remainingMinutesChar, "%02d", CurrentPerson.remainingTime % 60);
        sprintf(pastHoursChar, "%03d", pastMinutes / 60);
        sprintf(pastMinutesChar, "%02d", pastMinutes % 60);
        String msg = "T:" + String(remainingHoursChar) + ":" + String(remainingMinutesChar) + "|" + String(pastHoursChar) + ":" + String(pastMinutesChar); 
        lcd.setCursor(0,1);
        lcd.print(msg);
        
      }else{
        //we are in 2 minute time so we have to show timer
        lcd.setCursor(0 , 1);
        char MinuteChar[3];
        char SecendChar[3];
        sprintf(MinuteChar, "%02d", QueueTimer / 60);
        sprintf(SecendChar, "%02d", QueueTimer % 60);
        String msg = "Enter Card " + String(MinuteChar) + ":" + String(SecendChar);
        lcd.print(msg);
      }
    }else{
      lcd.setCursor(11,0);
      lcd.print("ready");
    }
    RefreshRTC();
  }
  void Introduction(){
     for (int i = 0; i < 16; i++) {
      lcd.setCursor(i, 0);
      lcd.write(byte(0));


      delay(50);
    }
  }
  void CheckTime(){
    int stopres = CheckForStop();
    if(stopres == 0)
      return;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("   Stoping...");
    lcd.setCursor(0,1);
    if(stopres == 1)
      lcd.print("  Time Ended");
    else 
      lcd.print(" limit reached");
    delay(700);
    tone(11 , 600 , 500);
    delay(100);
    tone(11 , 500 , 400);
    delay(100);
    tone(11 , 800 , 500);
    lcd.clear();
    lcd.setCursor(0 , 1);
    UserStarts = false;
    HasTimer = false;
    QueueTimer = 120;
    QueueTimerState = true;
    lastSecend = now.second();
    EEPROM.put(readResult , CurrentPerson);
  }
//pins section $
  void InitPins(){
    // pinMode(BuzzerPin, OUTPUT);
    // pinMode(SuccessPin, OUTPUT);
    // pinMode(ErrorPin, OUTPUT);
    // digitalWrite(BuzzerPin, LOW);
    // digitalWrite(SuccessPin, LOW);
    // digitalWrite(ErrorPin, LOW);
    pinMode(PompTriggerPin, OUTPUT);
    pinMode(btnStop, INPUT_PULLUP);
    pinMode(btnStart, INPUT_PULLUP);
    digitalWrite(PompTriggerPin, HIGH);
    Log("Pins OK");
    delay(350);
  }
  bool BtnCheck(uint8_t pin){
    if(digitalRead(pin) == 0){
      beep();
      delay(200);
      return true;
    }
    return false;
  }
//Real Time Clock Codes
  void initrtc(){
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    }
  }

  void RefreshRTC(){
    now = rtc.now();

    lcd.setCursor(0, 0);
    lcd.print("      ");// 1 extra space for clearing lcd
    lcd.setCursor(0, 0);
    lcd.print(now.hour());
    lcd.print(":");
    lcd.print(now.minute());
    
  }
void beep(){
  tone(11, 900, 100);
}
//program debuging section $
  void Log(String logtext){
    Serial.println(logtext);
  }
  void Log(long logtext){
  Serial.println(logtext);
  }




// junk codes for testing or recyling
  //Test of Adding people
  // EEPerson person = getInput(person);
  // Serial.println("RFID: " + String(person.rfid));
  // Serial.println("Pin Code: " + String(person.pinCode));
  // Serial.println("Dow Limit: " + String(person.dowLimit));
  // Serial.println("Limit Start: " + String(person.limitStart));
  // Serial.println("Limit Length: " + String(person.limitLength));
  // Serial.println("Remaining Time: " + String(person.remainingTime));
  // Serial.println("Adding Person");
  // AddUser(person);

  //Test Updating Of a person
  // EEPerson FoundPerson;
  // int result = readPersonById(12345678, FoundPerson);
  // if(result != NotFound){
  //   lcd.setCursor(0, 0);
  //   lcd.print("Person Found Going for Update");
  //   delay(1000);
  //   EEPerson AddingPerson = getInput(FoundPerson , true , result);
  //   AddUser(AddingPerson , true , result);
  // }
  // delay(2000);
  ///////////////////
  //   //Testing Update Feature
  //   EEPerson updPerson;
  //   updPerson.dowLimit = 5;
  //   updPerson.limitLength = 100;
  //   updPerson.limitStart = 1410;
  //   updPerson.pinCode = 2020;
  //   updPerson.remainingTime = 390000;
  //   updPerson.rfid = 12345678;
  //   getInput(updPerson , true , 22);