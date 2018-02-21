/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: vroom.proto */

#ifndef PROTOBUF_C_vroom_2eproto__INCLUDED
#define PROTOBUF_C_vroom_2eproto__INCLUDED

#include <protobuf-c/protobuf-c.h>

PROTOBUF_C__BEGIN_DECLS

#if PROTOBUF_C_VERSION_NUMBER < 1000000
# error This file was generated by a newer version of protoc-c which is incompatible with your libprotobuf-c headers. Please update your headers.
#elif 1002001 < PROTOBUF_C_MIN_COMPILER_VERSION
# error This file was generated by an older version of protoc-c which is incompatible with your libprotobuf-c headers. Please regenerate this file with a newer version of protoc-c.
#endif


typedef struct _Reply Reply;
typedef struct _CreateScene CreateScene;
typedef struct _DestroyScene DestroyScene;
typedef struct _CreateMemory CreateMemory;
typedef struct _CreateDataObject CreateDataObject;
typedef struct _CreateTextureObject CreateTextureObject;
typedef struct _SetRenderBuffer SetRenderBuffer;
typedef struct _UpdateSystemMatrix UpdateSystemMatrix;
typedef struct _DestroyDataObject DestroyDataObject;
typedef struct _CreateGeometryObject CreateGeometryObject;
typedef struct _CreateMeshColor CreateMeshColor;
typedef struct _CreateMeshTexture CreateMeshTexture;


/* --- enums --- */

typedef enum _CreateDataObject__Type {
  CREATE_DATA_OBJECT__TYPE__UV = 0,
  CREATE_DATA_OBJECT__TYPE__COLOR = 1,
  CREATE_DATA_OBJECT__TYPE__VERTEX = 2,
  CREATE_DATA_OBJECT__TYPE__NORMAL = 3,
  CREATE_DATA_OBJECT__TYPE__INDEX = 4,
  CREATE_DATA_OBJECT__TYPE__MATRIX = 5
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(CREATE_DATA_OBJECT__TYPE)
} CreateDataObject__Type;
typedef enum _CreateTextureObject__Format {
  CREATE_TEXTURE_OBJECT__FORMAT__RGB8 = 0
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(CREATE_TEXTURE_OBJECT__FORMAT)
} CreateTextureObject__Format;
typedef enum _CreateTextureObject__Type {
  CREATE_TEXTURE_OBJECT__TYPE__TEXTURE_2D = 0,
  CREATE_TEXTURE_OBJECT__TYPE__TEXTURE_CUBE_MAP = 1
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(CREATE_TEXTURE_OBJECT__TYPE)
} CreateTextureObject__Type;
typedef enum _UpdateSystemMatrix__MatrixType {
  UPDATE_SYSTEM_MATRIX__MATRIX_TYPE__HEAD = 0,
  UPDATE_SYSTEM_MATRIX__MATRIX_TYPE__BODY = 1
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(UPDATE_SYSTEM_MATRIX__MATRIX_TYPE)
} UpdateSystemMatrix__MatrixType;
typedef enum _UpdateSystemMatrix__UpdateType {
  UPDATE_SYSTEM_MATRIX__UPDATE_TYPE__MULTIPLY = 0,
  UPDATE_SYSTEM_MATRIX__UPDATE_TYPE__SET = 1
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(UPDATE_SYSTEM_MATRIX__UPDATE_TYPE)
} UpdateSystemMatrix__UpdateType;

/* --- messages --- */

struct  _Reply
{
  ProtobufCMessage base;
  int32_t id;
  int32_t error_code;
};
#define REPLY__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&reply__descriptor) \
    , 0, 0 }


struct  _CreateScene
{
  ProtobufCMessage base;
  char *name;
};
#define CREATE_SCENE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&create_scene__descriptor) \
    , NULL }


struct  _DestroyScene
{
  ProtobufCMessage base;
  int32_t id;
};
#define DESTROY_SCENE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&destroy_scene__descriptor) \
    , 0 }


struct  _CreateMemory
{
  ProtobufCMessage base;
  int32_t scene_id;
  int32_t size;
};
#define CREATE_MEMORY__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&create_memory__descriptor) \
    , 0, 0 }


