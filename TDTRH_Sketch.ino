#include <LiquidCrystal.h>
#include <RTClib.h> //by Adafruit library
#include <EEPROM.h>
#include <DHT.h> //by Adafruit library

#define DHTTYPE DHT11
#define DHTPIN 8
DHT dht(DHTPIN, DHTTYPE);
int rH;
float temp;
 
#define RS 2
#define EN 3
#define D4 4
#define D5 5
#define D6 6
#define D7 7
LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);
#define btnSetTime A0 //A1
#define btnSetAlarm A1 //A3
#define btnDec A2 //A0
#define btnInc A3 //A2
#define buzzer 9

RTC_DS3231 rtc;
DateTime now;
DateTime futureRemainder;

String daysOfTheWeek0[] = {"S", "M", "T", "W", "T", "F", "S"};
String weekNum[] = {"1", "2", "3", "4", "5", "6", "7"};
String daysOfTheWeek1[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
String monthChar[] = {"", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
int sec;
int min;
int hr;
int dow;
int day;
int month;
int year;
String dayOfWeek;
String time;
String date;
String monthStr;
float tempC_RTC;
String tempUnit;
byte degreeCSymbol[8] = {B10000, B00000, B00111, B01000, B01000, B01000, B00111, B00000};
byte tempSymbol[8] = {B00100, B01010, B01011, B01110, B01111, B01110, B11111, B000000};
byte humiditySymbol[8] = {B00100, B01010, B10001, B10001, B11111, B11111, B01110, B00000};
byte alarmOnSymbol[8] = {B00100, B01110, B01110, B01110, B11111, B11111, B00100, B00000};
byte alarmOffSymbol[8] = {B00100, B01110, B01100, B01010, B10111, B01111, B00100, B00000};
int mode = 0, flag = 0, setTime = 0, setAlarm = 0;
int aHr, aMin;
boolean isAlarmOn = false;
boolean isRepeatAlarmOn = false;
int alarmStatus = 0;
int repeatAlarmStatus = 0;
boolean isRepeatAlarmOff = false;
int rDays, rHr, rMin, rSec;
boolean isRemainderOn = false;
int remainderStatus = 0;

void setup() {
  pinMode(btnSetTime, INPUT_PULLUP);
  pinMode(btnInc, INPUT_PULLUP);
  pinMode(btnDec, INPUT_PULLUP);
  pinMode(btnSetAlarm, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);
  
  dht.begin();
  lcd.begin(16, 2);
  lcd.createChar(0, degreeCSymbol);
  lcd.createChar(1, tempSymbol);
  lcd.createChar(2, humiditySymbol);
  lcd.createChar(3, alarmOnSymbol);
  lcd.createChar(4, alarmOffSymbol);
  delay(100);
  rtcStatus();
  String str0 = "REAL TIME CLOCK WITH ALARM, DATE, TEMP AND RH.";
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(str0);
  delay(500);
  for (int i = 0; i < str0.length(); ++i) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(str0.substring(i, 16 + i));
    String str1 = "by Ashok Gandham";
    lcd.setCursor(0, 1);
    lcd.print(str1);
    delay(150);
  }
  lcd.clear();
  pinMode(LED_BUILTIN, OUTPUT);

  readAlarmFromEEPROM();
  if (alarmStatus == 1) {
    isAlarmOn = true;
  } else {
    isAlarmOn = false;
  }
  if (repeatAlarmStatus == 1) {
    isRepeatAlarmOn = true;
  } else {
    isRepeatAlarmOn = false;
  }
  readRemainderFromEEPROM();
  if (remainderStatus == 1) {
    isRemainderOn = true;
  } else {
    isRemainderOn = false;
  }
}

//LOOPING CONTINOUSLY REQUIRED VALUES ON LCD
void loop() {
  rtcStatus();
  displayTimeDateTempRh();
}

//CHECK RTC INITIALIZATION AND ITS POWER STATUS
void rtcStatus() {
  if(!rtc.begin()) {
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("RTC not found!");
    while(1);
  }
  if(rtc.lostPower()) {
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("RTC lost power.");
    lcd.setCursor(1, 1);
    lcd.print("Set the time!");
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //GET REAL TIME AND DATE WHILE COMPILING or SET AS SHOWN BELOW
    rtc.adjust(DateTime(2023, 21, 10, 10, 0, 0)); //year, month, day, hours, minutes, seconds
  }
  rtc.disable32K();
}

//DS3231 HELPS TO GET TEMPERATURE ALSO WITHOUT REQUIRING EXTERNAL DEVICE LIKE DHT11
void tempFromRTC() {
  lcd.setCursor(0, 1);
  lcd.write(byte(1));
  tempC_RTC = rtc.getTemperature();
  lcd.setCursor(2, 1);
  lcd.print(tempC_RTC);
  lcd.setCursor(6, 1);
  tempUnit =  "\xDF""C";
  lcd.print(tempUnit);
  lcd.setCursor(12, 1);
  lcd.print(year);
}

//GATHER AND GET ALL LCD VALUES AT ONE METHOD
void displayTimeDateTempRh() {
  now = rtc.now();
  lcd.clear();
  if (setTime == 0) {
    sec = now.second();
    min = now.minute();
    hr = now.hour();
    dayOfWeek = daysOfTheWeek0[now.dayOfTheWeek()];
    day = now.day();
    month = now.month();
    year = now.year();
    monthStr = monthChar[now.month()];
  } 
  if (mode == 0) {
    setupTime();
    getTime();
    getDHTValues(); //TEMPERATURE AND HUMIDITY FROM DHT11
    //tempFromRTC(); //TEMPERATURE FROM DS3231 RTC
  } else if (mode == 1) {
    setupAlarm();
  } else {
    setupRemainder();
  }
  delay(150);
  cursorPosition();
  delay(150);
}

//SET TIME AND DATE. NO NEED TO SET WEEK-DAY AS IT AUTOMATICALLY CALCULATE WEEK-DAY FROM ADJUSTED TIME AND DATE.
void setupTime() {
  if (setTime == 7 || (setTime > 0 && digitalRead(btnSetAlarm) == 0)) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Time & Date Set");
    delay(1000);
    rtc.adjust(DateTime(year, month, day, hr, min, sec));
    setTime = 0;
    mode = 0;
  }

  if (digitalRead(btnSetTime) == 0 && flag == 0) {
    flag = 1;
    setTime++;
  }

  if (digitalRead(btnSetTime) == 1) {
    flag = 0;
  }

  if (setTime == 0 && digitalRead(btnSetAlarm) == 0 && flag == 0) {
    mode = 1;
    flag = 1;
  }
  if (setTime == 0 && digitalRead(btnSetAlarm) == 1) {
    flag = 0;
  }

  if (digitalRead(btnInc) == 0) {
    if (setTime == 1) hr++;
    if (setTime == 2) min++;
    if (setTime == 3) sec++;
    if (setTime == 4) month++;
    if (setTime == 5) day++;
    if (setTime == 6) year++;

    if (hr > 23) hr = 0;
    if (min > 59) min = 0;
    if (sec > 59) sec = 0;
    if (month > 12) month = 1;
    if (day > 31) day = 1;
    if (year > 2099) year = 2000;
    monthStr = monthChar[month];
  }

  if (digitalRead(btnDec) == 0) {
    if (setTime == 1) hr--;
    if (setTime == 2) min--;
    if (setTime == 3) sec--;
    if (setTime == 4) month--;
    if (setTime == 5) day--;
    if (setTime == 6) year--;

    if (hr < 0) hr = 23;
    if (min < 0) min = 59;
    if (sec < 0) sec = 59;
    if (month < 1) month = 12;
    if (day < 1) day = 31;
    if (year < 2000) year = 2099;
    monthStr = monthChar[month];
  }
}

