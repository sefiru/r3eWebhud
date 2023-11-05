#include "sha1.h"
#include "base64.h"

char* gethandshakeKey(const char string1[]) {
    const char string2[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    char* chars = sha1(string1, string2);
    char charss[41];
    strcpy_s(charss, 41, chars);
    return hex_to_base64(charss);
    /*char* charsss = hex_to_base64(charss);
    //printf("%s\n", charsss);
    char charz[30];
    strcpy_s(charz, 30, charsss);
    return &charz;*/
}