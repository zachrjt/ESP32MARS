#include "display_functions.h"

extern TFT_eSPI tft;

extern String Event1;

extern String weather_description;  //Pulls the weather value like -4

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

void printNextEvent(void)
{
    tft.setCursor(0, 0, 2);
    tft.setTextColor(TFT_WHITE,TFT_BLACK);
    tft.setTextSize(2);

    printSplitString("Next event: \n");
    printSplitString(Event1);
}

void printTemperature(void)
{
    tft.setCursor(0, 120);
    tft.setTextColor(TFT_WHITE,TFT_BLACK);
    tft.setTextSize(1);

    printSplitString("Current Temperature: ");
    printSplitString(weather_description);
}