//SHOW TIME AND OTHER DATA ON LCD
void getTime() {
  lcd.clear();
  lcd.setCursor(0, 0);
  if(hr < 10) {
    lcd.print("0");
  }
  lcd.print(hr, DEC);
  lcd.setCursor(2, 0);
  lcd.print(":");
  if(min < 10) {
    lcd.print("0");
  }
  lcd.print(min, DEC);
  lcd.setCursor(5, 0);
  lcd.print(":");
  if(sec < 10) {
    lcd.print("0");
  }
  lcd.print(sec, DEC);

  lcd.setCursor(9, 0);
  lcd.print(dayOfWeek);
  lcd.setCursor(9, 1);
  if (isAlarmOn || isRepeatAlarmOn) {
    if (hr == aHr && min == aMin) {
      digitalWrite(buzzer, HIGH);
      delay(250);
      lcd.write(byte(3));
      digitalWrite(buzzer, LOW);
      delay(250);
    } else {
      lcd.write(byte(3));
    }
  } else {
    lcd.print(weekNum[now.dayOfTheWeek()]);
  }

  if (setTime < 6) {
    if (sec != 0) { //SHOWS YEAR AT 0 SECOND
      lcd.setCursor(11, 0);
      lcd.print(monthStr);
      lcd.setCursor(14, 0);
      if(day < 10) {
        lcd.print("0");
      }
      lcd.print(day, DEC);
    } else {
      lcd.setCursor(12, 0);
      lcd.print(year);
    }
  } else {
    lcd.setCursor(12, 0);
    lcd.print(year);
  }

  //TURN OFF ALARM OR REPEATED ALARM BY PRESSING INC/DEC BUTTON
  if (hr == aHr && min == aMin) {
    if (isAlarmOn || isRepeatAlarmOn) {
      if (digitalRead(btnInc) == 0 || digitalRead(btnDec) == 0) {
        mode == 0;
        setAlarm = 0;
        alarmOFF();
        if (isRepeatAlarmOn) {
          isRepeatAlarmOff = true;
          isRepeatAlarmOn = false;
        }
        return;
      } else if (sec >= 55) {
        alarmOFF();
      } else {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(250);
        digitalWrite(LED_BUILTIN, LOW);
        delay(250);
      }
    }
    if (isRepeatAlarmOff) {
      lcd.setCursor(9, 1);
      lcd.write(byte(3));
      if (sec == 59) {
        delay(1000);
        isRepeatAlarmOn = true;
        isRepeatAlarmOff = false;
        writeAlarmToEEPROM(aHr, aMin, alarmStatus, 1);
      }
    }
  }

  //TURN OFF REMAINDER BY PRESS & HOLD INC/DEC BUTTON OR LEAVE WITHOUT PRESSING THESE BUTTONS TO REPEAT REMAINDER
  if (year == futureRemainder.year() && month == futureRemainder.month() && day == futureRemainder.day() 
      && hr == futureRemainder.hour() && min == futureRemainder.minute()) {
    if (isRemainderOn) {
      if (digitalRead(btnInc) == 0 || digitalRead(btnDec) == 0) {
        mode == 0;
        isRemainderOn = false;
        writeRemainderToEEPROM(0, 0, 0, 0, 0); //REMOVE REMINDER FROM EEPROM
      } else {
        showRemainderTime("Remainder Alarm ");
        digitalWrite(buzzer, HIGH);
        delay(1000);
        digitalWrite(buzzer, LOW);
        delay(1000);
      }
      
    }
  }
}
void alarmOFF() {
  digitalWrite(buzzer, LOW);
  digitalWrite(LED_BUILTIN, LOW);
  isAlarmOn = false;
  alarmStatus = 0;
  writeAlarmToEEPROM(aHr, aMin, alarmStatus, repeatAlarmStatus);
}

