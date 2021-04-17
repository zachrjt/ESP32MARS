#include "SPI_functions.h"          //SPI functions header file

extern Button2 btn1;                 //creating button object for usage
extern Button2 btn2;                 //creating button object for usage
extern Button2 btn3;                 //creating button object for usage

extern SPIClass SDSPI; //defines the spi bus for use with the SD card
extern SPIClass PIC1_SPI;    //defines the spi bus for use with the PIC with clock display
extern SPIClass PIC2_SPI;    //defines the spi bus for use with the PIC with alarm capabilities
SPISettings PICSPISettings =    {
                                500000, 
                                MSBFIRST,
                                SPI_MODE0
                                };

uint8_t Time0;
uint8_t Time1;
uint8_t Time2;
extern int TIMESTAMP;   //to change when getTimeFromPIC() is called
extern int DATESTAMP;   //to change when getTimeFromPIC() is called
int Hours;
int Minutes;
int Seconds;

int ALARMSTAMP;         //uhhhhhhhhhhh I am lazy and tired of making fifty thousand variables ok im sorry
int SnoozeHours = 99;
int SnoozeMinutes = 99;
int SnoozeSeconds = 99;
int snoozeFlag;
uint8_t AlarmFlag;
int PomodoroMinutes = 99;
int PomodoroSeconds = 99;
int PomodoroFlag;           //0 if not using, 1 if study period, 2 if break period

extern Calendar myCalendar;
extern long sector_table;



void PICSPISetup()
{
    pinMode(PIC_MISO, INPUT);
    pinMode(PIC1_SPICS, OUTPUT);
    digitalWrite(PIC1_SPICS, HIGH);
    pinMode(PIC2_SPICS, OUTPUT);
    digitalWrite(PIC2_SPICS, HIGH);
    PIC1_SPI.begin(PIC_SPICLK, PIC_MISO, PIC_MOSI, PIC1_SPICS);
    PIC2_SPI.begin(PIC_SPICLK, PIC_MISO, PIC_MOSI, PIC2_SPICS);
}



void sendTimetoPIC1(void)
{
    Hours = TIMESTAMP / 10000;              //separate UTC timestamp into different int vars
    Minutes = (TIMESTAMP % 10000) / 100;
    Seconds = TIMESTAMP % 100;

    Hours += 18;   //convert from UTC time to local time
    Hours %= 24;

    Time0 = 0;
    Time1 = 0;
    Time2 = 0;

    if(Hours > 11)      //Conversions from ESP32 time to PIC display time vars (Hope this doesnt take too long in processing time)
    {
        Time2 += 0b00001000;
        Time0 += (uint8_t)(((Hours - 12) / 10) << 7);
        Time0 += (uint8_t)(((Hours - 12) % 10) << 3);
    }
    else if(Hours)
    {
        Time0 += (uint8_t)((Hours / 10) << 7);
        Time0 += (uint8_t)((Hours % 10) << 3);
    }
    else 
    {
        Time0 += 0b10010000;
    }

    Time0 += (uint8_t)(Minutes / 10);
    Time1 += (uint8_t)((Minutes % 10) << 4);

    Time1 += (uint8_t)(Seconds / 10);
    Time2 += (uint8_t)((Seconds % 10) << 4);

    /*Serial.println(Time0);
    Serial.println(Time1);
    Serial.println(Time2);*/

    digitalWrite(PIC1_SPICS, LOW);              //send time to PIC through SPI protocol
    PIC1_SPI.beginTransaction(PICSPISettings);

    PIC1_SPI.transfer(Time2);
    PIC1_SPI.transfer(Time1);
    PIC1_SPI.transfer(Time0);

    PIC1_SPI.endTransaction();
    digitalWrite(PIC1_SPICS, HIGH);
}



