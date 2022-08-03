namespace Utf16
{
    inline bool isHighSurrogate(uint16_t codeunit)
    {
        return codeunit >= 0xD800 && codeunit < 0xDC00;
    }

    inline bool isLowSurrogate(uint16_t codeunit)
    {
        return codeunit >= 0xDC00 && codeunit < 0xE000;
    }

    class Codepoint
    {
    public:
        Codepoint() : _highSurrogate(0), _codepoint(0) {}

        bool append(uint16_t codeunit)
        {
            if (isHighSurrogate(codeunit))
            {
                _highSurrogate = codeunit & 0x3FF;
                return false;
            }

            if (isLowSurrogate(codeunit))
            {
                _codepoint =
                    uint32_t(0x10000 + ((_highSurrogate << 10) | (codeunit & 0x3FF)));
                return true;
            }

            _codepoint = codeunit;
            return true;
        }

        uint32_t value() const
        {
            return _codepoint;
        }

    private:
        uint16_t _highSurrogate;
        uint32_t _codepoint;
    };
}
namespace Utf8
{
    inline void encodeCodepoint(uint32_t codepoint32, String &str)
    {
        // this function was optimize for code size on AVR

        if (codepoint32 < 0x80)
        {
            str += String(char(codepoint32));
        }
        else
        {
            // a buffer to store the string in reverse
            char buf[5];
            char *p = buf;

            *(p++) = 0;
            *(p++) = char((codepoint32 | 0x80) & 0xBF);
            uint16_t codepoint16 = uint16_t(codepoint32 >> 6);
            if (codepoint16 < 0x20)
            { // 0x800
                *(p++) = char(codepoint16 | 0xC0);
            }
            else
            {
                *(p++) = char((codepoint16 | 0x80) & 0xBF);
                codepoint16 = uint16_t(codepoint16 >> 6);
                if (codepoint16 < 0x10)
                { // 0x10000
                    *(p++) = char(codepoint16 | 0xE0);
                }
                else
                {
                    *(p++) = char((codepoint16 | 0x80) & 0xBF);
                    codepoint16 = uint16_t(codepoint16 >> 6);
                    *(p++) = char(codepoint16 | 0xF0);
                }
            }

            while (*(--p))
            {
                str += String(*p);
            }
        }
    }
}