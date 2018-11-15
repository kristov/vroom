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
    VM_HALT,
    VM_YIELD,
    VM_RESET,
    VM_CALL,
    VM_RETURN,
    VM_UINT16_PUSH,
    VM_UINT16_POP,
    VM_UINT16_ADD,
    VM_UINT16_MULTIPLY,
    VM_UINT16_MOVE_UINT8,
    VM_UINT8_PUSH,
    VM_UINT8_POP,
    VM_UINT8_STORE,
    VM_UINT8_LOAD,
    VM_UINT8_ADD,
    VM_UINT8_LOAD_UINT16,
    VM_UINT8_JUMP_ZERO,
    VM_UINT8_SUBTRACT,
    VM_UINT8_EQ,
    VM_UINT8_JUMP_NZERO,
    VM_UINT8_SWAP,
    VM_UINT16_COPY_UINT8,
    VM_UINT8_JUMP_EMPTY,
    VM_UINT16_DUP,
    VM_UINT8_STORE_UINT16,
    VM_UINT8_DUP,
    VM_JUMP
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
