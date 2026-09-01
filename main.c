#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


#include "tx_trace.h"

int main(void)
{
    struct tx_schemes_descriptor_list_t *descriptors;
    uint64_t i;
    TX_TRACE_INIT();

    if (traceX_getSchemesDescriptors(&descriptors) != TX_TRACE_SUCCES)
    {
        traceX_destroySchemesDescriptors(&descriptors);

        TX_TRACE_EXIT();
        return 1;
    }


    for (i = 0; i < descriptors->count; i++)
    {
        printf("%s\n", descriptors->schemes[i]->scheme_name);

    }
    // if (traceX_getSchemesList(&drvr_list) != TX_TRACE_SUCCES)
    // {
    //     return 1;
    // }

    // for (i = 0 ; i < drvr_list->count; i++)
    // {
	 
    // }

    traceX_destroySchemesDescriptors(&descriptors);


    TX_TRACE_EXIT();
}
