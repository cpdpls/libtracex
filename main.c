#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <signal.h>

#include "tracex.h"
#include "time.h"
#include "tracex_debug.h"


int main(void)
{
    size_t total_file_size;
    char *buffer;
    TRACEX_Ret_t status;
    struct TRACEX_header_t *header;
    TRACEX_handler_t *handler;

    TRACEX_INIT();
    FILE *test = fopen("trace.trx", "rb");

    fseek(test, 0L, SEEK_END);
    total_file_size = ftell(test);
    fseek(test, 0L, SEEK_SET);

    buffer = malloc(total_file_size);
    fread(buffer, total_file_size, 1, test);


    status = TRACEX_createHandler(&handler);
    srand(time(NULL));

    if (status != TRACEX_SUCCESS)
    {
        printf("%s\n", TRACEX_strerror(status));
        return 0;

    }

    status = TRACEX_parse(handler, buffer, total_file_size);
    printf("%s\n", TRACEX_strerror(status));


    if (status == TRACEX_SUCCESS)
    {
        status = TRACEX_getHeader(handler, &header);
        TRACEX_print_user_header(header);
        TRACEX_print_raw_header(handler);

        TRACEX_object_iterator_t *iter;
        const TRACEX_object_t * object;

        TRACEX_objectIteratorInit(handler, &iter);

        while (TRACEX_objectIteratorNext(iter, &object) != TRACEX_OBJ_ITER_END)
        {
            //printf("%.32s\n", object->name);
        }

        TRACEX_objectIteratorEnd(&iter);
        


    }
    free(buffer);
    TRACEX_destroyHandler(&handler);
    fclose(test);

}
