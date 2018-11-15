#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "rendervm.h"

#define NCODE(vm)           program[vm->pc++]
#define CTRL_PUSH(vm, v)    vm->ctrl_stack[++vm->ctrl_sp] = v
#define CTRL_POP(vm)        vm->ctrl_stack[vm->ctrl_sp--]
#define UINT8_PUSH(vm, v)   vm->uint8_stack[++vm->uint8_sp] = v
#define UINT8_POP(vm)       vm->uint8_stack[vm->uint8_sp--]
#define UINT8_PEEK(vm, v)   vm->uint8_stack[v]
#define UINT16_MAKE(vm)     (vm->b1 << 8) | vm->b0
#define UINT16_PUSH(vm, v)  vm->uint16_stack[++vm->uint16_sp] = v
#define UINT16_POP(vm)      vm->uint16_stack[vm->uint16_sp--]
#define UINT16_PEEK(vm, v)  vm->uint16_stack[v]
#define UINT32_PUSH(vm, v)  vm->uint32_stack[++vm->uint32_sp] = v
#define UINT32_POP(vm)      vm->uint32_stack[vm->uint32_sp--]
#define UINT32_PEEK(vm, v)  vm->uint32_stack[v]
#define FLOAT_PUSH(vm, v)   vm->float_stack[++vm->float_sp] = v
#define FLOAT_POP(vm)       vm->float_stack[vm->float_sp--]

const char *opcode2str[] = {
    "HALT",
    "YIELD",
    "RESET",
    "CALL",
    "RETURN",
    "JUMP",
    "[unused]",
    "[unused]",
    "[unused]",
    "[unused]",
    "[unused]",
    "[unused]",
    "[unused]",
    "[unused]",
    "[unused]",
    "[unused]",
    "UINT8_PUSH",
    "UINT8_POP",
    "UINT8_DUP",
    "UINT8_SWAP",
    "UINT8_STORE",
    "UINT8_STORE_UINT16",
    "UINT8_LOAD",
    "UINT8_LOAD_UINT16",
    "UINT8_MOVE_UINT8",
    "UINT8_COPY_UINT8",
    "UINT8_JUMP_EMPTY",
    "UINT8_JUMP_ZERO",
    "UINT8_JUMP_NZERO",
    "UINT8_ADD",
    "UINT8_SUBTRACT",
    "UINT8_MULTIPLY",
    "UINT8_DIVIDE_FLOAT",
    "UINT8_EQ",
    "[unused]",
    "[unused]",
    "[unused]",
    "[unused]",
    "[unused]",
    "[unused]",
    "[unused]",
    "[unused]",
    "[unused]",
    "[unused]",
    "[unused]",
    "[unused]",
    "[unused]",
    "[unused]",
    "UINT16_PUSH",
    "UINT16_POP",
    "UINT16_DUP",
    "UINT16_SWAP",
    "UINT16_STORE",
    "UINT16_STORE_UINT16",
    "UINT16_LOAD",
    "UINT16_LOAD_UINT16",
    "UINT16_MOVE_UINT8",
    "UINT16_COPY_UINT8",
    "UINT16_JUMP_EMPTY",
    "UINT16_JUMP_ZERO",
    "UINT16_JUMP_NZERO",
    "UINT16_ADD",
    "UINT16_SUBTRACT",
    "UINT16_MULTIPLY",
    "UINT16_DIVIDE_FLOAT",
    "UINT16_EQ",
    "[unused]",
    "[unused]",
    "[unused]",
    "[unused]",
    "[unused]",
    "[unused]",
    "[unused]",
    "[unused]",
    "[unused]",
    "[unused]",
    "[unused]",
    "[unused]",
    "[unused]",
    "[unused]",
    "UINT32_PUSH",
    "UINT32_POP",
    "UINT32_DUP",
    "UINT32_SWAP",
    "UINT32_STORE",
    "UINT32_STORE_UINT16",
    "UINT32_LOAD",
    "UINT32_LOAD_UINT16",
    "UINT32_MOVE_UINT8",
    "UINT32_COPY_UINT8",
    "UINT32_JUMP_EMPTY",
    "UINT32_JUMP_ZERO",
    "UINT32_JUMP_NZERO",
    "UINT32_ADD",
    "UINT32_SUBTRACT",
    "UINT32_MULTIPLY",
    "UINT32_DIVIDE_FLOAT",
    "UINT32_EQ",
    "[unused]",
    "[unused]",
    "[unused]",
    "[unused]",
    "[unused]",
    "[unused]",
    "[unused]",
    "[unused]",
    "[unused]",
    "[unused]",
    "[unused]",
    "[unused]",
    "[unused]",
    "[unused]",
    "FLOAT_PUSH",
    "FLOAT_POP",
    "FLOAT_DUP",
    "FLOAT_SWAP",
    "FLOAT_STORE",
    "FLOAT_STORE_UINT16",
    "FLOAT_LOAD",
    "FLOAT_LOAD_UINT16",
    "FLOAT_MOVE_UINT8",
    "FLOAT_COPY_UINT8",
    "FLOAT_JUMP_EMPTY",
    "FLOAT_JUMP_ZERO",
    "FLOAT_JUMP_NZERO",
    "FLOAT_ADD",
    "FLOAT_SUBTRACT",
    "FLOAT_MULTIPLY",
    "FLOAT_DIVIDE_FLOAT",
    "FLOAT_EQ",
    "[unused]"
};

