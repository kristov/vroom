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
#define UINT16_MAKE(vm)     ((vm->b1 << 8) | vm->b0)
#define UINT16_PUSH(vm, v)  vm->uint16_stack[++vm->uint16_sp] = v
#define UINT16_POP(vm)      vm->uint16_stack[vm->uint16_sp--]
#define UINT16_PEEK(vm, v)  vm->uint16_stack[v]
#define UINT32_MAKE(vm)     ((vm->b3 << 24) | (vm->b2 << 16) | (vm->b1 << 8) | vm->b0)
#define UINT32_PUSH(vm, v)  vm->uint32_stack[++vm->uint32_sp] = v
#define UINT32_POP(vm)      vm->uint32_stack[vm->uint32_sp--]
#define UINT32_PEEK(vm, v)  vm->uint32_stack[v]
#define FLOAT_PUSH(vm, v)   vm->float_stack[++vm->float_sp] = v
#define FLOAT_POP(vm)       vm->float_stack[vm->float_sp--]
#define FLOAT_PEEK(vm, v)   vm->float_stack[v]

typedef union {
    float f;
    uint32_t u;
} float_convert_t;

float FLOAT_MAKE(rendervm_t* vm) {
    float_convert_t tmp;
    tmp.u = ((vm->b3 << 24) | (vm->b2 << 16) | (vm->b1 << 8) | vm->b0);
    return tmp.f;
}

const char *opcode2str[] = {
    "HALT",
    "YIELD",
    "RESET",
    "CALL",
    "RETURN",
    "JUMP",
    "UINT8_POP",
    "UINT8_DUP",
    "UINT8_SWAP",
    "UINT8_JUMPEM",
    "UINT8_STORE",
    "UINT8_LOAD",
    "UINT8_ADD",
    "UINT8_SUB",
    "UINT8_MUL",
    "UINT8_EQ",
    "UINT8_JUMPNZ",
    "UINT8_JUMPZ",
    "UINT8_PUSH",
    "UINT16_POP",
    "UINT16_DUP",
    "UINT16_SWAP",
    "UINT16_JUMPEM",
    "UINT16_STORE",
    "UINT16_LOAD",
    "UINT16_ADD",
    "UINT16_SUB",
    "UINT16_MUL",
    "UINT16_EQ",
    "UINT16_JUMPNZ",
    "UINT16_JUMPZ",
    "UINT16_MOVE_UINT8",
    "UINT16_PUSH",
    "UINT32_POP",
    "UINT32_DUP",
    "UINT32_SWAP",
    "UINT32_JUMPEM",
    "UINT32_STORE",
    "UINT32_LOAD",
    "UINT32_ADD",
    "UINT32_SUB",
    "UINT32_MUL",
    "UINT32_EQ",
    "UINT32_JUMPNZ",
    "UINT32_JUMPZ",
    "UINT32_MOVE_UINT8",
    "UINT32_PUSH",
    "FLOAT_POP",
    "FLOAT_DUP",
    "FLOAT_SWAP",
    "FLOAT_JUMPEM",
    "FLOAT_STORE",
    "FLOAT_LOAD",
    "FLOAT_ADD",
    "FLOAT_SUB",
    "FLOAT_MUL",
    "FLOAT_EQ",
    "FLOAT_JUMPNZ",
    "FLOAT_JUMPZ",
    "FLOAT_PUSH",
    "VEC2_POP",
    "VEC2_DUP",
    "VEC2_SWAP",
    "VEC2_JUMPEM",
    "VEC2_STORE",
    "VEC2_LOAD",
    "VEC2_ADD",
    "VEC2_SUB",
    "VEC2_MUL",
    "VEC2_EQ",
    "VEC2_EXPLODE",
    "VEC2_IMPLODE",
    "VEC2_MULMAT2",
    "VEC2_MULMAT3",
    "VEC2_MULMAT4",
    "VEC3_POP",
    "VEC3_DUP",
    "VEC3_SWAP",
    "VEC3_JUMPEM",
    "VEC3_STORE",
    "VEC3_LOAD",
    "VEC3_ADD",
    "VEC3_SUB",
    "VEC3_MUL",
    "VEC3_EQ",
    "VEC3_EXPLODE",
    "VEC3_IMPLODE",
    "VEC3_MULMAT3",
    "VEC3_MULMAT4",
    "VEC4_POP",
    "VEC4_DUP",
    "VEC4_SWAP",
    "VEC4_JUMPEM",
    "VEC4_STORE",
    "VEC4_LOAD",
    "VEC4_ADD",
    "VEC4_SUB",
    "VEC4_MUL",
    "VEC4_EQ",
    "VEC4_EXPLODE",
    "VEC4_IMPLODE",
    "VEC4_MULMAT4",
    "MAT2_POP",
    "MAT2_DUP",
    "MAT2_SWAP",
    "MAT2_JUMPEM",
    "MAT2_STORE",
    "MAT2_LOAD",
    "MAT2_ADD",
    "MAT2_SUB",
    "MAT2_MUL",
    "MAT2_EQ",
    "MAT2_EXPLODE",
    "MAT2_IDENT",
    "MAT2_IMPLODE",
    "MAT2_ROTATE",
    "MAT2_SCALE",
    "MAT2_TRANSP",
    "MAT3_POP",
    "MAT3_DUP",
    "MAT3_SWAP",
    "MAT3_JUMPEM",
    "MAT3_STORE",
    "MAT3_LOAD",
    "MAT3_ADD",
    "MAT3_SUB",
    "MAT3_MUL",
    "MAT3_EQ",
    "MAT3_EXPLODE",
    "MAT3_IDENT",
    "MAT3_IMPLODE",
    "MAT3_ROTATE",
    "MAT3_SCALE",
    "MAT3_TRANSL",
    "MAT3_TRANSP",
    "MAT4_POP",
    "MAT4_DUP",
    "MAT4_SWAP",
    "MAT4_JUMPEM",
    "MAT4_STORE",
    "MAT4_LOAD",
    "MAT4_ADD",
    "MAT4_SUB",
    "MAT4_MUL",
    "MAT4_EQ",
    "MAT4_EXPLODE",
    "MAT4_IDENT",
    "MAT4_IMPLODE",
    "MAT4_ROTATEX",
    "MAT4_ROTATEY",
    "MAT4_ROTATEZ",
    "MAT4_SCALE",
    "MAT4_TRANSL",
    "MAT4_TRANSP",
    "[unused]"
};

