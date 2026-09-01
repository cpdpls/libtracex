#include <stdlib.h>
#include <string.h>
#include "tx_trace_open.h"
#include "tx_trace_errno.h"
#include "abstractio/aio.h"

TXTRACE_Ret_t traceX_getSchemesDescriptors(struct tx_schemes_descriptor_list_t **descriptors)
{
     TXTRACE_Ret_t status;
     struct aio_schemes_list_t *schemes;
     struct aio_driver_descriptor_t *aio_descr;
     uint32_t i;

     aio_descr = NULL;
     schemes = NULL;

     if (descriptors == NULL)
     {
          status = TX_TRACE_BAD_INPUT_PTR;
          goto handle_exit;
          
     }
     status = (TXTRACE_Ret_t)aio_getSchemes(&schemes);

     if (status != AIO_SUCCESS)
     {
          goto handle_exit;
     }

     *descriptors = (struct tx_schemes_descriptor_list_t *)malloc(sizeof(struct tx_schemes_descriptor_list_t));
     if (*descriptors == NULL)
     {
          status = TX_TRACE_ALLOC_FAIL;
          goto handle_error;
     }

     (*descriptors)->count = schemes->count;

     (*descriptors)->schemes = (struct tx_scheme_descr_t**)malloc(sizeof(struct tx_scheme_descr_t*) * schemes->count);
     if ((*descriptors)->schemes == NULL)
     {
          status = TX_TRACE_ALLOC_FAIL;
          goto handle_error;
     }

     for (i = 0; i < schemes->count; i++)
     {
          status = (TXTRACE_Ret_t)aio_schemeToDriverDescriptor(schemes->schemes[i], &aio_descr);

          if (status != AIO_SUCCESS)
          {
               goto handle_error;
          }


          (*descriptors)->schemes[i] = (struct tx_scheme_descr_t *)malloc(sizeof(struct tx_scheme_descr_t));
          if ((*descriptors)->schemes[i] == NULL)
          {
               status = TX_TRACE_ALLOC_FAIL;
               goto handle_error;
          }

          (*descriptors)->schemes[i]->scheme_name = (uint8_t*)malloc(sizeof(uint8_t) * (strlen(aio_descr->driver_scheme) + 1));
          if ((*descriptors)->schemes[i]->scheme_name == NULL)
          {
               status = TX_TRACE_ALLOC_FAIL;
               goto handle_error;
          }

          memcpy((*descriptors)->schemes[i]->scheme_name, aio_descr->driver_scheme, strlen(aio_descr->driver_scheme) + 1);

          switch(aio_descr->type)
          {
               case E_AIO_DRIVER_PHYSICAL_TYPE:
                    (*descriptors)->schemes[i]->type = E_TX_SCHEME_PHYSICAL_TYPE;
                    break;
               case E_AIO_DRIVER_STREAM_TYPE:
                    (*descriptors)->schemes[i]->type = E_TX_SCHEME_STREAM_TYPE;
                    break;
               
               default:
                    (*descriptors)->schemes[i]->type = E_TX_SCHEME_INVALID_TYPE;
                    break;

          }
          aio_destroyDriverDescriptor(&aio_descr);
     }

     status = TX_TRACE_SUCCES;
     goto handle_success;

handle_error:
     aio_destroyDriverDescriptor(&aio_descr);

handle_success:
     aio_destroySchemes(&schemes);

handle_exit:
     return status;
}

void traceX_destroySchemesDescriptors(struct tx_schemes_descriptor_list_t **descriptors)
{
     uint32_t i;

     if (descriptors != NULL)
     {
          if (*descriptors != NULL)
          {
               if ((*descriptors)->schemes != NULL)
               {
                    for (i = 0 ; i < (*descriptors)->count; i++)
                    {
                         if ((*descriptors)->schemes[i] != NULL)
                         {
                              if ((*descriptors)->schemes[i]->scheme_name != NULL)
                              {
                                   free((*descriptors)->schemes[i]->scheme_name);
                                   (*descriptors)->schemes[i]->scheme_name = NULL;
                              }
                              free((*descriptors)->schemes[i]);
                              (*descriptors)->schemes[i] = NULL;
                         }

                    }
                    free((*descriptors)->schemes);
                    (*descriptors)->schemes = NULL;
               }

               free(*descriptors);
               *descriptors = NULL;
          }
     }
}