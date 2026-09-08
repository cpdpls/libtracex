#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <signal.h>

#include "tracex.h"
#include "time.h"
#include "tracex_debug.h"


int main(void)
{
    char buffer[200];
    char *buff_ptr;
    TRACEX_Ret_t status;
    struct TRACEX_header_t *header;
    TRACEX_handler_t *handler;

    TRACEX_INIT();
    FILE *test = fopen("trace.trx", "rb");

    fread(buffer, 200, 1, test);


    status = TRACEX_createHandler(&handler);
    srand(time(NULL));

    if (status != TRACEX_SUCCESS)
    {
        printf("%s\n", TRACEX_strerror(status));
        return 0;

    }

    buff_ptr = &buffer[0];
    do
    {
        int random = ((rand()) % 15) + 1;

        status = TRACEX_parse(handler, buff_ptr, random);
        buff_ptr += random;
        printf("%s\n", TRACEX_strerror(status));

    } while(status == TRACEX_NEED_MORE);


    if (status == TRACEX_SUCCESS)
    {
        status = TRACEX_getHeader(handler, &header);
        TRACEX_print_user_header(header);
        TRACEX_print_raw_header(handler);

    }
    TRACEX_destroyHandler(&handler);
    fclose(test);

}
