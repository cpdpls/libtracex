#include <stdio.h>
#include <stdint.h>
#include "abstractio/aio.h"
#include "abstractio/aio_driver.h"

int main(void)
{
    AIO_BOOTSTRAP();

    AIO_EXIT();
}