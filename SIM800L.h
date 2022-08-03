#ifndef SIM800L
#define SIM800L

#define GSM_SERIAL_DEBUG
#ifdef GSM_SERIAL_DEBUG
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_PRINT(x)
#endif

#include <Arduino.h>
#include <SoftwareSerial.h>
#include "unicode.hpp"

#endif

class SMS
{
private:
public:
    String status;
    String sender;
    String message;
    String date;
};

class GSM
{
private:
    const uint _timeout = 1000;
    bool ready = true;
    uint8_t RST_PIN;
    SoftwareSerial *ss;
    String _buffer;

    String _readSerial();
    void config();

    String utf16decode(String utf16);

public:
    GSM(uint8_t RX_PIN, uint8_t TX_PIN, uint8_t RST_PIN);
    void begin(uint32_t baud = 115200);
    void reset();
    void handle();

    void sendRaw(String command);
    void setBaud(uint32_t baud);

    String getSignalQuality();

    bool readMessage(String index, SMS &sms);
    SMS parseSMS(String message);

    void readAllUnread();
    void deleteAllMessages();

    bool sendSMS(String number, String message);

    bool (*newSMSCallback)(SMS sms);
};
