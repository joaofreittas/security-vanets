#ifndef ASCIIHEX_H
#define ASCIIHEX_H

class Konverter
{
    public:
        void hex2ascii(byte * hex, char * ascii);
        void ascii2hex(char * ascii, byte * hex);
		String byte2strBin(byte * in, int x);
		void string2byteArray(String strIn, byte * bX);
		String byte2strB64(byte * byteIn, int x);
		void strB64toByte(String str64, byte * bOut);
};

#endif