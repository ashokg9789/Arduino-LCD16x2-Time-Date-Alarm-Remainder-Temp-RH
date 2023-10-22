# Arduino-LCD16x2-Time-Date-Alarm-Remainder-Temp-RH
This repository is about to display of current Time, Date with Week day/number, Temperature with Celsius &amp; Fahrenheit and Relative Humidity on 16x2 LCD screen. Also can able to set Alarm for once or repeatedly at every 24hrs at specific selected time. And also can able to set remainder at specific days.

4 Buttons works as below mentioned.
> Button(1) TIME SET : By pressing this button helps to modify or change Time(00:00:00) and Date(MmmDD & YYYY -> Month Day & Year).
   No need to set week day, it automatically detects as per modified time.

> Button(2) ALARM/REMAINDER SET : By pressing this button helps to set Single-time Alarm or Repeated Alarm (HH:mm) or Remainder (Days, HH:mm:ss).
   Single-time Alarm: By changing alarm to ON from OFF. Note: If Remainder alarm ON, Single-time Alarm will be auto OFF.
   Repeated Alarm:    By changing alarm repeat to ON from OFF. If Single-time Alarm ON, Repeat alarm will be auto OFF.
   Remainder:         This is future time and this remainder time will be added to current time.
          Ex: if Current time is 21/OCT/2023 17:40:30 and future remainder time is 15Days 06:10:20 then the Remainder alarm will be turned ON
              and buzzer will be starts sound at the date of time >> 05/NOV/2023 23:50:50
 
> Button(3) DEC : Can able to decrease the number or turned OFF/ON. This button also helps to turn OFF Single-time or Repeated or Remainder Alarm.
 
> Button(4) INC : Can able to increase the number and/or turned OFF/ON. This button also helps to turn OFF Single-time or Repeated or Remainder Alarm. 

![RTC_ARDUINO_CLOCK_TEMP_RH](https://github.com/ashokg9789/Arduino-LCD16x2-Time-Date-Alarm-Remainder-Temp-RH/assets/83178640/f7afe37e-3f69-4007-a96b-19e4bff4ec9c)
