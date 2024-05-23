# Overview

This project is a water splitting project that can used in water wells or other splitting systems that require a time-limiting system for use, a timer to limit the amount of usage and User Save and editing system.

This project has buzzer to have better user experience and uses a 16x2 character display and custom characters and animations for better User Interface.

this project controls a relay that can be used for triggering a contactor or starting a water pump that sucks the water of water well.
# Features and Description of Variables
The good Feature of this project are:



* You can register and edit people's data with a 4 by 4 Keypad
    > The Getting Data is:
    > * Phone number
    > * 4-digit Security Pin
    > * RFID tag data
    > * Limit day (Limitdow): the day of the week that the user allows to use water pomp
    > * Limit Start: the time that the user can use water
    > * Limit length: the hours of allowed usage of water  
    >
    >   > limit start > Allow zone > limit length  
    >   > 18:30       > Allow zone > 18:30 + 100 hours
    >   > this means for example the user can use water from 18:30 to 100 hours after this time // 100 hours after this time means the user is Allowed to use water from 18:30 to 100 hours (4 days and 4 hours). After this time or before Limit Start time, the user is not allowed to use water and the system accrues an error.
    > * Remaining Time: the Total Time of allowed usage of water (the user get this amount from the master. master can set the data of users and can recharge the total amount of Remain Time.    
    > ==***The Code of structure EEPRerson // stands for EEPROM Person:***==
    > ```cpp
    >   struct EEPerson {
    >       unsigned long rfid;         // 8 digits
    >       char phoneNumber[12];       // 12 digits
    >       int pinCode;                // 4 digits
    >       int dowLimit;               // 1 digit
    >       int limitStart;             // 4 digits
    >       int limitLength;            // 3 digits 
    >       unsigned long remainingTime;// 6 digits 
    >    };
    >    ```
* When you are starting the process of pump you can set a timer and after the timer runs out the system saves you time data to EEPROM and stops the process but won't stop the pump why?  


* When a customer stops the process we never turn the relay (pump) off cause if we continuously turn our pump on and off it can damage to our pump that is why I set a 2-minute timer in this 2 minute the pump is still on and the system waits for other people to start if no one enters a card and the 2 minute time runs out , we figure out we don't have anyone in queue and the pump will turn off.
