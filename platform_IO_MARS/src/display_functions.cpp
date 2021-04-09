#include "display_functions.h"

extern TFT_eSPI tft;

extern String Event1;

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