struct  _CreateDataObject
{
  ProtobufCMessage base;
  int32_t scene_id;
  int32_t memory_id;
  CreateDataObject__Type type;
  int32_t memory_offset;
  int32_t memory_length;
  int32_t value_length;
};
#define CREATE_DATA_OBJECT__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&create_data_object__descriptor) \
    , 0, 0, 0, 0, 0, 0 }


struct  _CreateTextureObject
{
  ProtobufCMessage base;
  int32_t scene_id;
  int32_t memory_id;
  int32_t memory_offset;
  int32_t memory_length;
  int32_t width;
  int32_t height;
  CreateTextureObject__Format format;
  CreateTextureObject__Type type;
};
#define CREATE_TEXTURE_OBJECT__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&create_texture_object__descriptor) \
    , 0, 0, 0, 0, 0, 0, 0, 0 }


struct  _SetRenderBuffer
{
  ProtobufCMessage base;
  int32_t scene_id;
  int32_t memory_id;
  int32_t nr_objects;
};
#define SET_RENDER_BUFFER__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&set_render_buffer__descriptor) \
    , 0, 0, 0 }


struct  _UpdateSystemMatrix
{
  ProtobufCMessage base;
  int32_t scene_id;
  int32_t memory_id;
  UpdateSystemMatrix__MatrixType matrix_type;
  UpdateSystemMatrix__UpdateType update_type;
  int32_t offset;
  int32_t size;
};
#define UPDATE_SYSTEM_MATRIX__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&update_system_matrix__descriptor) \
    , 0, 0, 0, 0, 0, 0 }


struct  _DestroyDataObject
{
  ProtobufCMessage base;
  int32_t scene_id;
  int32_t id;
};
#define DESTROY_DATA_OBJECT__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&destroy_data_object__descriptor) \
    , 0, 0 }


struct  _CreateGeometryObject
{
  ProtobufCMessage base;
  int32_t scene_id;
  int32_t vertex_id;
  int32_t normal_id;
  int32_t index_id;
};
#define CREATE_GEOMETRY_OBJECT__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&create_geometry_object__descriptor) \
    , 0, 0, 0, 0 }


struct  _CreateMeshColor
{
  ProtobufCMessage base;
  int32_t scene_id;
  int32_t geometry_id;
  float r;
  float g;
  float b;
  float a;
};
#define CREATE_MESH_COLOR__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&create_mesh_color__descriptor) \
    , 0, 0, 0, 0, 0, 0 }


struct  _CreateMeshTexture
{
  ProtobufCMessage base;
  int32_t scene_id;
  int32_t geometry_id;
  int32_t texture_id;
  int32_t uv_id;
};
#define CREATE_MESH_TEXTURE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&create_mesh_texture__descriptor) \
    , 0, 0, 0, 0 }


/* Reply methods */
void   reply__init
                     (Reply         *message);
size_t reply__get_packed_size
                     (const Reply   *message);
size_t reply__pack
                     (const Reply   *message,
                      uint8_t             *out);
size_t reply__pack_to_buffer
                     (const Reply   *message,
                      ProtobufCBuffer     *buffer);
Reply *
       reply__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   reply__free_unpacked
                     (Reply *message,
                      ProtobufCAllocator *allocator);
/* CreateScene methods */
void   create_scene__init
                     (CreateScene         *message);
size_t create_scene__get_packed_size
                     (const CreateScene   *message);
size_t create_scene__pack
                     (const CreateScene   *message,
                      uint8_t             *out);
size_t create_scene__pack_to_buffer
                     (const CreateScene   *message,
                      ProtobufCBuffer     *buffer);
CreateScene *
       create_scene__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   create_scene__free_unpacked
                     (CreateScene *message,
                      ProtobufCAllocator *allocator);
/* DestroyScene methods */
void   destroy_scene__init
                     (DestroyScene         *message);
size_t destroy_scene__get_packed_size
                     (const DestroyScene   *message);
size_t destroy_scene__pack
                     (const DestroyScene   *message,
                      uint8_t             *out);
size_t destroy_scene__pack_to_buffer
                     (const DestroyScene   *message,
                      ProtobufCBuffer     *buffer);
DestroyScene *
       destroy_scene__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   destroy_scene__free_unpacked
                     (DestroyScene *message,
                      ProtobufCAllocator *allocator);
