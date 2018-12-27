#ifndef VRMS_H
#define VRMS_H

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

typedef enum vrms_type {
    VRMS_REPLY,
    VRMS_CREATESCENE,
    VRMS_DESTROYSCENE,
    VRMS_CREATEMEMORY,
    VRMS_CREATEDATAOBJECT,
    VRMS_CREATETEXTUREOBJECT,
    VRMS_DESTROYOBJECT,
    VRMS_ATTACHMEMORY,
    VRMS_RUNPROGRAM,
    VRMS_SETSKYBOX
} vrms_type_t;

typedef enum vrms_matrix_type {
    VRMS_MATRIX_HEAD,
    VRMS_MATRIX_BODY
} vrms_matrix_type_t;

typedef enum vrms_update_type {
    VRMS_UPDATE_MULTIPLY,
    VRMS_UPDATE_SET
} vrms_update_type_t;

typedef enum vrms_error {
    VRMS_OK,
    VRMS_INVALIDREQUEST,
    VRMS_UNKNOWNID,
    VRMS_OUTOFMEMORY
} vrms_error_t;

#endif