uint8_t rendervm_exec(rendervm_t* vm, uint8_t* program, uint16_t length) {
    uint16_t opcode;
    uint8_t u80, u81, u82, u83;
    uint16_t u160, u161;
    uint32_t u320, u321;
    float fl0, fl1;

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
        case VM_UINT8_POP:
            u80 = UINT8_POP(vm);
            break;
        case VM_UINT8_DUP:
            u80 = NCODE(vm);
            u81 = vm->uint8_sp;
            for (u82 = u80; u82 > 0; u82--) {
                u83 = UINT8_PEEK(vm, (u81 - (u82 - 1)));
                UINT8_PUSH(vm, u83);
            }
            break;
        case VM_UINT8_SWAP:
            u80 = UINT8_POP(vm);
            u81 = UINT8_POP(vm);
            UINT8_PUSH(vm, u80);
            UINT8_PUSH(vm, u81);
            break;
        case VM_UINT8_JUMPEM:
            vm->b0 = NCODE(vm);
            vm->b1 = NCODE(vm);
            if (vm->uint8_sp == VM_MAX_ADDR) {
                u160 = UINT16_MAKE(vm);
                vm->pc = u160;
            }
            break;
        case VM_UINT8_STORE:
            u80 = UINT8_POP(vm);
            u160 = UINT16_POP(vm);
            vm->uint8_memory[u160] = u80;
            break;
        case VM_UINT8_LOAD:
            u160 = UINT16_POP(vm);
            u80 = vm->uint8_memory[u160];
            UINT8_PUSH(vm, u80);
            break;
        case VM_UINT8_ADD:
            u80 = UINT8_POP(vm);
            u81 = UINT8_POP(vm);
            UINT8_PUSH(vm, (u81 + u80));
            break;
        case VM_UINT8_SUB:
            u80 = UINT8_POP(vm);
            u81 = UINT8_POP(vm);
            UINT8_PUSH(vm, (u81 - u80));
            break;
        case VM_UINT8_MUL:
            u80 = UINT8_POP(vm);
            u81 = UINT8_POP(vm);
            UINT8_PUSH(vm, (u81 * u80));
            break;
        case VM_UINT8_EQ:
            u80 = UINT8_POP(vm);
            u81 = UINT8_POP(vm);
            UINT8_PUSH(vm, (u81 == u80 ? 1 : 0));
            break;
        case VM_UINT8_JUMPNZ:
            vm->b0 = NCODE(vm);
            vm->b1 = NCODE(vm);
            if (UINT8_POP(vm)) {
                u160 = UINT16_MAKE(vm);
                vm->pc = u160;
            }
            break;
        case VM_UINT8_JUMPZ:
            vm->b0 = NCODE(vm);
            vm->b1 = NCODE(vm);
            if (UINT8_POP(vm) == 0) {
                u160 = UINT16_MAKE(vm);
                vm->pc = u160;
            }
            break;
        case VM_UINT8_PUSH:
            u80 = NCODE(vm);
            UINT8_PUSH(vm, u80);
            break;
        case VM_UINT16_POP:
            u160 = UINT16_POP(vm);
            break;
        case VM_UINT16_DUP:
            u80 = NCODE(vm);
            u81 = vm->uint16_sp;
            for (u82 = u80; u82 > 0; u82--) {
                u160 = UINT16_PEEK(vm, (u81 - (u82 - 1)));
                UINT16_PUSH(vm, u160);
            }
            break;
        case VM_UINT16_SWAP:
            u160 = UINT16_POP(vm);
            u161 = UINT16_POP(vm);
            UINT16_PUSH(vm, u160);
            UINT16_PUSH(vm, u161);
            break;
        case VM_UINT16_JUMPEM:
            vm->b0 = NCODE(vm);
            vm->b1 = NCODE(vm);
            if (vm->uint16_sp == VM_MAX_ADDR) {
                u160 = UINT16_MAKE(vm);
                vm->pc = u160;
            }
            break;
        case VM_UINT16_STORE:
            u160 = UINT16_POP(vm);
            u161 = UINT16_POP(vm);
            vm->uint16_memory[u161] = u160;
            break;
        case VM_UINT16_LOAD:
            u160 = UINT16_POP(vm);
            u161 = vm->uint16_memory[u160];
            UINT16_PUSH(vm, u161);
            break;
        case VM_UINT16_ADD:
            u160 = UINT16_POP(vm);
            u161 = UINT16_POP(vm);
            UINT16_PUSH(vm, (u161 + u160));
            break;
        case VM_UINT16_SUB:
            u160 = UINT16_POP(vm);
            u161 = UINT16_POP(vm);
            UINT16_PUSH(vm, (u161 - u160));
            break;
        case VM_UINT16_MUL:
            u160 = UINT16_POP(vm);
            u161 = UINT16_POP(vm);
            UINT16_PUSH(vm, (u161 * u160));
            break;
        case VM_UINT16_EQ:
            u160 = UINT16_POP(vm);
            u161 = UINT16_POP(vm);
            UINT16_PUSH(vm, (u161 == u160 ? 1 : 0));
            break;
        case VM_UINT16_JUMPNZ:
            vm->b0 = NCODE(vm);
            vm->b1 = NCODE(vm);
            if (UINT16_POP(vm)) {
                u160 = UINT16_MAKE(vm);
                vm->pc = u160;
            }
            break;
        case VM_UINT16_JUMPZ:
            vm->b0 = NCODE(vm);
            vm->b1 = NCODE(vm);
            if (UINT16_POP(vm) == 0) {
                u160 = UINT16_MAKE(vm);
                vm->pc = u160;
            }
            break;
        case VM_UINT16_MOVE_UINT8:
            printf("UINT16_MOVE_UINT8: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_UINT16_PUSH:
            vm->b0 = NCODE(vm);
            vm->b1 = NCODE(vm);
            u160 = UINT16_MAKE(vm);
            UINT16_PUSH(vm, u160);
            break;
        case VM_UINT32_POP:
            u320 = UINT32_POP(vm);
            break;
        case VM_UINT32_DUP:
            u80 = NCODE(vm);
            u81 = vm->uint32_sp;
            for (u82 = u80; u82 > 0; u82--) {
                u320 = UINT32_PEEK(vm, (u81 - (u82 - 1)));
                UINT32_PUSH(vm, u320);
            }
            break;
        case VM_UINT32_SWAP:
            u320 = UINT32_POP(vm);
            u321 = UINT32_POP(vm);
            UINT32_PUSH(vm, u320);
            UINT32_PUSH(vm, u321);
            break;
        case VM_UINT32_JUMPEM:
            vm->b0 = NCODE(vm);
            vm->b1 = NCODE(vm);
            if (vm->uint32_sp == VM_MAX_ADDR) {
                u160 = UINT16_MAKE(vm);
                vm->pc = u160;
            }
            break;
        case VM_UINT32_STORE:
            u320 = UINT32_POP(vm);
            u161 = UINT16_POP(vm);
            vm->uint32_memory[u161] = u320;
            break;
        case VM_UINT32_LOAD:
            u160 = UINT16_POP(vm);
            u320 = vm->uint32_memory[u160];
            UINT32_PUSH(vm, u320);
            break;
        case VM_UINT32_ADD:
            u320 = UINT32_POP(vm);
            u321 = UINT32_POP(vm);
            UINT32_PUSH(vm, (u321 + u320));
            break;
        case VM_UINT32_SUB:
            u320 = UINT32_POP(vm);
            u321 = UINT32_POP(vm);
            UINT32_PUSH(vm, (u321 - u320));
            break;
        case VM_UINT32_MUL:
            u320 = UINT32_POP(vm);
            u321 = UINT32_POP(vm);
            UINT32_PUSH(vm, (u321 * u320));
            break;
        case VM_UINT32_EQ:
            u320 = UINT32_POP(vm);
            u321 = UINT32_POP(vm);
            UINT32_PUSH(vm, (u321 == u320 ? 1 : 0));
            break;
        case VM_UINT32_JUMPNZ:
            vm->b0 = NCODE(vm);
            vm->b1 = NCODE(vm);
            if (UINT32_POP(vm)) {
                u160 = UINT16_MAKE(vm);
                vm->pc = u160;
            }
            break;
        case VM_UINT32_JUMPZ:
            vm->b0 = NCODE(vm);
            vm->b1 = NCODE(vm);
            if (UINT32_POP(vm) == 0) {
                u160 = UINT16_MAKE(vm);
                vm->pc = u160;
            }
            break;
        case VM_UINT32_MOVE_UINT8:
            printf("UINT32_MOVE_UINT8: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_UINT32_PUSH:
            vm->b0 = NCODE(vm);
            vm->b1 = NCODE(vm);
            vm->b2 = NCODE(vm);
            vm->b3 = NCODE(vm);
            u320 = UINT32_MAKE(vm);
            UINT32_PUSH(vm, u320);
            break;
        case VM_FLOAT_POP:
            fl0 = FLOAT_POP(vm);
            break;
        case VM_FLOAT_DUP:
            u80 = NCODE(vm);
            u81 = vm->float_sp;
            for (u82 = u80; u82 > 0; u82--) {
                fl0 = FLOAT_PEEK(vm, (u81 - (u82 - 1)));
                FLOAT_PUSH(vm, fl0);
            }
            break;
        case VM_FLOAT_SWAP:
            fl0 = FLOAT_POP(vm);
            fl1 = FLOAT_POP(vm);
            FLOAT_PUSH(vm, fl0);
            FLOAT_PUSH(vm, fl1);
            break;
        case VM_FLOAT_JUMPEM:
            vm->b0 = NCODE(vm);
            vm->b1 = NCODE(vm);
            if (vm->float_sp == VM_MAX_ADDR) {
                u160 = UINT16_MAKE(vm);
                vm->pc = u160;
            }
            break;
        case VM_FLOAT_STORE:
            fl0 = FLOAT_POP(vm);
            u161 = UINT16_POP(vm);
            vm->float_memory[u161] = fl0;
            break;
        case VM_FLOAT_LOAD:
            u160 = UINT16_POP(vm);
            fl0 = vm->float_memory[u160];
            FLOAT_PUSH(vm, fl0);
            break;
        case VM_FLOAT_ADD:
            fl0 = FLOAT_POP(vm);
            fl1 = FLOAT_POP(vm);
            FLOAT_PUSH(vm, (fl1 + fl0));
            break;
        case VM_FLOAT_SUB:
            fl0 = FLOAT_POP(vm);
            fl1 = FLOAT_POP(vm);
            FLOAT_PUSH(vm, (fl1 - fl0));
            break;
        case VM_FLOAT_MUL:
            fl0 = FLOAT_POP(vm);
            fl1 = FLOAT_POP(vm);
            FLOAT_PUSH(vm, (fl1 * fl0));
            break;
        case VM_FLOAT_EQ:
            fl0 = FLOAT_POP(vm);
            fl1 = FLOAT_POP(vm);
            FLOAT_PUSH(vm, (fl1 == fl0 ? 1 : 0));
            break;
        case VM_FLOAT_JUMPNZ:
            vm->b0 = NCODE(vm);
            vm->b1 = NCODE(vm);
            if (FLOAT_POP(vm)) {
                u160 = UINT16_MAKE(vm);
                vm->pc = u160;
            }
            break;
        case VM_FLOAT_JUMPZ:
            vm->b0 = NCODE(vm);
            vm->b1 = NCODE(vm);
            if (FLOAT_POP(vm) == 0.0f) {
                u160 = UINT16_MAKE(vm);
                vm->pc = u160;
            }
            break;
        case VM_FLOAT_PUSH:
            vm->b0 = NCODE(vm);
            vm->b1 = NCODE(vm);
            vm->b2 = NCODE(vm);
            vm->b3 = NCODE(vm);
            fl0 = FLOAT_MAKE(vm);
            FLOAT_PUSH(vm, fl0);
            break;
        case VM_VEC2_POP:
            printf("VEC2_POP: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_VEC2_DUP:
            printf("VEC2_DUP: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_VEC2_SWAP:
            printf("VEC2_SWAP: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_VEC2_JUMPEM:
            printf("VEC2_JUMPEM: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_VEC2_STORE:
            printf("VEC2_STORE: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_VEC2_LOAD:
            printf("VEC2_LOAD: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_VEC2_ADD:
            printf("VEC2_ADD: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_VEC2_SUB:
            printf("VEC2_SUB: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_VEC2_MUL:
            printf("VEC2_MUL: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_VEC2_EQ:
            printf("VEC2_EQ: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_VEC2_EXPLODE:
            printf("VEC2_EXPLODE: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_VEC2_IMPLODE:
            printf("VEC2_IMPLODE: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_VEC2_MULMAT2:
            printf("VEC2_MULMAT2: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_VEC2_MULMAT3:
            printf("VEC2_MULMAT3: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_VEC2_MULMAT4:
            printf("VEC2_MULMAT4: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_VEC3_POP:
            printf("VEC3_POP: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_VEC3_DUP:
            printf("VEC3_DUP: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_VEC3_SWAP:
            printf("VEC3_SWAP: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_VEC3_JUMPEM:
            printf("VEC3_JUMPEM: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_VEC3_STORE:
            printf("VEC3_STORE: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_VEC3_LOAD:
            printf("VEC3_LOAD: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_VEC3_ADD:
            printf("VEC3_ADD: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_VEC3_SUB:
            printf("VEC3_SUB: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_VEC3_MUL:
            printf("VEC3_MUL: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_VEC3_EQ:
            printf("VEC3_EQ: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_VEC3_EXPLODE:
            printf("VEC3_EXPLODE: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_VEC3_IMPLODE:
            printf("VEC3_IMPLODE: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_VEC3_MULMAT3:
            printf("VEC3_MULMAT3: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_VEC3_MULMAT4:
            printf("VEC3_MULMAT4: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_VEC4_POP:
            printf("VEC4_POP: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_VEC4_DUP:
            printf("VEC4_DUP: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_VEC4_SWAP:
            printf("VEC4_SWAP: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_VEC4_JUMPEM:
            printf("VEC4_JUMPEM: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_VEC4_STORE:
            printf("VEC4_STORE: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_VEC4_LOAD:
            printf("VEC4_LOAD: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_VEC4_ADD:
            printf("VEC4_ADD: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_VEC4_SUB:
            printf("VEC4_SUB: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_VEC4_MUL:
            printf("VEC4_MUL: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_VEC4_EQ:
            printf("VEC4_EQ: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_VEC4_EXPLODE:
            printf("VEC4_EXPLODE: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_VEC4_IMPLODE:
            printf("VEC4_IMPLODE: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_VEC4_MULMAT4:
            printf("VEC4_MULMAT4: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT2_POP:
            printf("MAT2_POP: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT2_DUP:
            printf("MAT2_DUP: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT2_SWAP:
            printf("MAT2_SWAP: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT2_JUMPEM:
            printf("MAT2_JUMPEM: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT2_STORE:
            printf("MAT2_STORE: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT2_LOAD:
            printf("MAT2_LOAD: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT2_ADD:
            printf("MAT2_ADD: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT2_SUB:
            printf("MAT2_SUB: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT2_MUL:
            printf("MAT2_MUL: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT2_EQ:
            printf("MAT2_EQ: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT2_EXPLODE:
            printf("MAT2_EXPLODE: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT2_IDENT:
            printf("MAT2_IDENT: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT2_IMPLODE:
            printf("MAT2_IMPLODE: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT2_ROTATE:
            printf("MAT2_ROTATE: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT2_SCALE:
            printf("MAT2_SCALE: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT2_TRANSP:
            printf("MAT2_TRANSP: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT3_POP:
            printf("MAT3_POP: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT3_DUP:
            printf("MAT3_DUP: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT3_SWAP:
            printf("MAT3_SWAP: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT3_JUMPEM:
            printf("MAT3_JUMPEM: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT3_STORE:
            printf("MAT3_STORE: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT3_LOAD:
            printf("MAT3_LOAD: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT3_ADD:
            printf("MAT3_ADD: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT3_SUB:
            printf("MAT3_SUB: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT3_MUL:
            printf("MAT3_MUL: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT3_EQ:
            printf("MAT3_EQ: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT3_EXPLODE:
            printf("MAT3_EXPLODE: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT3_IDENT:
            printf("MAT3_IDENT: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT3_IMPLODE:
            printf("MAT3_IMPLODE: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT3_ROTATE:
            printf("MAT3_ROTATE: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT3_SCALE:
            printf("MAT3_SCALE: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT3_TRANSL:
            printf("MAT3_TRANSL: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT3_TRANSP:
            printf("MAT3_TRANSP: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT4_POP:
            printf("MAT4_POP: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT4_DUP:
            printf("MAT4_DUP: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT4_SWAP:
            printf("MAT4_SWAP: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT4_JUMPEM:
            printf("MAT4_JUMPEM: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT4_STORE:
            printf("MAT4_STORE: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT4_LOAD:
            printf("MAT4_LOAD: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT4_ADD:
            printf("MAT4_ADD: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT4_SUB:
            printf("MAT4_SUB: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT4_MUL:
            printf("MAT4_MUL: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT4_EQ:
            printf("MAT4_EQ: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT4_EXPLODE:
            printf("MAT4_EXPLODE: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT4_IDENT:
            printf("MAT4_IDENT: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT4_IMPLODE:
            printf("MAT4_IMPLODE: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT4_ROTATEX:
            printf("MAT4_ROTATEX: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT4_ROTATEY:
            printf("MAT4_ROTATEY: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT4_ROTATEZ:
            printf("MAT4_ROTATEZ: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT4_SCALE:
            printf("MAT4_SCALE: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT4_TRANSL:
            printf("MAT4_TRANSL: UNIMPLEMENTED\n");
            vm->running = 0;
            break;
        case VM_MAT4_TRANSP:
            printf("MAT4_TRANSP: UNIMPLEMENTED\n");
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
    vm->vec2_sp = VM_MAX_ADDR;
    vm->vec3_sp = VM_MAX_ADDR;
    vm->vec4_sp = VM_MAX_ADDR;
    vm->mat2_sp = VM_MAX_ADDR;
    vm->mat3_sp = VM_MAX_ADDR;
    vm->mat4_sp = VM_MAX_ADDR;

    vm->ctrl_stack = malloc(VM_STACK_SIZE);
    memset(vm->ctrl_stack, 0, VM_STACK_SIZE);

    vm->uint8_stack = malloc(VM_STACK_SIZE);
    memset(vm->uint8_stack, 0, VM_STACK_SIZE);

    vm->uint16_stack = malloc(VM_STACK_SIZE);
    memset(vm->uint16_stack, 0, VM_STACK_SIZE);

    vm->uint32_stack = malloc(VM_STACK_SIZE);
    memset(vm->uint32_stack, 0, VM_STACK_SIZE);

    vm->float_stack = malloc(VM_STACK_SIZE);
    memset(vm->float_stack, 0, VM_STACK_SIZE);

    vm->vec2_stack = malloc(VM_STACK_SIZE);
    memset(vm->vec2_stack, 0, VM_STACK_SIZE);

    vm->vec3_stack = malloc(VM_STACK_SIZE);
    memset(vm->vec3_stack, 0, VM_STACK_SIZE);

    vm->vec4_stack = malloc(VM_STACK_SIZE);
    memset(vm->vec4_stack, 0, VM_STACK_SIZE);

    vm->mat2_stack = malloc(VM_STACK_SIZE);
    memset(vm->mat2_stack, 0, VM_STACK_SIZE);

    vm->mat3_stack = malloc(VM_STACK_SIZE);
    memset(vm->mat3_stack, 0, VM_STACK_SIZE);

    vm->mat4_stack = malloc(VM_STACK_SIZE);
    memset(vm->mat4_stack, 0, VM_STACK_SIZE);

    return vm;
}

