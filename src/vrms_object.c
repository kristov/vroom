#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "safe_malloc.h"
#include "vrms.h"
#include "vrms_object.h"

vrms_object_t* vrms_object_create() {
    vrms_object_t* object = SAFEMALLOC(sizeof(vrms_object_t));
    memset(object, 0, sizeof(vrms_object_t));
    object->type = VRMS_OBJECT_INVALID;
    return object;
}

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

vrms_object_t* vrms_object_geometry_create(uint32_t vertex_id, uint32_t normal_id, uint32_t index_id) {
    vrms_object_t* object = vrms_object_create();
    object->type = VRMS_OBJECT_GEOMETRY;
    object->realized = 0;

    vrms_object_geometry_t* object_geometry = SAFEMALLOC(sizeof(vrms_object_geometry_t));
    memset(object_geometry, 0, sizeof(vrms_object_geometry_t));

    object_geometry->vertex_id = vertex_id;
    object_geometry->normal_id = normal_id;
    object_geometry->index_id = index_id;
    object->object.object_geometry = object_geometry;

    return object;
}

vrms_object_t* vrms_object_mesh_color_create(uint32_t geometry_id, float r, float g, float b, float a) {
    vrms_object_t* object = vrms_object_create();
    object->type = VRMS_OBJECT_MESH_COLOR;
    object->realized = 0;

    vrms_object_mesh_color_t* object_mesh_color = SAFEMALLOC(sizeof(vrms_object_mesh_color_t));
    memset(object_mesh_color, 0, sizeof(vrms_object_mesh_color_t));
    object_mesh_color->geometry_id = geometry_id;
    object_mesh_color->r = r;
    object_mesh_color->g = g;
    object_mesh_color->b = b;
    object_mesh_color->a = a;
    object->object.object_mesh_color = object_mesh_color;

    return object;
}

vrms_object_t* vrms_object_program_create(uint32_t program_length) {
    vrms_object_t* object = vrms_object_create();
    object->type = VRMS_OBJECT_PROGRAM;
    object->realized = 0;

    vrms_object_program_t* object_program = SAFEMALLOC(sizeof(vrms_object_program_t));
    memset(object_program, 0, sizeof(vrms_object_program_t));
    object_program->length = program_length;
    object_program->data = NULL;
    object->object.object_program = object_program;

    return object;
}

vrms_object_t* vrms_object_mesh_texture_create(uint32_t geometry_id, uint32_t texture_id, uint32_t uv_id) {
    vrms_object_t* object = vrms_object_create();
    object->type = VRMS_OBJECT_MESH_TEXTURE;
    object->realized = 0;

    vrms_object_mesh_texture_t* object_mesh_texture = SAFEMALLOC(sizeof(vrms_object_mesh_texture_t));
    memset(object_mesh_texture, 0, sizeof(vrms_object_mesh_texture_t));
    object_mesh_texture->geometry_id = geometry_id;
    object_mesh_texture->uv_id = uv_id;
    object_mesh_texture->texture_id = texture_id;
    object->object.object_mesh_texture = object_mesh_texture;

    return object;
}

vrms_object_t* vrms_object_skybox_create(uint32_t texture_id) {
    vrms_object_t* object = vrms_object_create();
    object->type = VRMS_OBJECT_SKYBOX;
    object->realized = 0;

    vrms_object_skybox_t* object_skybox = SAFEMALLOC(sizeof(vrms_object_skybox_t));
    memset(object_skybox, 0, sizeof(vrms_object_skybox_t));
    object_skybox->texture_id = texture_id;
    object->object.object_skybox = object_skybox;

    return object;
}

void vrms_object_memory_destroy(vrms_object_memory_t* memory) {
    free(memory);
}

void vrms_object_data_destroy(vrms_object_data_t* data) {
    free(data);
}

void vrms_object_geometry_destroy(vrms_object_geometry_t* geometry) {
    free(geometry);
}

void vrms_object_mesh_color_destroy(vrms_object_mesh_color_t* mesh_color) {
    free(mesh_color);
}

void vrms_object_mesh_texture_destroy(vrms_object_mesh_texture_t* mesh_texture) {
    free(mesh_texture);
}

void vrms_object_texture_destroy(vrms_object_texture_t* texture) {
    free(texture);
}

void vrms_object_program_destroy(vrms_object_program_t* program) {
    if (NULL != program->data) {
        free(program->data);
    }
    free(program);
}

void vrms_object_matrix_destroy(vrms_object_matrix_t* matrix) {
    free(matrix->data);
    free(matrix);
}

void vrms_object_skybox_destroy(vrms_object_skybox_t* skybox) {
    free(skybox);
}
