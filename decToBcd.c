#include "config.h"
uchar decToBcd(uchar dec)
{
    uchar result1, result2;
    result1 = (dec/10)<<4;
    result2 = (dec%10);
    return result1|result2;

}