/* CreateMemory methods */
void   create_memory__init
                     (CreateMemory         *message);
size_t create_memory__get_packed_size
                     (const CreateMemory   *message);
size_t create_memory__pack
                     (const CreateMemory   *message,
                      uint8_t             *out);
size_t create_memory__pack_to_buffer
                     (const CreateMemory   *message,
                      ProtobufCBuffer     *buffer);
CreateMemory *
       create_memory__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   create_memory__free_unpacked
                     (CreateMemory *message,
                      ProtobufCAllocator *allocator);
/* CreateDataObject methods */
void   create_data_object__init
                     (CreateDataObject         *message);
size_t create_data_object__get_packed_size
                     (const CreateDataObject   *message);
size_t create_data_object__pack
                     (const CreateDataObject   *message,
                      uint8_t             *out);
size_t create_data_object__pack_to_buffer
                     (const CreateDataObject   *message,
                      ProtobufCBuffer     *buffer);
CreateDataObject *
       create_data_object__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   create_data_object__free_unpacked
                     (CreateDataObject *message,
                      ProtobufCAllocator *allocator);
/* CreateTextureObject methods */
void   create_texture_object__init
                     (CreateTextureObject         *message);
size_t create_texture_object__get_packed_size
                     (const CreateTextureObject   *message);
size_t create_texture_object__pack
                     (const CreateTextureObject   *message,
                      uint8_t             *out);
size_t create_texture_object__pack_to_buffer
                     (const CreateTextureObject   *message,
                      ProtobufCBuffer     *buffer);
CreateTextureObject *
       create_texture_object__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   create_texture_object__free_unpacked
                     (CreateTextureObject *message,
                      ProtobufCAllocator *allocator);
/* SetRenderBuffer methods */
void   set_render_buffer__init
                     (SetRenderBuffer         *message);
size_t set_render_buffer__get_packed_size
                     (const SetRenderBuffer   *message);
size_t set_render_buffer__pack
                     (const SetRenderBuffer   *message,
                      uint8_t             *out);
size_t set_render_buffer__pack_to_buffer
                     (const SetRenderBuffer   *message,
                      ProtobufCBuffer     *buffer);
SetRenderBuffer *
       set_render_buffer__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   set_render_buffer__free_unpacked
                     (SetRenderBuffer *message,
                      ProtobufCAllocator *allocator);
/* UpdateSystemMatrix methods */
void   update_system_matrix__init
                     (UpdateSystemMatrix         *message);
size_t update_system_matrix__get_packed_size
                     (const UpdateSystemMatrix   *message);
size_t update_system_matrix__pack
                     (const UpdateSystemMatrix   *message,
                      uint8_t             *out);
size_t update_system_matrix__pack_to_buffer
                     (const UpdateSystemMatrix   *message,
                      ProtobufCBuffer     *buffer);
UpdateSystemMatrix *
       update_system_matrix__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   update_system_matrix__free_unpacked
                     (UpdateSystemMatrix *message,
                      ProtobufCAllocator *allocator);
/* DestroyDataObject methods */
void   destroy_data_object__init
                     (DestroyDataObject         *message);
size_t destroy_data_object__get_packed_size
                     (const DestroyDataObject   *message);
size_t destroy_data_object__pack
                     (const DestroyDataObject   *message,
                      uint8_t             *out);
size_t destroy_data_object__pack_to_buffer
                     (const DestroyDataObject   *message,
                      ProtobufCBuffer     *buffer);
DestroyDataObject *
       destroy_data_object__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   destroy_data_object__free_unpacked
                     (DestroyDataObject *message,
                      ProtobufCAllocator *allocator);
/* CreateGeometryObject methods */
void   create_geometry_object__init
                     (CreateGeometryObject         *message);
size_t create_geometry_object__get_packed_size
                     (const CreateGeometryObject   *message);
size_t create_geometry_object__pack
                     (const CreateGeometryObject   *message,
                      uint8_t             *out);
size_t create_geometry_object__pack_to_buffer
                     (const CreateGeometryObject   *message,
                      ProtobufCBuffer     *buffer);
