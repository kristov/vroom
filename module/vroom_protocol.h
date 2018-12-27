#ifndef VROOM_PROTOCOL_H
#define VROOM_PROTOCOL_H

typedef enum vroom_protocol_type {
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
} vroom_protocol_type_t;

typedef enum vroom_protocol_error {
    VRMS_OK,
    VRMS_INVALIDREQUEST,
    VRMS_UNKNOWNID,
    VRMS_OUTOFMEMORY
} vroom_protocol_error_t;

#endif