uint8_t rendervm_exec(rendervm_t* vm, uint8_t* program, uint16_t length) {
    uint16_t opcode;
    uint8_t u80, u81;
    uint16_t u160, u161, u162, u163;

    if (!vm->running) {
        return vm->running;
    }

    if (vm->pc >= length) {
        rendervm_reset(vm);
        return 0;
    }

    opcode = NCODE(vm);

    switch (opcode) {
        case VM_HALT:
            vm->pc = 0;
            vm->running = 0;
            break;
        case VM_YIELD:
            vm->running = 0;
            break;
        case VM_RESET:
            rendervm_reset(vm);
            break;
        case VM_CALL:
            vm->b0 = NCODE(vm);
            vm->b1 = NCODE(vm);
            u160 = UINT16_MAKE(vm);
            CTRL_PUSH(vm, vm->pc);
            vm->pc = u160;
            break;
        case VM_RETURN:
            vm->pc = CTRL_POP(vm);
            break;
        case VM_JUMP:
            vm->b0 = NCODE(vm);
            vm->b1 = NCODE(vm);
            u160 = UINT16_MAKE(vm);
            vm->pc = u160;
            break;

        // UINT8 INSTRUCTIONS
        case VM_UINT8_PUSH:
            u80 = NCODE(vm);
            UINT8_PUSH(vm, u80);
            break;
        case VM_UINT8_POP:
            u80 = UINT8_POP(vm);
            vm->running = 0;
            break;
        case VM_UINT8_DUP:
            vm->b0 = NCODE(vm);
            vm->b1 = NCODE(vm);
            u160 = UINT16_MAKE(vm);
            u161 = vm->uint8_sp;
            for (u162 = u160; u162 > 0; u162--) {
                u80 = UINT8_PEEK(vm, (u161 - (u162 - 1)));
                UINT8_PUSH(vm, u80);
            }
            break;
        case VM_UINT8_SWAP:
            u80 = UINT8_POP(vm);
            u81 = UINT8_POP(vm);
            UINT8_PUSH(vm, u80);
            UINT8_PUSH(vm, u81);
            break;
        case VM_UINT8_STORE:
            vm->b0 = NCODE(vm);
            vm->b1 = NCODE(vm);
            u160 = UINT16_MAKE(vm);
            u80 = UINT8_POP(vm);
            vm->uint8_memory[u160] = u80;
            break;
        case VM_UINT8_STORE_UINT16:
            u80 = UINT8_POP(vm);
            u160 = UINT16_POP(vm);
            vm->uint8_memory[u160] = u80;
            break;
        case VM_UINT8_LOAD:
            vm->b0 = NCODE(vm);
            vm->b1 = NCODE(vm);
            u160 = UINT16_MAKE(vm);
            u80 = vm->uint8_memory[u160];
            UINT8_PUSH(vm, u80);
            break;
        case VM_UINT8_LOAD_UINT16:
            u160 = UINT16_POP(vm);
            u80 = vm->uint8_memory[u160];
            UINT8_PUSH(vm, u80);
            break;
        case VM_UINT8_MOVE_UINT8:
            printf("UINT8_MOVE_UINT8: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_UINT8_COPY_UINT8:
            printf("UINT8_COPY_UINT8: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_UINT8_JUMP_EMPTY:
            vm->b0 = NCODE(vm);
            vm->b1 = NCODE(vm);
            if (vm->uint8_sp == VM_MAX_ADDR) {
                u160 = UINT16_MAKE(vm);
                vm->pc = u160;
            }
            break;
        case VM_UINT8_JUMP_ZERO:
            vm->b0 = NCODE(vm);
            vm->b1 = NCODE(vm);
            if (UINT8_POP(vm) == 0) {
                u160 = UINT16_MAKE(vm);
                vm->pc = u160;
            }
            break;
        case VM_UINT8_JUMP_NZERO:
            vm->b0 = NCODE(vm);
            vm->b1 = NCODE(vm);
            if (UINT8_POP(vm)) {
                u160 = UINT16_MAKE(vm);
                vm->pc = u160;
            }
            break;
        case VM_UINT8_ADD:
            u80 = UINT8_POP(vm);
            u81 = UINT8_POP(vm);
            UINT8_PUSH(vm, (u81 + u80));
            break;
        case VM_UINT8_SUBTRACT:
            u80 = UINT8_POP(vm);
            u81 = UINT8_POP(vm);
            UINT8_PUSH(vm, (u81 - u80));
            break;
        case VM_UINT8_MULTIPLY:
            printf("UINT8_MULTIPLY: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_UINT8_DIVIDE_FLOAT:
            printf("UINT8_DIVIDE_FLOAT: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_UINT8_EQ:
            u80 = UINT8_POP(vm);
            u81 = UINT8_POP(vm);
            UINT8_PUSH(vm, (u81 == u80 ? 1 : 0));
            break;

        // UINT16 INSTRUCTIONS
        case VM_UINT16_PUSH:
            vm->b0 = NCODE(vm);
            vm->b1 = NCODE(vm);
            u160 = UINT16_MAKE(vm);
            UINT16_PUSH(vm, u160);
            break;
        case VM_UINT16_POP:
            u160 = UINT16_POP(vm);
            break;
        case VM_UINT16_DUP:
            vm->b0 = NCODE(vm);
            vm->b1 = NCODE(vm);
            u160 = UINT16_MAKE(vm);
            u161 = vm->uint16_sp;
            for (u162 = u160; u162 > 0; u162--) {
                u163 = UINT16_PEEK(vm, (u161 - (u162 - 1)));
                UINT16_PUSH(vm, u163);
            }
            break;
        case VM_UINT16_SWAP:
            printf("UINT16_SWAP: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_UINT16_STORE:
            printf("UINT16_STORE: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_UINT16_STORE_UINT16:
            printf("UINT16_STORE_UINT16: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_UINT16_LOAD:
            printf("UINT16_LOAD: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_UINT16_LOAD_UINT16:
            printf("UINT16_LOAD_UINT16: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_UINT16_MOVE_UINT8:
            UINT16_PUSH(vm, UINT8_POP(vm));
            break;
        case VM_UINT16_COPY_UINT8:
            vm->b0 = NCODE(vm);
            vm->b1 = NCODE(vm);
            u160 = UINT16_MAKE(vm);
            u161 = vm->uint8_sp;
            for (u162 = u160; u162 > 0; u162--) {
                u163 = UINT8_PEEK(vm, (u161 - (u162 - 1)));
                UINT16_PUSH(vm, u163);
            }
            break;
        case VM_UINT16_JUMP_EMPTY:
            printf("UINT16_JUMP_EMPTY: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_UINT16_JUMP_ZERO:
            printf("UINT16_JUMP_ZERO: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_UINT16_JUMP_NZERO:
            printf("UINT16_JUMP_NZERO: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_UINT16_ADD:
            u160 = UINT16_POP(vm);
            u161 = UINT16_POP(vm);
            UINT16_PUSH(vm, (u160 + u161));
            break;
        case VM_UINT16_SUBTRACT:
            printf("UINT16_SUBTRACT: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_UINT16_MULTIPLY:
            u160 = UINT16_POP(vm);
            u161 = UINT16_POP(vm);
            UINT16_PUSH(vm, (u160 * u161));
            break;
        case VM_UINT16_DIVIDE_FLOAT:
            printf("UINT16_DIVIDE_FLOAT: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_UINT16_EQ:
            printf("UINT16_EQ: UNIMPLEMENTED");
            vm->running = 0;
            break;

        // UINT32 INSTRUCTIONS
        case VM_UINT32_PUSH:
            printf("UINT32_PUSH: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_UINT32_POP:
            printf("UINT32_POP: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_UINT32_DUP:
            printf("UINT32_DUP: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_UINT32_SWAP:
            printf("UINT32_SWAP: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_UINT32_STORE:
            printf("UINT32_STORE: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_UINT32_STORE_UINT16:
            printf("UINT32_STORE_UINT16: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_UINT32_LOAD:
            printf("UINT32_LOAD: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_UINT32_LOAD_UINT16:
            printf("UINT32_LOAD_UINT16: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_UINT32_MOVE_UINT8:
            printf("UINT32_MOVE_UINT8: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_UINT32_COPY_UINT8:
            printf("UINT32_COPY_UINT8: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_UINT32_JUMP_EMPTY:
            printf("UINT32_JUMP_EMPTY: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_UINT32_JUMP_ZERO:
            printf("UINT32_JUMP_ZERO: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_UINT32_JUMP_NZERO:
            printf("UINT32_JUMP_NZERO: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_UINT32_ADD:
            printf("UINT32_ADD: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_UINT32_SUBTRACT:
            printf("UINT32_SUBTRACT: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_UINT32_MULTIPLY:
            printf("UINT32_MULTIPLY: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_UINT32_DIVIDE_FLOAT:
            printf("UINT32_DIVIDE_FLOAT: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_UINT32_EQ:
            printf("UINT32_EQ: UNIMPLEMENTED");
            vm->running = 0;
            break;

        // FLOAT INSTRUCTIONS
        case VM_FLOAT_PUSH:
            printf("FLOAT_PUSH: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_FLOAT_POP:
            printf("FLOAT_POP: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_FLOAT_DUP:
            printf("FLOAT_DUP: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_FLOAT_SWAP:
            printf("FLOAT_SWAP: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_FLOAT_STORE:
            printf("FLOAT_STORE: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_FLOAT_STORE_UINT16:
            printf("FLOAT_STORE_UINT16: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_FLOAT_LOAD:
            printf("FLOAT_LOAD: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_FLOAT_LOAD_UINT16:
            printf("FLOAT_LOAD_UINT16: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_FLOAT_MOVE_UINT8:
            printf("FLOAT_MOVE_UINT8: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_FLOAT_COPY_UINT8:
            printf("FLOAT_COPY_UINT8: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_FLOAT_JUMP_EMPTY:
            printf("FLOAT_JUMP_EMPTY: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_FLOAT_JUMP_ZERO:
            printf("FLOAT_JUMP_ZERO: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_FLOAT_JUMP_NZERO:
            printf("FLOAT_JUMP_NZERO: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_FLOAT_ADD:
            printf("FLOAT_ADD: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_FLOAT_SUBTRACT:
            printf("FLOAT_SUBTRACT: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_FLOAT_MULTIPLY:
            printf("FLOAT_MULTIPLY: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_FLOAT_DIVIDE_FLOAT:
            printf("FLOAT_DIVIDE_FLOAT: UNIMPLEMENTED");
            vm->running = 0;
            break;
        case VM_FLOAT_EQ:
            printf("FLOAT_EQ: UNIMPLEMENTED");
            vm->running = 0;
            break;
        default:
            break;
    }
    vm->last_opcode = opcode;

    return vm->running;
}

const char* rendervm_opcode2str(rendervm_opcode_t opcode) {
    return opcode2str[opcode];
}

void rendervm_reset(rendervm_t* vm) {
    memset(vm->ctrl_stack, 0, VM_STACK_SIZE);

    vm->pc = 0;
    vm->running = 1;

    vm->ctrl_sp = VM_MAX_ADDR;
    vm->uint8_sp = VM_MAX_ADDR;
    vm->uint16_sp = VM_MAX_ADDR;
    vm->uint32_sp = VM_MAX_ADDR;
    vm->float_sp = VM_MAX_ADDR;
}

void rendervm_destroy(rendervm_t* vm) {
    free(vm->ctrl_stack);
    free(vm);
}

rendervm_t* rendervm_create() {
    rendervm_t* vm = malloc(sizeof(rendervm_t));
    memset(vm, 0, sizeof(rendervm_t));

    vm->running = 1;

    vm->ctrl_sp = VM_MAX_ADDR;
    vm->uint8_sp = VM_MAX_ADDR;
    vm->uint16_sp = VM_MAX_ADDR;
    vm->uint32_sp = VM_MAX_ADDR;
    vm->float_sp = VM_MAX_ADDR;

    vm->ctrl_stack = malloc(VM_STACK_SIZE);
    memset(vm->ctrl_stack, 0, VM_STACK_SIZE);

    vm->uint8_stack = malloc(VM_STACK_SIZE);
    memset(vm->uint8_stack, 0, VM_STACK_SIZE);

    vm->uint16_stack = malloc(VM_STACK_SIZE);
    memset(vm->uint16_stack, 0, VM_STACK_SIZE);

    return vm;
}

