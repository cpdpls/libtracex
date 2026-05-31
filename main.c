#include <stdio.h>
#include <stdint.h>
#include "aio.h"

int main(void)
{
    AIO_Ret_t status;
    aio_fs *file;
    AIO_BOOTSTRAP();
    
   status = aio_open(&file, "TC:test_dump.trx");

   printf("%s", aiostrerror(status));
    return 0;
}