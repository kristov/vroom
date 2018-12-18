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
#define VEC2_PUSH1(vm, v)   vm->vec2_stack[++vm->vec2_sp] = v
#define VEC2_POP1(vm)       vm->vec2_stack[vm->vec2_sp--]
#define VEC2_PEEK1(vm, v)   vm->vec2_stack[v]

typedef union {
    float f;
    uint32_t u;
} float_convert_t;

float FLOAT_MAKE(rendervm_t* vm) {
    float_convert_t tmp;
    tmp.u = ((vm->b3 << 24) | (vm->b2 << 16) | (vm->b1 << 8) | vm->b0);
    return tmp.f;
}

typedef struct rendervm_opcode_info {
    const char* name;
    uint8_t arg_byte_length;
} rendervm_opcode_info_t;

const rendervm_opcode_info_t opcode_info[] = {
    {"HALT", 0x00},
    {"YIELD", 0x00},
    {"RESET", 0x00},
    {"CALL", 0x02},
    {"RETURN", 0x00},
    {"JUMP", 0x02},
    {"UINT8_POP", 0x01},
    {"UINT8_DUP", 0x01},
    {"UINT8_SWAP", 0x00},
    {"UINT8_JUMPEM", 0x02},
    {"UINT8_STORE", 0x00},
    {"UINT8_LOAD", 0x00},
    {"UINT8_ADD", 0x00},
    {"UINT8_SUB", 0x00},
    {"UINT8_MUL", 0x00},
    {"UINT8_EQ", 0x00},
    {"UINT8_ADDN", 0x01},
    {"UINT8_JUMPNZ", 0x02},
    {"UINT8_JUMPZ", 0x02},
    {"UINT8_PUSH", 0x01},
    {"UINT16_POP", 0x01},
    {"UINT16_DUP", 0x01},
    {"UINT16_SWAP", 0x00},
    {"UINT16_JUMPEM", 0x02},
    {"UINT16_STORE", 0x00},
    {"UINT16_LOAD", 0x00},
    {"UINT16_ADD", 0x00},
    {"UINT16_SUB", 0x00},
    {"UINT16_MUL", 0x00},
    {"UINT16_EQ", 0x00},
    {"UINT16_ADDN", 0x02},
    {"UINT16_JUMPNZ", 0x02},
    {"UINT16_JUMPZ", 0x02},
    {"UINT16_MOVE_UINT8", 0x01},
    {"UINT16_PUSH", 0x02},
    {"UINT32_POP", 0x01},
    {"UINT32_DUP", 0x01},
    {"UINT32_SWAP", 0x00},
    {"UINT32_JUMPEM", 0x02},
    {"UINT32_STORE", 0x00},
    {"UINT32_LOAD", 0x00},
    {"UINT32_ADD", 0x00},
    {"UINT32_SUB", 0x00},
    {"UINT32_MUL", 0x00},
    {"UINT32_EQ", 0x00},
    {"UINT32_ADDN", 0x04},
    {"UINT32_JUMPNZ", 0x02},
    {"UINT32_JUMPZ", 0x02},
    {"UINT32_MOVE_UINT8", 0x01},
    {"UINT32_PUSH", 0x04},
    {"UINT32_REG_GET", 0x01},
    {"UINT32_REG_SET", 0x01},
    {"FLOAT_POP", 0x01},
    {"FLOAT_DUP", 0x01},
    {"FLOAT_SWAP", 0x00},
    {"FLOAT_JUMPEM", 0x02},
    {"FLOAT_STORE", 0x00},
    {"FLOAT_LOAD", 0x00},
    {"FLOAT_ADD", 0x00},
    {"FLOAT_SUB", 0x00},
    {"FLOAT_MUL", 0x00},
    {"FLOAT_EQ", 0x00},
    {"FLOAT_ADDN", 0x04},
    {"FLOAT_JUMPNZ", 0x02},
    {"FLOAT_JUMPZ", 0x02},
    {"FLOAT_PUSH", 0x04},
    {"VEC2_POP", 0x01},
    {"VEC2_DUP", 0x01},
    {"VEC2_SWAP", 0x00},
    {"VEC2_JUMPEM", 0x02},
    {"VEC2_STORE", 0x00},
    {"VEC2_LOAD", 0x00},
    {"VEC2_ADD", 0x00},
    {"VEC2_SUB", 0x00},
    {"VEC2_MUL", 0x00},
    {"VEC2_EQ", 0x00},
    {"VEC2_EXPLODE", 0x00},
    {"VEC2_IMPLODE", 0x00},
    {"VEC2_MULMAT2", 0x00},
    {"VEC2_MULMAT3", 0x00},
    {"VEC2_MULMAT4", 0x00},
    {"VEC3_POP", 0x01},
    {"VEC3_DUP", 0x01},
    {"VEC3_SWAP", 0x00},
    {"VEC3_JUMPEM", 0x02},
    {"VEC3_STORE", 0x00},
    {"VEC3_LOAD", 0x00},
    {"VEC3_ADD", 0x00},
    {"VEC3_SUB", 0x00},
    {"VEC3_MUL", 0x00},
    {"VEC3_EQ", 0x00},
    {"VEC3_EXPLODE", 0x00},
    {"VEC3_IMPLODE", 0x00},
    {"VEC3_MULMAT3", 0x00},
    {"VEC3_MULMAT4", 0x00},
    {"VEC4_POP", 0x01},
    {"VEC4_DUP", 0x01},
    {"VEC4_SWAP", 0x00},
    {"VEC4_JUMPEM", 0x02},
    {"VEC4_STORE", 0x00},
    {"VEC4_LOAD", 0x00},
    {"VEC4_ADD", 0x00},
    {"VEC4_SUB", 0x00},
    {"VEC4_MUL", 0x00},
    {"VEC4_EQ", 0x00},
    {"VEC4_EXPLODE", 0x00},
    {"VEC4_IMPLODE", 0x00},
    {"VEC4_MULMAT4", 0x00},
    {"MAT2_POP", 0x01},
    {"MAT2_DUP", 0x01},
    {"MAT2_SWAP", 0x00},
    {"MAT2_JUMPEM", 0x02},
    {"MAT2_STORE", 0x00},
    {"MAT2_LOAD", 0x00},
    {"MAT2_ADD", 0x00},
    {"MAT2_SUB", 0x00},
    {"MAT2_MUL", 0x00},
    {"MAT2_EQ", 0x00},
    {"MAT2_EXPLODE", 0x00},
    {"MAT2_IDENT", 0x00},
    {"MAT2_IMPLODE", 0x00},
    {"MAT2_ROTATE", 0x00},
    {"MAT2_SCALE", 0x00},
    {"MAT2_TRANSP", 0x00},
    {"MAT3_POP", 0x01},
    {"MAT3_DUP", 0x01},
    {"MAT3_SWAP", 0x00},
    {"MAT3_JUMPEM", 0x02},
    {"MAT3_STORE", 0x00},
    {"MAT3_LOAD", 0x00},
    {"MAT3_ADD", 0x00},
    {"MAT3_SUB", 0x00},
    {"MAT3_MUL", 0x00},
    {"MAT3_EQ", 0x00},
    {"MAT3_EXPLODE", 0x00},
    {"MAT3_IDENT", 0x00},
    {"MAT3_IMPLODE", 0x00},
    {"MAT3_ROTATE", 0x00},
    {"MAT3_SCALE", 0x00},
    {"MAT3_TRANSL", 0x00},
    {"MAT3_TRANSP", 0x00},
    {"MAT4_POP", 0x01},
    {"MAT4_DUP", 0x01},
    {"MAT4_SWAP", 0x00},
    {"MAT4_JUMPEM", 0x02},
    {"MAT4_STORE", 0x00},
    {"MAT4_LOAD", 0x00},
    {"MAT4_ADD", 0x00},
    {"MAT4_SUB", 0x00},
    {"MAT4_MUL", 0x00},
    {"MAT4_EQ", 0x00},
    {"MAT4_EXPLODE", 0x00},
    {"MAT4_IDENT", 0x00},
    {"MAT4_IMPLODE", 0x00},
    {"MAT4_ROTATEX", 0x00},
    {"MAT4_ROTATEY", 0x00},
    {"MAT4_ROTATEZ", 0x00},
    {"MAT4_SCALE", 0x00},
    {"MAT4_TRANSL", 0x00},
    {"MAT4_TRANSP", 0x00},
    {"[unused]", 0x00}
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
            u80 = NCODE(vm);
            for (u81 = 0; u81 < u80; u81++) {
                u82 = UINT8_POP(vm);
            }
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
            if (u160 >= vm->uint8_memory_size) {
                rendervm_exception(vm, VM_X_OUT_OF_BOUNDS);
                break;
            }
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
        case VM_UINT8_ADDN:
            u80 = NCODE(vm);
            vm->uint8_stack[vm->uint8_sp] += u80;
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
            u80 = NCODE(vm);
            for (u81 = 0; u81 < u80; u81++) {
                u160 = UINT16_POP(vm);
            }
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
            if (u161 >= vm->uint16_memory_size) {
                rendervm_exception(vm, VM_X_OUT_OF_BOUNDS);
                break;
            }
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
        case VM_UINT16_ADDN:
            vm->b0 = NCODE(vm);
            vm->b1 = NCODE(vm);
            u160 = UINT16_MAKE(vm);
            vm->uint16_stack[vm->uint16_sp] += u160;
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
            UINT16_PUSH(vm, UINT8_POP(vm));
            break;
        case VM_UINT16_PUSH:
            vm->b0 = NCODE(vm);
            vm->b1 = NCODE(vm);
            u160 = UINT16_MAKE(vm);
            UINT16_PUSH(vm, u160);
            break;
        case VM_UINT32_POP:
            u80 = NCODE(vm);
            for (u81 = 0; u81 < u80; u81++) {
                u320 = UINT32_POP(vm);
            }
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
            if (u161 >= vm->uint32_memory_size) {
                rendervm_exception(vm, VM_X_OUT_OF_BOUNDS);
                break;
            }
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
        case VM_UINT32_ADDN:
            vm->b0 = NCODE(vm);
            vm->b1 = NCODE(vm);
            vm->b2 = NCODE(vm);
            vm->b3 = NCODE(vm);
            u320 = UINT32_MAKE(vm);
            vm->uint32_stack[vm->uint32_sp] += u320;
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
            UINT32_PUSH(vm, UINT8_POP(vm));
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
        case VM_UINT32_REG_GET:
            u80 = NCODE(vm);
            if (u80 > 9) break;
            UINT32_PUSH(vm, vm->draw_reg[u80]);
            break;
        case VM_UINT32_REG_SET:
            u80 = NCODE(vm);
            if (u80 > 9) break;
            vm->draw_reg[u80] = UINT32_POP(vm);
            break;
        case VM_FLOAT_POP:
            u80 = NCODE(vm);
            for (u81 = 0; u81 < u80; u81++) {
                fl0 = FLOAT_POP(vm);
            }
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
            if (u161 >= vm->float_memory_size) {
                rendervm_exception(vm, VM_X_OUT_OF_BOUNDS);
                break;
            }
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
        case VM_FLOAT_ADDN:
            vm->b0 = NCODE(vm);
            vm->b1 = NCODE(vm);
            vm->b2 = NCODE(vm);
            vm->b3 = NCODE(vm);
            fl0 = FLOAT_MAKE(vm);
            vm->float_stack[vm->float_sp] += fl0;
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
            u80 = NCODE(vm);
            for (u81 = 0; u81 < u80; u81++) {
                vm->vec2_sp -= 2;
            }
            break;
        case VM_VEC2_DUP:
            u80 = NCODE(vm);
            u80 = u80 * 2;
            if (u80 > 128) {
                // exception
                vm->running = 0;
                break;
            }
            u81 = vm->vec2_sp;
            for (u82 = u80; u82 > 0; u82--) {
                fl0 = VEC2_PEEK1(vm, (u81 - (u82 - 1)));
                VEC2_PUSH1(vm, fl0);
            }
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
            if (vm->callback != NULL) {
                vm->callback(vm, opcode, vm->user_data);
            }
            break;
    }
    vm->last_opcode = opcode;

    return vm->running;
}

uint8_t rendervm_has_exception(rendervm_t* vm) {
    return (VM_X_ALL_OK == vm->exception) ? 0 : 1;
}

void rendervm_exception(rendervm_t* vm, rendervm_exception_t exception) {
    vm->exception = exception;
    vm->running = 0;
}

void rendervm_attach_callback(rendervm_t* vm, rendervm_callback_t callback, void* user_data) {
    vm->callback = callback;
    vm->user_data = user_data;
}

uint8_t rendervm_opcode2arglen(rendervm_opcode_t opcode) {
    return opcode_info[opcode].arg_byte_length;
}

const char* rendervm_opcode2str(rendervm_opcode_t opcode) {
    return opcode_info[opcode].name;
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

