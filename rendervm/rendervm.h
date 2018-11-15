#ifndef RENDERVM_H
#define RENDERVM_H

#include <stdint.h>

#define VM_MAX_ADDR      65535
#define VM_STACK_SIZE    65536
#define VM_MAX_NR_FLOAT  16384
#define VM_MAX_NR_INT32  16384
#define VM_MAX_NR_INT16  32768
#define VM_MAX_NR_INT8   65536

typedef struct rendervm {
    uint16_t pc;

    uint16_t* ctrl_stack;
    uint16_t ctrl_sp;

    uint8_t* uint8_stack;
    uint16_t uint8_sp;

    uint16_t* uint16_stack;
    uint16_t uint16_sp;

    uint32_t* uint32_stack;
    uint16_t uint32_sp;

    float* float_stack;
    uint16_t float_sp;

    uint8_t b0;
    uint8_t b1;
    uint8_t b2;
    uint8_t b3;

    uint8_t* uint8_memory;
    uint16_t* uint16_memory;
    uint32_t* uint32_memory;
    float* float_memory;

    uint8_t running;
    uint8_t last_opcode;
} rendervm_t;

typedef enum rendervm_opcode {
    VM_HALT = 0x00,
    VM_YIELD = 0x01,
    VM_RESET = 0x02,
    VM_CALL = 0x03,
    VM_RETURN = 0x04,
    VM_JUMP = 0x05,
    VM_UINT8_PUSH = 0x10,
    VM_UINT8_POP = 0x11,
    VM_UINT8_DUP = 0x12,
    VM_UINT8_SWAP = 0x13,
    VM_UINT8_STORE = 0x14,
    VM_UINT8_STORE_UINT16 = 0x15,
    VM_UINT8_LOAD = 0x16,
    VM_UINT8_LOAD_UINT16 = 0x17,
    VM_UINT8_MOVE_UINT8 = 0x18,
    VM_UINT8_COPY_UINT8 = 0x19,
    VM_UINT8_JUMP_EMPTY = 0x1a,
    VM_UINT8_JUMP_ZERO = 0x1b,
    VM_UINT8_JUMP_NZERO = 0x1c,
    VM_UINT8_ADD = 0x1d,
    VM_UINT8_SUBTRACT = 0x1e,
    VM_UINT8_MULTIPLY = 0x1f,
    VM_UINT8_DIVIDE_FLOAT = 0x20,
    VM_UINT8_EQ = 0x21,
    VM_UINT16_PUSH = 0x30,
    VM_UINT16_POP = 0x31,
    VM_UINT16_DUP = 0x32,
    VM_UINT16_SWAP = 0x33,
    VM_UINT16_STORE = 0x34,
    VM_UINT16_STORE_UINT16 = 0x35,
    VM_UINT16_LOAD = 0x36,
    VM_UINT16_LOAD_UINT16 = 0x37,
    VM_UINT16_MOVE_UINT8 = 0x38,
    VM_UINT16_COPY_UINT8 = 0x39,
    VM_UINT16_JUMP_EMPTY = 0x3a,
    VM_UINT16_JUMP_ZERO = 0x3b,
    VM_UINT16_JUMP_NZERO = 0x3c,
    VM_UINT16_ADD = 0x3d,
    VM_UINT16_SUBTRACT = 0x3e,
    VM_UINT16_MULTIPLY = 0x3f,
    VM_UINT16_DIVIDE_FLOAT = 0x40,
    VM_UINT16_EQ = 0x41,
    VM_UINT32_PUSH = 0x50,
    VM_UINT32_POP = 0x51,
    VM_UINT32_DUP = 0x52,
    VM_UINT32_SWAP = 0x53,
    VM_UINT32_STORE = 0x54,
    VM_UINT32_STORE_UINT16 = 0x55,
    VM_UINT32_LOAD = 0x56,
    VM_UINT32_LOAD_UINT16 = 0x57,
    VM_UINT32_MOVE_UINT8 = 0x58,
    VM_UINT32_COPY_UINT8 = 0x59,
    VM_UINT32_JUMP_EMPTY = 0x5a,
    VM_UINT32_JUMP_ZERO = 0x5b,
    VM_UINT32_JUMP_NZERO = 0x5c,
    VM_UINT32_ADD = 0x5d,
    VM_UINT32_SUBTRACT = 0x5e,
    VM_UINT32_MULTIPLY = 0x5f,
    VM_UINT32_DIVIDE_FLOAT = 0x60,
    VM_UINT32_EQ = 0x61,
    VM_FLOAT_PUSH = 0x70,
    VM_FLOAT_POP = 0x71,
    VM_FLOAT_DUP = 0x72,
    VM_FLOAT_SWAP = 0x73,
    VM_FLOAT_STORE = 0x74,
    VM_FLOAT_STORE_UINT16 = 0x75,
    VM_FLOAT_LOAD = 0x76,
    VM_FLOAT_LOAD_UINT16 = 0x77,
    VM_FLOAT_MOVE_UINT8 = 0x78,
    VM_FLOAT_COPY_UINT8 = 0x79,
    VM_FLOAT_JUMP_EMPTY = 0x7a,
    VM_FLOAT_JUMP_ZERO = 0x7b,
    VM_FLOAT_JUMP_NZERO = 0x7c,
    VM_FLOAT_ADD = 0x7d,
    VM_FLOAT_SUBTRACT = 0x7e,
    VM_FLOAT_MULTIPLY = 0x7f,
    VM_FLOAT_DIVIDE_FLOAT = 0x80,
    VM_FLOAT_EQ = 0x81,
    VM__INSEND = 0xff
} rendervm_opcode_t;

rendervm_t* rendervm_create();
void rendervm_destroy(rendervm_t* vm);
//uint8_t rendervm_last_opcode(rendervm_t* vm);
//uint8_t rendervm_resume(rendervm_t* vm);
void rendervm_reset(rendervm_t* vm);
uint8_t rendervm_exec(rendervm_t* vm, uint8_t* program, uint16_t length);
const char* rendervm_opcode2str(rendervm_opcode_t opcode);
//void rendervm_alloc_ex_interrupt(rendervm_t* vm);
//uint8_t rendervm_has_exception(rendervm_t* vm);

#endif
