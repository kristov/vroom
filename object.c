#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "safemalloc.h"
#include "vroom.h"
#include "object.h"

vrms_object_t* vrms_object_memory_create(uint32_t fd, void* address, uint32_t size) {
    vrms_object_t* object = vrms_object_create();
    object->type = VRMS_OBJECT_MEMORY;
    object->realized = 1;

    vrms_object_memory_t* object_memory = SAFEMALLOC(sizeof(vrms_object_memory_t));
    memset(object_memory, 0, sizeof(vrms_object_memory_t));

    object_memory->fd = fd;
    object_memory->address = address;
    object_memory->size = size;
    object->object.object_memory = object_memory;

    return object;
}

vrms_object_t* vrms_object_data_create(uint32_t memory_id, uint32_t memory_offset, uint32_t memory_length, uint32_t item_length, uint32_t data_length, vrms_data_type_t type) {
    vrms_object_t* object = vrms_object_create();
    object->type = VRMS_OBJECT_DATA;
    object->realized = 0;

    vrms_object_data_t* object_data = SAFEMALLOC(sizeof(vrms_object_data_t));
    memset(object_data, 0, sizeof(vrms_object_data_t));

    object_data->memory_id = memory_id;
    object_data->memory_offset = memory_offset;
    object_data->memory_length = memory_length;
    object_data->item_length = item_length;
    object_data->data_length = data_length;
    object_data->type = type;
    object->object.object_data = object_data;

    if ((VRMS_TEXTURE == type) || (VRMS_MATRIX == type) || (VRMS_PROGRAM == type) || (VRMS_REGISTER == type)) {
        object->realized = 1;
    }

    return object;
}

vrms_object_t* vrms_object_texture_create(uint32_t memory_length, uint32_t width, uint32_t height, vrms_texture_format_t format, vrms_texture_type_t type) {
    vrms_object_t* object = vrms_object_create();
    object->type = VRMS_OBJECT_TEXTURE;
    object->realized = 0;

    vrms_object_texture_t* object_texture = SAFEMALLOC(sizeof(vrms_object_texture_t));
    memset(object_texture, 0, sizeof(vrms_object_texture_t));

    object_texture->memory_length = memory_length;
    object_texture->width = width;
    object_texture->height = height;
    object_texture->format = format;
    object_texture->type = type;
    object->object.object_texture = object_texture;

    return object;
}

void vrms_object_memory_destroy(vrms_object_memory_t* memory) {
    free(memory);
}

void vrms_object_data_destroy(vrms_object_data_t* data) {
    free(data);
}

void vrms_object_texture_destroy(vrms_object_texture_t* texture) {
    free(texture);
}

void vrms_object_matrix_destroy(vrms_object_matrix_t* matrix) {
    free(matrix->data);
    free(matrix);
}

vrms_object_t* vrms_object_create() {
    vrms_object_t* object = SAFEMALLOC(sizeof(vrms_object_t));
    memset(object, 0, sizeof(vrms_object_t));
    object->type = VRMS_OBJECT_INVALID;
    return object;
}
