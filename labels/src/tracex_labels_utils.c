#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "cJSON.h"
#include "tracex/tracex_labels_errno.h"

tracex_labels_ret_t tracex_labels_utils_load_json_file(const uint8_t *path, cJSON **root)
{
    tracex_labels_ret_t status;
    FILE *file_ptr = NULL;
    size_t file_size = 0;
    uint8_t *tmp_buffer = NULL;

    *root = NULL;

    /* Try to open the requested json raw file */
    file_ptr = fopen(path, "rb");

    /* Check for a valid operation */
    if (file_ptr == NULL) {
        status = TRACEX_LABELS_JSON_NOT_FOUND;
        goto handle_return;
    }

    /* Retrieve the file size in order to allocate the correc buffer size */
    fseek(file_ptr, 0, SEEK_END);
    file_size = ftell(file_ptr);
    fseek(file_ptr, 0, SEEK_SET);

    /* Check for an empty json file case */
    if (file_size == 0)     {
        status = TRACEX_LABELS_JSON_EMPTY;
        goto handle_return;
    }

    /* Allocate the actual buffer that will holds the raw file in memory */
    tmp_buffer = (uint8_t*)malloc(sizeof(uint8_t) *file_size);

    /* Check for an allocation error */
    if (tmp_buffer == NULL) {
        status = TRACEX_LABELS_JSON_ALLOC_FAILURE;
        goto handle_return;
    }

    /* Try to read the file and check if we successfully read the whole raw file*/
    if (fread(tmp_buffer, 1, file_size, file_ptr) != file_size) {
        status = TRACEX_LABELS_JSON_READ_FAILURE;
        goto handle_return;
    }

    /* Parse the actual json file */
    *root = cJSON_ParseWithLength(tmp_buffer, file_size);

    /* Check for a parsing error */
    if (*root == NULL) {
        status = TRACEX_LABELS_JSON_PARSING_FAILURE;
        goto handle_return;
    }

    status = TRACEX_LABELS_SUCCESS;

handle_return:

    /* Free all the previous allocations */
    if (tmp_buffer != NULL) {
        free(tmp_buffer);
        tmp_buffer = NULL;
    }

    if (file_ptr != NULL) {
        fclose(file_ptr);
        file_ptr = NULL;
    }
    return status;

}