#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


#include "tracex.h"
#include "time.h"

int main(void)
{
    char buffer[200];
    char *buff_ptr;
    TRACEX_Ret_t status;

    FILE *test = fopen("trace.trx", "rb");

    fread(buffer, 200, 1, test);

    TRACEX_handler_t *parser;

    parser = TRACEX_createParser();
    srand(time(NULL));

    if (parser == NULL)
    {
        printf("HIGH-LEVEL allocation error !\n");
        return 0;
    }

    buff_ptr = &buffer[0];
    do
    {
        // int random = (rand()) % 15;

        // printf("Trying random :%d\n", random);
        status = TRACEX_parse(parser, buff_ptr++, 1);
        printf("%s\n", TRACEX_strerror(status));

    } while(status != TRACEX_SUCCESS);


    TRACEX_destroyParser(&parser);
    fclose(test);

}
