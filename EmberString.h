
#ifndef EMBER_STRING_H__
#define EMBER_STRING_H__

#include "ember.h"

namespace ember {
    unsigned int hash(const unsigned char *str);

    // Delete numChars from dest, by shifting the string numChars to the left
    void strerase(char *dest, int numChars);

    const size_t MAX_OUTPUT_LINE = 2048;
}


#endif