//GET TEMPERATURE AND HUMIDITY FROM DHT11 DEVICE.
void getDHTValues() {
  rH = dht.readHumidity();
  temp = dht.readTemperature();
  float tempF = dht.readTemperature(true);
  lcd.setCursor(0, 1);
  //lcd.print("T:");
  lcd.write(byte(1));
  lcd.setCursor(11, 1);
  //lcd.print("RH:");
  lcd.write(byte(2));
  if (isnan(temp)) {
    lcd.setCursor(2, 1);
    lcd.print("NA");
  } else if (isnan(rH)) {
    lcd.setCursor(12, 1);
    lcd.print("NA");
  } else {
    lcd.setCursor(2, 1);
    if (sec % 15 != 0) { //SHOWS °F FOR EVERY 15TH SECOND
      if(temp < 10) {
        lcd.print(" ");
      }
      lcd.print(temp, 1);
      lcd.setCursor(6, 1);
      lcd.print("\xDF""C");
    } else {
      if(tempF < 10) {
        lcd.print(" ");
      }
      lcd.print(tempF, 1);
      lcd.setCursor(6, 1);
      lcd.print("\xDF""F");
    }
  
    if (rH > 99) {
      lcd.setCursor(12, 1);
      lcd.print(rH, DEC);
    } else {
      if (rH > 9) {
        lcd.setCursor(13, 1);
        lcd.print(rH, DEC);
      } else {
        lcd.setCursor(13, 1);
        lcd.print(rH, DEC);
        lcd.setCursor(14, 1);
        lcd.println("%");
        lcd.setCursor(15, 1);
        lcd.println(" ");
        return;
      }
    }
    lcd.setCursor(15, 1);
    lcd.println("%");
  }
}