CreateGeometryObject *
       create_geometry_object__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   create_geometry_object__free_unpacked
                     (CreateGeometryObject *message,
                      ProtobufCAllocator *allocator);
/* CreateMeshColor methods */
void   create_mesh_color__init
                     (CreateMeshColor         *message);
size_t create_mesh_color__get_packed_size
                     (const CreateMeshColor   *message);
size_t create_mesh_color__pack
                     (const CreateMeshColor   *message,
                      uint8_t             *out);
size_t create_mesh_color__pack_to_buffer
                     (const CreateMeshColor   *message,
                      ProtobufCBuffer     *buffer);
CreateMeshColor *
       create_mesh_color__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   create_mesh_color__free_unpacked
                     (CreateMeshColor *message,
                      ProtobufCAllocator *allocator);
/* CreateMeshTexture methods */
void   create_mesh_texture__init
                     (CreateMeshTexture         *message);
size_t create_mesh_texture__get_packed_size
                     (const CreateMeshTexture   *message);
size_t create_mesh_texture__pack
                     (const CreateMeshTexture   *message,
                      uint8_t             *out);
size_t create_mesh_texture__pack_to_buffer
                     (const CreateMeshTexture   *message,
                      ProtobufCBuffer     *buffer);
CreateMeshTexture *
       create_mesh_texture__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   create_mesh_texture__free_unpacked
                     (CreateMeshTexture *message,
                      ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*Reply_Closure)
                 (const Reply *message,
                  void *closure_data);
typedef void (*CreateScene_Closure)
                 (const CreateScene *message,
                  void *closure_data);
typedef void (*DestroyScene_Closure)
                 (const DestroyScene *message,
                  void *closure_data);
typedef void (*CreateMemory_Closure)
                 (const CreateMemory *message,
                  void *closure_data);
typedef void (*CreateDataObject_Closure)
                 (const CreateDataObject *message,
                  void *closure_data);
typedef void (*CreateTextureObject_Closure)
                 (const CreateTextureObject *message,
                  void *closure_data);
typedef void (*SetRenderBuffer_Closure)
                 (const SetRenderBuffer *message,
                  void *closure_data);
typedef void (*UpdateSystemMatrix_Closure)
                 (const UpdateSystemMatrix *message,
                  void *closure_data);
typedef void (*DestroyDataObject_Closure)
                 (const DestroyDataObject *message,
                  void *closure_data);
typedef void (*CreateGeometryObject_Closure)
                 (const CreateGeometryObject *message,
                  void *closure_data);
typedef void (*CreateMeshColor_Closure)
                 (const CreateMeshColor *message,
                  void *closure_data);
typedef void (*CreateMeshTexture_Closure)
                 (const CreateMeshTexture *message,
                  void *closure_data);

/* --- services --- */


/* --- descriptors --- */

extern const ProtobufCMessageDescriptor reply__descriptor;
extern const ProtobufCMessageDescriptor create_scene__descriptor;
extern const ProtobufCMessageDescriptor destroy_scene__descriptor;
extern const ProtobufCMessageDescriptor create_memory__descriptor;
extern const ProtobufCMessageDescriptor create_data_object__descriptor;
extern const ProtobufCEnumDescriptor    create_data_object__type__descriptor;
extern const ProtobufCMessageDescriptor create_texture_object__descriptor;
extern const ProtobufCEnumDescriptor    create_texture_object__format__descriptor;
extern const ProtobufCEnumDescriptor    create_texture_object__type__descriptor;
extern const ProtobufCMessageDescriptor set_render_buffer__descriptor;
extern const ProtobufCMessageDescriptor update_system_matrix__descriptor;
extern const ProtobufCEnumDescriptor    update_system_matrix__matrix_type__descriptor;
extern const ProtobufCEnumDescriptor    update_system_matrix__update_type__descriptor;
extern const ProtobufCMessageDescriptor destroy_data_object__descriptor;
extern const ProtobufCMessageDescriptor create_geometry_object__descriptor;
extern const ProtobufCMessageDescriptor create_mesh_color__descriptor;
extern const ProtobufCMessageDescriptor create_mesh_texture__descriptor;

PROTOBUF_C__END_DECLS


#endif  /* PROTOBUF_C_vroom_2eproto__INCLUDED */
