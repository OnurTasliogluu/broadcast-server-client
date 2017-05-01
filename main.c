#include "server.h"
#include "debug.h"

#include <stdio.h>

int32_t main()
{
    if (server_init())
    {
        DEBUG_ERROR("Cocuk Process Olusturulamadı.");
    }
    return 0;
}