//SET SINGLE OR REPEATED ALARM
void setupAlarm() {
  if (setTime == 0 && digitalRead(btnSetAlarm) == 0) {
    setAlarm++;
    mode = 1;
  }
  if (mode == 1) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Alarm ");
    lcd.setCursor(6, 0);
    if (setAlarm == 0) {
      if (isRepeatAlarmOn) {
        readAlarmFromEEPROM();
      } else {
        aHr = hr;
        aMin = min;
      }
      setAlarm = 1;
    }
    if (aHr < 10) {
      lcd.print("0");
    }
    lcd.print(aHr, DEC);
    lcd.setCursor(8, 0);
    lcd.print(":");
    lcd.setCursor(9, 0);
    if (aMin < 10) {
      lcd.print("0");
    }
    lcd.print(aMin, DEC);

    lcd.setCursor(12, 0);
    if (isAlarmOn == 1) {
      lcd.println(" ON ");
    } else {
      lcd.println(" OFF");
    }
    lcd.setCursor(0, 1);
    lcd.print("Alarm Repeat ");
    lcd.setCursor(12, 1);
    if (isRepeatAlarmOn == 1) {
      lcd.println(" ON ");
    } else {
      if (setAlarm == 4) {
        lcd.println(" OFF");
      } else {
        if (isRepeatAlarmOff ) {
          lcd.println(" ON ");
        } else {
          lcd.println(" OFF");
        }
      }
    }
    if (digitalRead(btnInc) == 0) {
      if (setAlarm == 1) aHr++;
      if (setAlarm == 2) aMin++;
      if (setAlarm == 3) {
        isAlarmOn = !isAlarmOn;
        if (isAlarmOn) {
          isRepeatAlarmOn = false;
        }
      } 
      if (setAlarm == 4) { 
        isRepeatAlarmOn = !isRepeatAlarmOn;
        if (isRepeatAlarmOn) {
          isAlarmOn = false;
        }
      };
    }
    if (aHr > 23) aHr = 0;
    if (aMin > 59) aMin = 0;

    if (digitalRead(btnDec) == 0) {
      if (setAlarm == 1) aHr--;
      if (setAlarm == 2) aMin--;
      if (setAlarm == 3) {
        isAlarmOn = !isAlarmOn;
        if (isAlarmOn) {
          isRepeatAlarmOn = false;
        }
      } 
      if (setAlarm == 4) { 
        isRepeatAlarmOn = !isRepeatAlarmOn;
        if (isRepeatAlarmOn) {
          isAlarmOn = false;
        }
      };
    }
    if (aHr < 0) aHr = 23;
    if (aMin < 0) aMin = 59;

    if (setAlarm == 5) {
      mode = 2;
    }
  }
  if (digitalRead(btnSetTime) == 0 && mode > 0 && setAlarm < 10) {
    mode = 0;
    setAlarm = 0;
    lcd.clear();
    lcd.setCursor(0, 0);
    if (isAlarmOn) {
      lcd.print("Alarm        ON ");
      writeAlarmToEEPROM(aHr, aMin, alarmStatus, 0);
    } else if (isRepeatAlarmOn) {
      lcd.print("Repeat Alarm ON ");
      writeAlarmToEEPROM(aHr, aMin, 0, repeatAlarmStatus);
    } else {
      lcd.print("Alarm Not Set!");
      writeAlarmToEEPROM(aHr, aMin, 0, 0);
    }
    readAlarmFromEEPROM();
    delay(1000);
  }
}

