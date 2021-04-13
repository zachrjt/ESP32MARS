#include "display_functions.h"

extern TFT_eSPI tft;

extern String Event1;
extern String weather_description;
extern int PomodoroFlag;
extern int PomodoroMinutes;
extern int PomodoroSeconds;

void displaySetup()
{
    tft.init();
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);
}

void printSplitString(String text)
{
  int wordStart = 0;
  int wordEnd = 0;
  while ( (text.indexOf(' ', wordStart) >= 0) && ( wordStart <= text.length())) {
    wordEnd = text.indexOf(' ', wordStart + 1);
    uint16_t len = tft.textWidth(text.substring(wordStart, wordEnd));
    if (tft.getCursorX() + len >= tft.width()) {
      tft.println();
      wordStart++;
    }
    tft.print(text.substring(wordStart, wordEnd));
    wordStart = wordEnd;
  }
}

void printNextEvent(void) //Prints the string on Event1 global var
{
  tft.setCursor(0, 0);
  tft.setTextColor(TFT_WHITE,TFT_BLACK);
  tft.setTextSize(2);

  printSplitString("Next event: \n");
  printSplitString(Event1);
}

void printTemperature(void) //Prints the temperature found on weather_description
{
  tft.setCursor(0, 90);
  tft.setTextColor(TFT_WHITE,TFT_BLACK);
  tft.setTextSize(2);

  printSplitString("Current Temperature:");
  printSplitString(weather_description + "C");
}

void updateDisplay(void)    //We can have bigger letters and have the two modes switch back and forth,
{                           //except the event summary dont fit with the next letter size, might 
  static int displayFocus;  //look into making shorter event summaries later on (probably not)

  tft.fillScreen(TFT_BLACK);

  //if(displayFocus)
  {
    printNextEvent();
  }
  //else
  {
    printTemperature(); 
  }
  displayFocus ^= 1;
}

void displayPomodoro(void)
{
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(TFT_WHITE,TFT_BLACK);
  tft.setTextSize(3);

  if(PomodoroFlag == 1)
  {
    printSplitString("Study Time \n" + String(PomodoroMinutes) + ":" + String(PomodoroSeconds)
                     + " left");
  }
  else if (PomodoroFlag == 2)
  {
    printSplitString("Break Time \n" + String(PomodoroMinutes) + ":" + String(PomodoroSeconds)
                     + " left");
  }
}