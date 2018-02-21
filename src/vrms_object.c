#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "safe_malloc.h"
#include "vrms.h"
#include "vrms_object.h"

vrms_object_t* vrms_object_create() {
    vrms_object_t* vrms_object = SAFEMALLOC(sizeof(vrms_object_t));
    vrms_object->type = VRMS_OBJECT_INVALID;
    return vrms_object;
}

vrms_object_t* vrms_object_memory_create(void* address, uint32_t size) {
    vrms_object_t* object = vrms_object_create();
    object->type = VRMS_OBJECT_MEMORY;

    vrms_object_memory_t* object_memory = SAFEMALLOC(sizeof(vrms_object_memory_t));
    memset(object_memory, 0, sizeof(vrms_object_memory_t));

    object_memory->address = address;
    object_memory->size = size;
    object->object.object_memory = object_memory;

    return object;
}

vrms_object_t* vrms_object_data_create(vrms_data_type_t type, uint32_t size, uint32_t nr_strides, uint32_t stride) {
    vrms_object_t* object = vrms_object_create();
    object->type = VRMS_OBJECT_DATA;

    vrms_object_data_t* object_data = SAFEMALLOC(sizeof(vrms_object_data_t));
    memset(object_data, 0, sizeof(vrms_object_data_t));

    object_data->type = type;
    object_data->size = size;
    object_data->nr_strides = nr_strides;
    object_data->stride = stride;
    object->object.object_data = object_data;

    return object;
}

vrms_object_t* vrms_object_texture_create(uint32_t size, uint32_t width, uint32_t height, vrms_texture_format_t format, vrms_texture_type_t type) {
    vrms_object_t* object = vrms_object_create();
    object->type = VRMS_OBJECT_TEXTURE;

    vrms_object_texture_t* object_texture = SAFEMALLOC(sizeof(vrms_object_texture_t));
    memset(object_texture, 0, sizeof(vrms_object_texture_t));

    object_texture->size = size;
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

vrms_object_t* vrms_object_mesh_texture_create(uint32_t geometry_id, uint32_t texture_id, uint32_t uv_id) {
    vrms_object_t* object = vrms_object_create();
    object->type = VRMS_OBJECT_MESH_TEXTURE;

    vrms_object_mesh_texture_t* object_mesh_texture = SAFEMALLOC(sizeof(vrms_object_mesh_texture_t));
    memset(object_mesh_texture, 0, sizeof(vrms_object_mesh_texture_t));
    object_mesh_texture->geometry_id = geometry_id;
    object_mesh_texture->uv_id = uv_id;
    object_mesh_texture->texture_id = texture_id;
    object->object.object_mesh_texture = object_mesh_texture;

    return object;
}

void vrms_object_data_destroy(vrms_object_data_t* data) {
    if (NULL != data->local_storage) {
        free(data->local_storage);
    }
    if (0 < data->gl_id) {
        glDeleteBuffers(1, &data->gl_id);
    }
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

void vrms_object_matrix_destroy(vrms_object_matrix_t* matrix) {
    free(matrix->data);
    free(matrix);
}

void vrms_object_destroy(vrms_object_t* object) {
    if (VRMS_OBJECT_DATA == object->type) {
        vrms_object_data_destroy(object->object.object_data);
    }
    else if (VRMS_OBJECT_GEOMETRY == object->type) {
        vrms_object_geometry_destroy(object->object.object_geometry);
    }
    else if (VRMS_OBJECT_MESH_COLOR == object->type) {
        vrms_object_mesh_color_destroy(object->object.object_mesh_color);
    }
    else if (VRMS_OBJECT_MESH_TEXTURE == object->type) {
        vrms_object_mesh_texture_destroy(object->object.object_mesh_texture);
    }
}
