# mysprinkler

Quick implementation of automated irrigiation control using Beaglebone Black/Green and SainSmart 8 channel relay module.

Relay Module: https://www.amazon.ca/SainSmart-101-70-102-8-Channel-Relay-Module/dp/B0057OC5WK/ref=sr_1_3?m=A1CJB5SYI9X4XC&s=merchant-items&ie=UTF8&qid=1499620991&sr=1-3

Currently supports unlimited irrigation zones (dependant on the number of relays you have).
Supports unlimited number of programs/schedules.

Current version requires reduced and modified BlackLib project:

https://github.com/gitaaronc/BlackLib.git

or the original BlackLib project located: https://github.com/yigityuce/BlackLib.git

Configuring Global Zone information<br/>
Please see sample configuration file for more information<br/>
```
ZONES:
  1: // zone id, no dupliate ID #'s allowed. Numbering scheme doesn't matter call them what you want using only unsigned integer
    enabled: true #global enabled/disabled.
    name: Front Yard, close to road #just a friendly name for this zone
    invert_logic: true #when true, gpio is low when zone is "ON". when false, gpio is high when zone is "ON"
    gpio: 69 #the gpio number
```
Please see sample configuration yaml.<br/>
Supported program modes:<br/>

```
PROGRAMS:
  1: // program id, no duplicates allowed, use only unsigned integer value
    hour: 22 # the hour to start this program
    minute: 00 # the minutes after the hour to start this program
    interval: 1 # days between runs, works with mode set to interval
    weekdays: monday # weekdays to run the program on, works with mode set to weekdays
    mode: interval #even_only, odd_only, weekdays
    zone_detail: #details of zones included as part of this program
      1: #the ID of the zone included in this program, must match a zone id from the zone section of configuration
        duration: 25 # number of minutes to run this zone

```

