#include "SIM800L.h"

String GSM::_readSerial()
{
    ulong s = millis();
    String res = "";
    while (millis() - s < _timeout)
    {
        while (ss->available())
        {
            s = millis();
            char c = ss->read();
            res += c;
            yield();
        }
    }
    DEBUG_PRINT(res);
    return res;
}

void GSM::config()
{
    DEBUG_PRINTLN(F("gsm config"));
    // set message format
    ss->println(F("AT+CMGF=1"));
    _buffer = _readSerial();
    // set character set
    ss->println(F("AT+CSCS=\"UCS2\""));
    _buffer = _readSerial();
    // set text mode params
    // ss->println(F("AT+CSMP=17,168,0,8"));
    ss->println("AT+CSMP=49,167,0,8");
    _buffer = _readSerial();
}

void GSM::readAllUnread()
{
    bool success = true;
    SMS sms;
    SMS temp;
    int i = 1;
    while (readMessage(String(i), sms))
    {
        while (readMessage(String(i + 1), temp))
        {
            if (sms.sender != temp.sender)
                break;
            sms.message += temp.message;
            i++;
        }

        if (!newSMSCallback(sms))
        {
            success = false;
            break;
        }
        i++;
    }
    if (success)
    {
        ss->println(F("AT+CMGDA=\"DEL READ\""));
        _readSerial();
    }
}

void GSM::handle()
{
    if (!ss->available())
        return;
    _buffer = _readSerial();

    if (!ready)
    {
        if (_buffer.indexOf("SMS") != -1)
        {
            ready = true;
            setBaud(9600);
            config();
            DEBUG_PRINTLN(F("We are ready!"));
            yield();
            readAllUnread();
        }
        return;
    }
    else
    {
        ulong s = millis();
        while (millis() - s < 20000)
        {
            if (ss->available())
            {
                _readSerial();
                s = millis();
            }
            yield();
        }
        readAllUnread();
    }
}

void GSM::reset()
{
    pinMode(RST_PIN, OUTPUT);
    digitalWrite(RST_PIN, 0);
    delay(1000);
    pinMode(RST_PIN, PULLUP);
    ready = false;
}

SMS GSM::parseSMS(String message)
{
    SMS sms;
    int idx1 = message.indexOf(",", 1);
    sms.status = message.substring(1, idx1 - 1);
    int idx2 = message.indexOf(",", idx1 + 1);
    sms.sender = utf16decode(message.substring(idx1 + 2, idx2 - 1));
    idx1 = idx2 + 5;
    idx2 = message.indexOf("\"", idx1 + 1);
    sms.date = message.substring(idx1, idx2 - 3);
    String msg = message.substring(idx2 + 3, message.indexOf("OK") - 4);
    sms.message = utf16decode(msg);
    return sms;
}

String GSM::utf16decode(String utf16text)
{
    Utf16::Codepoint codepoint;
    String text;
    int n = utf16text.length();
    uint16_t U;
    char S[5];
    for (int i = 0; i < n; i += 4)
    {
        utf16text.substring(i, i + 4).toCharArray(S, 5);
        U = strtoul(S, NULL, 16);
        if (codepoint.append(U))
            Utf8::encodeCodepoint(codepoint.value(), text);
    }
    return text;
}

void GSM::sendRaw(String command)
{
    ss->print(command);
}

GSM::GSM(uint8_t RX_PIN, uint8_t TX_PIN, uint8_t RST_PIN)
{
    this->RST_PIN = RST_PIN;
    this->ss = new SoftwareSerial(RX_PIN, TX_PIN);
    _buffer.reserve(255); // reserve memory to prevent intern fragmention
}

void GSM::begin(uint32_t baud)
{
    ss->begin(baud);
}

void GSM::setBaud(uint32_t baud)
{
    do
    {
        ss->print(F("AT+IPR="));
        ss->println(baud);
        yield();
        _buffer = _readSerial();
    } while (_buffer.indexOf("OK") == -1);
    ss->end();
    delay(1000);
    ss->begin(baud);
    delay(1000);
}

bool GSM::readMessage(String index, SMS &sms)
{
    ss->print(F("AT+CMGR="));
    ss->println(index);
    _buffer = _readSerial();
    if (_buffer.indexOf("+CMGR:") == -1)
        return false;
    _buffer = _buffer.substring(_buffer.indexOf("+CMGR:") + 7);
    sms = parseSMS(_buffer);
    return true;
}

String GSM::getSignalQuality()
{
    while (ss->available())
        ss->read();
    ss->println(F("AT+CSQ"));
    _buffer = _readSerial();
    _buffer.remove(0, 9);
    if (_buffer.startsWith("+CSQ"))
        return _buffer.substring(6, 10);
    return "0";
}

// String GSM::getUnreadIndexes()
// {
//     String indexList;
//     ss->println(F("AT+CMGL=\"ALL\""));
//     _buffer = _readSerial();
//     _buffer.remove(0, 16);
//     while (_buffer.length() > 0)
//     {
//         int q = _buffer.indexOf("+CMGL:");
//         if (q == -1)
//             break;
//         _buffer = _buffer.substring(q + 7);
//         String num = _buffer.substring(0, _buffer.indexOf("\"") - 1);
//         indexList += num + ",";
//     }
//     return indexList;
// }

void GSM::deleteAllMessages()
{
    ss->println(F("AT+CMGDA=\"DEL ALL\""));
    _readSerial();
}

// bool GSM::sendSMS(String number, String message)
// {
//     number = StringtoUTF16(number);
//     message = StringtoUTF16(message);
//     while (ss->available())
//         ss->read();
//     ss->print(F("AT+CMGS=\""));
//     ss->print(number);
//     ss->println(F("\""));
//     _buffer = _readSerial();
//     ss->println(message);
//     ss->write(0x1A);
//     ss->println();
//     _buffer = _readSerial();
//     return _buffer.indexOf("OK") > 0;
// }
