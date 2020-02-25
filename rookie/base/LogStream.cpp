

#include <string>
#include <algorithm>


const char digits[] = "9876543210123456789";
const char* zero = digits + 9;
const char digitsHex[] = "0123456789ABCDEF";

size_t convertHex(char buf[], uintptr_t value)//dec to hex
{
    uintptr_t i = value;
    char* p = buf;

    do
    {
        int lsd = static_cast<int>(i % 16);
        i /= 16;
        *p++ = digitsHex[lsd];
    } while (i != 0); //16进制数转换为字符串

    *p = '\0';
    std::reverse(buf, p);

    return p - buf;
}
