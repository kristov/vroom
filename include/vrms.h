typedef enum vrms_type {
    VRMS_REPLY,
    VRMS_CREATESCENE,
    VRMS_DESTROYSCENE,
    VRMS_CREATEDATAOBJECT,
    VRMS_DESTROYDATAOBJECT,
    VRMS_CREATEGEOMETRYOBJECT,
    VRMS_CREATECOLORMESH,
    VRMS_CREATETEXTUREMESH,
} vrms_type_t;

typedef enum vrms_data_type {
    VRMS_UV,
    VRMS_COLOR,
    VRMS_TEXTURE,
    VRMS_VERTEX,
    VRMS_NORMAL,
    VRMS_INDEX
} vrms_data_type_t;

typedef enum vrms_error {
    VRMS_OK,
    VRMS_INVALIDREQUEST,
    VRMS_UNKNOWNID,
    VRMS_OUTOFMEMORY
} vrms_error_t;