void  getTimefromPIC1(void)                     //Dont pull from PIC more than once a second or you will get erroneous values for Time2
{
    digitalWrite(PIC1_SPICS, LOW);
    PIC1_SPI.beginTransaction(PICSPISettings);

    Time2 = PIC1_SPI.transfer(FIRST_TIME_REQUEST);
    Time1 = PIC1_SPI.transfer(SUBSEQUENT_TIME_REQUEST);
    Time0 = PIC1_SPI.transfer(SUBSEQUENT_TIME_REQUEST);

    PIC1_SPI.endTransaction();
    digitalWrite(PIC1_SPICS, HIGH);

    Serial.println(Time2);
    Serial.println(Time1);
    Serial.println(Time0);

    Hours = 0;
    Minutes = 0;
    Seconds = 0;
                                                    //Hope this doesnt take too long in processing time
    Hours += (int)((Time0 >> 7) * 10);              //bitmasking and shifting according to PIC time display standars
    Hours += (int)((Time0 & 0b01111000) >> 3);

    Minutes += (int)(Time0 & 0b00000111) * 10;
    Minutes += (int)((Time1 & 0b11110000) >> 4);

    Seconds += (int)(Time1 & 0b00001111) * 10;
    Seconds += (int)((Time2 & 0b11110000) >> 4);


    if((Time2 & 0b00001000) == 0b00001000)
    {
        if(Hours < 12)
        {
            Hours += 12;                                //24 hour clock
        }
    }
    else
    {
        Hours %= 12;                                //make it 0:00 am rather than 12:00 am
    }

    TIMESTAMP = ((Hours + 6) % 24) * 10000 + Minutes * 100 + Seconds;

    if(TIMESTAMP < 1)
    {
        WifiUpdate();
    }

    Serial.println(Hours);
    Serial.println(Minutes);
    Serial.println(Seconds);
    Serial.println(TIMESTAMP);
}



void sendAlarmFlagtoPIC2(void)
{
    digitalWrite(PIC2_SPICS, LOW);
    PIC2_SPI.beginTransaction(PICSPISettings);

    PIC2_SPI.transfer(AlarmFlag);

    PIC2_SPI.endTransaction();
    digitalWrite(PIC2_SPICS, HIGH);
}



void clockButtonsSetup()
{
    pinMode(BUTTON_1_PIN, INPUT);
    pinMode(BUTTON_2_PIN, INPUT);
    pinMode(BUTTON_3_PIN, INPUT);
    btn1.setPressedHandler(pressed);
    btn2.setPressedHandler(pressed);
    btn3.setPressedHandler(pressed);
}



//Button Interrupts (?) for features, for some reason these are each triggered once during Start up so AlarmFlag is default off
void pressed(Button2& btn) 
{
    static int setup = 0;

    if (setup > 3)
    {
        if(btn ==  btn1)
        {
            if (AlarmFlag)  //Snooze Alarm if its on
            {
            AlarmFlag = ALARM_OFF;
            sendAlarmFlagtoPIC2();

            if(Minutes < 55)    //Set up snooze event 5 minutes from now
            {
                SnoozeMinutes = Minutes + SNOOZE_TIME_MINUTES;
                SnoozeHours = Hours;
            }
            else 
            {
                SnoozeMinutes = Minutes + SNOOZE_TIME_MINUTES - 60;
                SnoozeHours = (Hours + 1) % 24;
            }
            SnoozeSeconds = 0;
            snoozeFlag = 1;
            }
        }
        else if (btn ==  btn2)
        {
            if (AlarmFlag)  //Turn off Alarm if its on
            {
                AlarmFlag = ALARM_OFF;
                sendAlarmFlagtoPIC2();
            }
        }
        else if (btn == btn3)
        {
            if(!PomodoroFlag)
            {
                PomodoroMinutes = 20;
                PomodoroSeconds = 0;
                PomodoroFlag = 1;
                displayPomodoro();
            }
        }
    }
    else
    {
        setup++;
    }
}



void checkSnooze()
{
    if((Hours == SnoozeHours) && (Minutes == SnoozeMinutes) && (Seconds == SnoozeSeconds))
    {
        AlarmFlag = ALARM_ON;
        sendAlarmFlagtoPIC2();
        SnoozeHours = 99;
        SnoozeMinutes = 99;
        SnoozeSeconds = 99;
        snoozeFlag = 0;
    }
}



void updatePomodoroTime(void)
{
    if (PomodoroSeconds == 0)
    {
        if(PomodoroMinutes == 0)
        {
            if(PomodoroFlag == 1)
            {
                PomodoroMinutes = 5;
                PomodoroSeconds = 0;
                AlarmFlag = ALARM_ON;
                sendAlarmFlagtoPIC2();
                PomodoroFlag = 2;
            }
            else
            {
                PomodoroMinutes = 99;
                PomodoroSeconds = 99;
                AlarmFlag = ALARM_ON;
                sendAlarmFlagtoPIC2();
                PomodoroFlag = 0;
            }
        }
        else
        {
            PomodoroMinutes--;
            PomodoroSeconds = 59;
        }
    }
    else
    {
        PomodoroSeconds--;
    }
}