void setupRemainder() {
  if (setTime == 0 && digitalRead(btnSetAlarm) == 0) {
      setAlarm++;
    }
    lcd.clear();
    lcd.setCursor(13, 0);
    if (isRemainderOn) {
      lcd.print("ON");
    } else {
      lcd.print("OFF");
    }
    lcd.setCursor(0, 1);
    lcd.print(rDays, DEC);
    lcd.setCursor(8, 1);
    if (rHr < 10) {
      lcd.print("0");
    }
    lcd.print(rHr, DEC);
    lcd.setCursor(11, 1);
    if (rMin < 10) {
      lcd.print("0");
    }
    lcd.print(rMin, DEC);
    lcd.setCursor(14, 1);
    if (rSec < 10) {
      lcd.print("0");
    }
    lcd.print(rSec, DEC);

    if (digitalRead(btnInc) == 0) {
      if (setAlarm == 5) isRemainderOn = !isRemainderOn; 
      if (setAlarm == 6) rDays++;
      if (setAlarm == 7) rHr++;
      if (setAlarm == 8) rMin++;
      if (setAlarm == 9) rSec++;
    }
    if (rDays > 365) rDays = 0;
    if (rHr > 23) rHr = 0;
    if (rMin > 59) rMin = 0;
    if (rSec > 59) rSec = 0;

    if (digitalRead(btnDec) == 0) {
      if (setAlarm == 5) isRemainderOn = !isRemainderOn; 
      if (setAlarm == 6) rDays--;
      if (setAlarm == 7) rHr--;
      if (setAlarm == 8) rMin--;
      if (setAlarm == 9) rSec--;
    }
    if (rDays < 0) rDays = 365;
    if (rHr < 0) rHr = 23;
    if (rMin < 0) rMin = 59;
    if (rSec < 0) rSec = 59;

    lcd.setCursor(0, 0);
    lcd.print("Remainder");
    if (rDays < 100) {
      lcd.setCursor(2, 1);
    } else {
      lcd.setCursor(3, 1);
    }
    lcd.print("Days ");
    lcd.setCursor(10, 1);
    lcd.print(":");
    lcd.setCursor(13, 1);
    lcd.print(":");

  if (setAlarm == 10) {
    setAlarm = 0;
    mode = 0;
    if (isRemainderOn) {
      remainderStatus = 1;
      writeRemainderToEEPROM(rDays, rHr, rMin, rSec, remainderStatus);
      showRemainderTime("Remainder ON at");
      delay(5000);
    }
    lcd.clear();
    lcd.setCursor(0, 0);
    if (isRepeatAlarmOn) {
      lcd.print("Repeat Alarm ON ");
    } else {
      if (isAlarmOn) {
        lcd.print("Alarm        ON ");
      }
    }
    lcd.setCursor(0, 1);
    if (isRemainderOn) {
      lcd.print("Remainder    ON ");
    } else {
      lcd.print("Remainder    OFF");
    }

    if (alarmStatus == 1 || repeatAlarmStatus == 1) {
      writeAlarmToEEPROM(aHr, aMin, alarmStatus, repeatAlarmStatus);
    } else {
      writeAlarmToEEPROM(-1, -1, alarmStatus, alarmStatus);
    }
    delay(2000);
  }
  if (digitalRead(btnSetTime) == 0 && mode > 0 && setAlarm < 10) {
    mode = 0;
    setAlarm = 0;
    if (isRemainderOn) {
      remainderStatus = 1;
      writeRemainderToEEPROM(rDays, rHr, rMin, rSec, remainderStatus);
      showRemainderTime("Remainder ON at");
      delay(3000);
    }
    lcd.clear();
    lcd.setCursor(0, 0);
    if (isRepeatAlarmOn) {
      lcd.print("Repeat Alarm ON ");
    } else {
      if (isAlarmOn) {
        lcd.print("Alarm        ON ");
      } else {
        lcd.print("Alarm        OFF");
      }
    }
    lcd.setCursor(0, 1);
    if (isRemainderOn) {
      lcd.print("Remainder    ON ");
    } else {
      lcd.print("Remainder    OFF");
    }
    readAlarmFromEEPROM();
    delay(1000);
  }
}
void showRemainderTime(String row0) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(row0);
  lcd.setCursor(0, 1);
  lcd.print(futureRemainder.year());
  lcd.setCursor(5, 1);
  int fmon = futureRemainder.month();
  lcd.print(monthChar[fmon]);
  lcd.setCursor(8, 1);
  int fd = futureRemainder.day();
  if (fd < 10) {
    lcd.print("0");
  }
  lcd.print(fd, DEC);
  lcd.setCursor(11, 1);
  int fh = futureRemainder.hour();
  if (fh < 10) {
    lcd.print("0");
  }
  lcd.print(fh, DEC);
  lcd.setCursor(13, 1);
  lcd.print(":");
  lcd.setCursor(14, 1);
  int fmin = futureRemainder.minute();
  if (fmin < 10) {
    lcd.print("0");
  }
  lcd.print(fmin, DEC);
}

