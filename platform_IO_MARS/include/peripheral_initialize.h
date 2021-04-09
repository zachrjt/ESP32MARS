#ifndef PERIPHERAL_INITIALIZE_H
    #define PERIPHERAL_INITIALIZE_H

    int setup_functions(void);
    int WiFiSetup(void);
    int SDCardSetup(void);
    int WebServerSetup(void);
    int LocalTimeSetup();
    int AlarmSetup();
    int SDCardInterruptSetup();

#endif

#include "SPI_functions.h"