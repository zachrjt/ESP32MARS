#ifndef PERIPHERAL_INITIALIZE_H
    #define PERIPHERAL_INITIALIZE_H




//DEFINE STATEMENT SECTION START---------------------------------------------------------------------------------------------------------------------------------------------------
//DEFINE STATEMENT SECTION END-----------------------------------------------------------------------------------------------------------------------------------------------------




//FUNCTION DECLARATION SECTION START-----------------------------------------------------------------------------------------------------------------------------------------------
    int setup_functions(void);
    int WiFiSetup(void);
    int SDCardSetup(void);
    int WebServerSetup(void);
    int LocalTimeSetup();
    int AlarmSetup();
    int SDCardInterruptSetup();
//FUNCTION DECLARATION SECTION END-------------------------------------------------------------------------------------------------------------------------------------------------




#endif  // #ifndef PERIPHERAL_INITIALIZE_H