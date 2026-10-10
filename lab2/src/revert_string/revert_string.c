#include <string.h>

#include "revert_string.h"

void RevertString(char *str)
{
    size_t length = strlen(str);

    for (size_t i = 0; i < length / 2; ++i)
    {
        char temp = str[i];
        str[i] = str[length - i - 1];
        str[length - i - 1] = temp;
    }
}
