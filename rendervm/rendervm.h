#ifndef RENDERVM_H
#define RENDERVM_H

#include <stdint.h>

#define VM_MAX_ADDR      255
#define VM_STACK_SIZE    256

#define MEMORY_ATTACH_UINT8 0x01;
#define MEMORY_ATTACH_UINT16 0x02;
#define MEMORY_ATTACH_UINT32 0x03;
#define MEMORY_ATTACH_FLOAT 0x04;
#define MEMORY_ATTACH_VEC2 0x05;
#define MEMORY_ATTACH_VEC3 0x06;
#define MEMORY_ATTACH_VEC4 0x07;
#define MEMORY_ATTACH_MAT2 0x08;
#define MEMORY_ATTACH_MAT3 0x09;
#define MEMORY_ATTACH_MAT4 0x0a;

typedef enum rendervm_exception {
    VM_X_ALL_OK         = 0x00,
    VM_X_INV_OPCODE     = 0x01,
    VM_X_OUT_OF_BOUNDS  = 0x02
} rendervm_exception_t;

typedef struct rendervm rendervm_t;
typedef enum rendervm_opcode rendervm_opcode_t;

typedef void (*rendervm_callback_t)(rendervm_t* vm, rendervm_opcode_t opcode, void* user_data);

typedef struct rendervm {
    uint16_t pc;

    uint8_t flags;

    uint32_t draw_reg[10];

    uint16_t* ctrl_stack;
    uint8_t ctrl_sp;

    uint8_t* uint8_stack;
    uint8_t uint8_sp;

    uint16_t* uint16_stack;
    uint8_t uint16_sp;

    uint32_t* uint32_stack;
    uint8_t uint32_sp;

    float* float_stack;
    uint8_t float_sp;

    float* vec2_stack;
    uint8_t vec2_sp;

    float* vec3_stack;
    uint8_t vec3_sp;

    float* vec4_stack;
    uint8_t vec4_sp;

    float* mat2_stack;
    uint8_t mat2_sp;

    float* mat3_stack;
    uint8_t mat3_sp;

    float* mat4_stack;
    uint8_t mat4_sp;

    uint8_t b0;
    uint8_t b1;
    uint8_t b2;
    uint8_t b3;

    uint8_t* uint8_memory;
    uint32_t uint8_memory_size;

    uint16_t* uint16_memory;
    uint32_t uint16_memory_size;

    uint32_t* uint32_memory;
    uint32_t uint32_memory_size;

    float* float_memory;
    uint32_t float_memory_size;

    float* vec2_memory;
    uint32_t vec2_memory_size;

    float* vec3_memory;
    uint32_t vec3_memory_size;

    float* vec4_memory;
    uint32_t vec4_memory_size;

    float* mat2_memory;
    uint32_t mat2_memory_size;

    float* mat3_memory;
    uint32_t mat3_memory_size;

    float* mat4_memory;
    uint32_t mat4_memory_size;

    rendervm_exception_t exception;
    uint8_t running;
    uint8_t last_opcode;
    uint8_t next_opcode;

    void* user_data;
    rendervm_callback_t callback;
} rendervm_t;

