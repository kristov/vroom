#ifndef VRMS_H
#define VRMS_H

#define SIZEOF_UINT8 sizeof(uint8_t)
#define SIZEOF_UINT16 sizeof(uint16_t)
#define SIZEOF_UINT32 sizeof(uint32_t)
#define SIZEOF_FLOAT sizeof(float)
#define SIZEOF_VEC2 (SIZEOF_FLOAT * 2)
#define SIZEOF_VEC3 (SIZEOF_FLOAT * 3)
#define SIZEOF_VEC4 (SIZEOF_FLOAT * 4)
#define SIZEOF_MAT2 (SIZEOF_FLOAT * 4)
#define SIZEOF_MAT3 (SIZEOF_FLOAT * 9)
#define SIZEOF_MAT4 (SIZEOF_FLOAT * 16)

typedef enum vrms_object_type {
    VRMS_OBJECT_INVALID,
    VRMS_OBJECT_SCENE,
    VRMS_OBJECT_MEMORY,
    VRMS_OBJECT_DATA,
    VRMS_OBJECT_TEXTURE
} vrms_object_type_t;

typedef enum vrms_data_type {
    VRMS_UINT8,
    VRMS_UINT16,
    VRMS_UINT32,
    VRMS_FLOAT,
    VRMS_VEC2,
    VRMS_VEC3,
    VRMS_VEC4,
    VRMS_MAT2,
    VRMS_MAT3,
    VRMS_MAT4
} vrms_data_type_t;

typedef enum vrms_texture_format {
    VRMS_FORMAT_BGR888,
    VRMS_FORMAT_XBGR8888,
    VRMS_FORMAT_ABGR8888,
    VRMS_FORMAT_RGB888,
    VRMS_FORMAT_XRGB8888,
    VRMS_FORMAT_ARGB8888
} vrms_texture_format_t;

typedef enum vrms_texture_type {
    VRMS_TEXTURE_2D,
    VRMS_TEXTURE_CUBE_MAP
} vrms_texture_type_t;

typedef enum vrms_matrix_type {
    VRMS_MATRIX_HEAD,
    VRMS_MATRIX_BODY
} vrms_matrix_type_t;

typedef enum vrms_update_type {
    VRMS_UPDATE_MULTIPLY,
    VRMS_UPDATE_SET
} vrms_update_type_t;

#endif
