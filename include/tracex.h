#ifndef __TRACEX_H__
#define __TRACEX_H__

#include "tracex_errno.h"
#include "tracex_header.h"

typedef struct TRACEX_handler_t TRACEX_handler_t;


TRACEX_Ret_t TRACEX_parse(TRACEX_handler_t *parser, void *buffer, size_t buffer_length);
TRACEX_handler_t *TRACEX_createParser(void);
void TRACEX_destroyParser(TRACEX_handler_t **parser);


#endif