typedef enum rendervm_opcode {
    VM_HALT = 0x00,
    VM_YIELD = 0x01,
    VM_RESET = 0x02,
    VM_CALL = 0x03,
    VM_RETURN = 0x04,
    VM_JUMP = 0x05,
    VM_UINT8_POP = 0x06,
    VM_UINT8_DUP = 0x07,
    VM_UINT8_SWAP = 0x08,
    VM_UINT8_JUMPEM = 0x09,
    VM_UINT8_STORE = 0x0a,
    VM_UINT8_LOAD = 0x0b,
    VM_UINT8_ADD = 0x0c,
    VM_UINT8_SUB = 0x0d,
    VM_UINT8_MUL = 0x0e,
    VM_UINT8_EQ = 0x0f,
    VM_UINT8_ADDN = 0x10,
    VM_UINT8_JUMPNZ = 0x11,
    VM_UINT8_JUMPZ = 0x12,
    VM_UINT8_PUSH = 0x13,
    VM_UINT16_POP = 0x14,
    VM_UINT16_DUP = 0x15,
    VM_UINT16_SWAP = 0x16,
    VM_UINT16_JUMPEM = 0x17,
    VM_UINT16_STORE = 0x18,
    VM_UINT16_LOAD = 0x19,
    VM_UINT16_ADD = 0x1a,
    VM_UINT16_SUB = 0x1b,
    VM_UINT16_MUL = 0x1c,
    VM_UINT16_EQ = 0x1d,
    VM_UINT16_ADDN = 0x1e,
    VM_UINT16_JUMPNZ = 0x1f,
    VM_UINT16_JUMPZ = 0x20,
    VM_UINT16_MOVE_UINT8 = 0x21,
    VM_UINT16_PUSH = 0x22,
    VM_UINT32_POP = 0x23,
    VM_UINT32_DUP = 0x24,
    VM_UINT32_SWAP = 0x25,
    VM_UINT32_JUMPEM = 0x26,
    VM_UINT32_STORE = 0x27,
    VM_UINT32_LOAD = 0x28,
    VM_UINT32_ADD = 0x29,
    VM_UINT32_SUB = 0x2a,
    VM_UINT32_MUL = 0x2b,
    VM_UINT32_EQ = 0x2c,
    VM_UINT32_ADDN = 0x2d,
    VM_UINT32_JUMPNZ = 0x2e,
    VM_UINT32_JUMPZ = 0x2f,
    VM_UINT32_MOVE_UINT8 = 0x30,
    VM_UINT32_PUSH = 0x31,
    VM_UINT32_REG_GET = 0x32,
    VM_UINT32_REG_SET = 0x33,
    VM_FLOAT_POP = 0x34,
    VM_FLOAT_DUP = 0x35,
    VM_FLOAT_SWAP = 0x36,
    VM_FLOAT_JUMPEM = 0x37,
    VM_FLOAT_STORE = 0x38,
    VM_FLOAT_LOAD = 0x39,
    VM_FLOAT_ADD = 0x3a,
    VM_FLOAT_SUB = 0x3b,
    VM_FLOAT_MUL = 0x3c,
    VM_FLOAT_EQ = 0x3d,
    VM_FLOAT_ADDN = 0x3e,
    VM_FLOAT_JUMPNZ = 0x3f,
    VM_FLOAT_JUMPZ = 0x40,
    VM_FLOAT_PUSH = 0x41,
    VM_VEC2_POP = 0x42,
    VM_VEC2_DUP = 0x43,
    VM_VEC2_SWAP = 0x44,
    VM_VEC2_JUMPEM = 0x45,
    VM_VEC2_STORE = 0x46,
    VM_VEC2_LOAD = 0x47,
    VM_VEC2_ADD = 0x48,
    VM_VEC2_SUB = 0x49,
    VM_VEC2_MUL = 0x4a,
    VM_VEC2_EQ = 0x4b,
    VM_VEC2_EXPLODE = 0x4c,
    VM_VEC2_IMPLODE = 0x4d,
    VM_VEC2_MULMAT2 = 0x4e,
    VM_VEC2_MULMAT3 = 0x4f,
    VM_VEC2_MULMAT4 = 0x50,
    VM_VEC3_POP = 0x51,
    VM_VEC3_DUP = 0x52,
    VM_VEC3_SWAP = 0x53,
    VM_VEC3_JUMPEM = 0x54,
    VM_VEC3_STORE = 0x55,
    VM_VEC3_LOAD = 0x56,
    VM_VEC3_ADD = 0x57,
    VM_VEC3_SUB = 0x58,
    VM_VEC3_MUL = 0x59,
    VM_VEC3_EQ = 0x5a,
    VM_VEC3_EXPLODE = 0x5b,
    VM_VEC3_IMPLODE = 0x5c,
    VM_VEC3_MULMAT3 = 0x5d,
    VM_VEC3_MULMAT4 = 0x5e,
    VM_VEC4_POP = 0x5f,
    VM_VEC4_DUP = 0x60,
    VM_VEC4_SWAP = 0x61,
    VM_VEC4_JUMPEM = 0x62,
    VM_VEC4_STORE = 0x63,
    VM_VEC4_LOAD = 0x64,
    VM_VEC4_ADD = 0x65,
    VM_VEC4_SUB = 0x66,
    VM_VEC4_MUL = 0x67,
    VM_VEC4_EQ = 0x68,
    VM_VEC4_EXPLODE = 0x69,
    VM_VEC4_IMPLODE = 0x6a,
    VM_VEC4_MULMAT4 = 0x6b,
    VM_MAT2_POP = 0x6c,
    VM_MAT2_DUP = 0x6d,
    VM_MAT2_SWAP = 0x6e,
    VM_MAT2_JUMPEM = 0x6f,
    VM_MAT2_STORE = 0x70,
    VM_MAT2_LOAD = 0x71,
    VM_MAT2_ADD = 0x72,
    VM_MAT2_SUB = 0x73,
    VM_MAT2_MUL = 0x74,
    VM_MAT2_EQ = 0x75,
    VM_MAT2_EXPLODE = 0x76,
    VM_MAT2_IDENT = 0x77,
    VM_MAT2_IMPLODE = 0x78,
    VM_MAT2_ROTATE = 0x79,
    VM_MAT2_SCALE = 0x7a,
    VM_MAT2_TRANSP = 0x7b,
    VM_MAT3_POP = 0x7c,
    VM_MAT3_DUP = 0x7d,
    VM_MAT3_SWAP = 0x7e,
    VM_MAT3_JUMPEM = 0x7f,
    VM_MAT3_STORE = 0x80,
    VM_MAT3_LOAD = 0x81,
    VM_MAT3_ADD = 0x82,
    VM_MAT3_SUB = 0x83,
    VM_MAT3_MUL = 0x84,
    VM_MAT3_EQ = 0x85,
    VM_MAT3_EXPLODE = 0x86,
    VM_MAT3_IDENT = 0x87,
    VM_MAT3_IMPLODE = 0x88,
    VM_MAT3_ROTATE = 0x89,
    VM_MAT3_SCALE = 0x8a,
    VM_MAT3_TRANSL = 0x8b,
    VM_MAT3_TRANSP = 0x8c,
    VM_MAT4_POP = 0x8d,
    VM_MAT4_DUP = 0x8e,
    VM_MAT4_SWAP = 0x8f,
    VM_MAT4_JUMPEM = 0x90,
    VM_MAT4_STORE = 0x91,
    VM_MAT4_LOAD = 0x92,
    VM_MAT4_ADD = 0x93,
    VM_MAT4_SUB = 0x94,
    VM_MAT4_MUL = 0x95,
    VM_MAT4_EQ = 0x96,
    VM_MAT4_EXPLODE = 0x97,
    VM_MAT4_IDENT = 0x98,
    VM_MAT4_IMPLODE = 0x99,
    VM_MAT4_ROTATEX = 0x9a,
    VM_MAT4_ROTATEY = 0x9b,
    VM_MAT4_ROTATEZ = 0x9c,
    VM_MAT4_SCALE = 0x9d,
    VM_MAT4_TRANSL = 0x9e,
    VM_MAT4_TRANSP = 0x9f,
    VM__INSEND = 0xff
} rendervm_opcode_t;

