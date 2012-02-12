
#include "EmberString.h"

// I'd give you a link, but you and I both know I just typed "Best c
// string hash function" into google and took the first hit.
unsigned int
ember::hash(const unsigned char *str)
{
    unsigned int hash = 5381;
    int c;
    
    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    
    return hash;
}

void ember::strerase(char *dest, int numChars)
{
    char *d, *s;
    for(d = dest, s = dest + numChars; *s; d++, s++) {
        *d = *s;
    }
    *d = 0;
}