//SET LCD CURSOR POSITION TO CHANGE/SET TIME DATE OR ALARM
void cursorPosition() {
  if (mode == 0) {
    if (setTime == 1) {
      lcd.setCursor(0, 0);
      lcd.print("  ");
    }
    if (setTime == 2) {
      lcd.setCursor(3, 0);
    lcd.print("  ");
    }
    if (setTime == 3) {
      lcd.setCursor(6, 0);
      lcd.print("  ");
    }
    if (setTime == 4) {
      lcd.setCursor(11, 0);
      lcd.print("   ");
    }
    if (setTime == 5) {
      lcd.setCursor(14, 0);
      lcd.print("  ");
    }
    if (setTime == 6) {
      lcd.setCursor(12, 0);
      lcd.println("    ");
    }
  } else if (mode == 1) {
      if (setAlarm == 1) {
        lcd.setCursor(6, 0);
        lcd.print("  ");
      }
      if (setAlarm == 2) {
        lcd.setCursor(9, 0);
        lcd.print("  ");
      }
      if (setAlarm == 3) {
        lcd.setCursor(13, 0);
        if (isAlarmOn) {
          lcd.print("   ");
          alarmStatus = 1;
        } else {
          lcd.print("   ");
          alarmStatus = 0;
        }
      }
      if (setAlarm == 4) {
        lcd.setCursor(13, 1);
        if (isRepeatAlarmOn) {
          lcd.print("   ");
          repeatAlarmStatus = 1;
        } else {
          lcd.print("   ");
          repeatAlarmStatus = 0;
        }
      }
    } else {
      if (setAlarm == 5) {
        lcd.setCursor(13, 0);
        lcd.print("   ");
      }
      if (setAlarm == 6) {
        lcd.setCursor(0, 1);
        if (rDays < 99) {
          lcd.print("  ");
        } else {
          lcd.print("   ");
        }
      }
      if (setAlarm == 7) {
        lcd.setCursor(8, 1);
        lcd.print("  ");
      }
      if (setAlarm == 8) {
        lcd.setCursor(11, 1);
        lcd.print("  ");
      }
      if (setAlarm == 9) {
        lcd.setCursor(14, 1);
        lcd.print("  ");
      }
    }
}

//STORE ALARM's HOUR, MINUTE, SINGLE ALARM STATUS AND REPEATED ALARM STATUS IN EEPROM. IT HELPS TO STORE VALUES
void writeAlarmToEEPROM(int aHr, int aMin, int alarmStatus, int repeatAlarmStatus) {
  EEPROM.write(0, aHr);
  EEPROM.write(1, aMin);
  EEPROM.write(2, alarmStatus);
  EEPROM.write(3, repeatAlarmStatus);
}

//READ EEPROM STORED VALUES. IT HELPS TO GET STORED VALUES AFTER POWER ON FROM POWER OFF
void readAlarmFromEEPROM() {
  aHr = EEPROM.read(0);
  aMin = EEPROM.read(1);
  alarmStatus = EEPROM.read(2);
  repeatAlarmStatus = EEPROM.read(3);
}

void writeRemainderToEEPROM(int rDays, int rHr, int rMin, int rSec, int remainderStatus) {
  futureRemainder = (now + TimeSpan(rDays, rHr, rMin, rSec)); //FUTURE nTH DAY, HOURS, MINUTES, SECONDS
  EEPROM.write(4, rDays);
  EEPROM.write(5, rHr);
  EEPROM.write(6, rMin);
  EEPROM.write(7, rSec);
  EEPROM.write(8, remainderStatus);
}
void readRemainderFromEEPROM() {
  rDays = EEPROM.read(4);
  rHr = EEPROM.read(5);
  rMin = EEPROM.read(6);
  rSec = EEPROM.read(7);
  remainderStatus = EEPROM.read(8);
  now = rtc.now();
  futureRemainder = (now + TimeSpan(rDays, rHr, rMin, rSec)); //FUTURE nTH DAY, HOURS, MINUTES, SECONDS
}