rendervm_t* rendervm_create();

void rendervm_destroy(rendervm_t* vm);

uint8_t rendervm_last_opcode(rendervm_t* vm);

uint8_t rendervm_next_opcode(rendervm_t* vm);

//uint8_t rendervm_resume(rendervm_t* vm);

void rendervm_reset(rendervm_t* vm);

uint8_t rendervm_exec(rendervm_t* vm, uint8_t* program, uint16_t length);

uint8_t rendervm_opcode2arglen(rendervm_opcode_t opcode);

const char* rendervm_opcode2str(rendervm_opcode_t opcode);

//void rendervm_interrupt(rendervm_t* vm);

uint8_t rendervm_has_exception(rendervm_t* vm);

void rendervm_exception(rendervm_t* vm, rendervm_exception_t exception);

void rendervm_attach_callback(rendervm_t* vm, rendervm_callback_t callback, void* user_data);

void rendervm_memory_detach_uint8(rendervm_t* vm, uint8_t* mem);

void rendervm_memory_detach_uint16(rendervm_t* vm, uint16_t* mem);

void rendervm_memory_detach_uint32(rendervm_t* vm, uint32_t* mem);

void rendervm_memory_detach_float(rendervm_t* vm, float* mem);

void rendervm_memory_detach_vec2(rendervm_t* vm, float* mem);

void rendervm_memory_detach_vec3(rendervm_t* vm, float* mem);

void rendervm_memory_detach_vec4(rendervm_t* vm, float* mem);

void rendervm_memory_detach_mat2(rendervm_t* vm, float* mem);

void rendervm_memory_detach_mat3(rendervm_t* vm, float* mem);

void rendervm_memory_detach_mat4(rendervm_t* vm, float* mem);

void rendervm_memory_attach_uint8(rendervm_t* vm, uint8_t* mem);

void rendervm_memory_attach_uint16(rendervm_t* vm, uint16_t* mem);

void rendervm_memory_attach_uint32(rendervm_t* vm, uint32_t* mem);

void rendervm_memory_attach_float(rendervm_t* vm, float* mem);

void rendervm_memory_attach_vec2(rendervm_t* vm, float* mem);

void rendervm_memory_attach_vec3(rendervm_t* vm, float* mem);

void rendervm_memory_attach_vec4(rendervm_t* vm, float* mem);

void rendervm_memory_attach_mat2(rendervm_t* vm, float* mem);

void rendervm_memory_attach_mat3(rendervm_t* vm, float* mem);

void rendervm_memory_attach_mat4(rendervm_t* vm, float* mem);

#endif
