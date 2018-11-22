#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include "rendervm.h"
#include "test_harness.h"

/*
float _R4DP(float v) {
    v = v * 10000;
    v = floor(v + 0.5f);
    return v / 10000;
}
*/

void print_ctrl_stack(rendervm_t* vm) {
    uint16_t index = 0;
    if (vm->ctrl_sp == VM_MAX_ADDR) {
        return;
    }
    printf("    ctrl st: [ ");
    for (index = 0; index <= vm->ctrl_sp; index++) {
        printf("0x%02x ", vm->ctrl_stack[index]);
    }
    printf("]\n");
}

void print_uint8_stack(rendervm_t* vm) {
    uint16_t index = 0;
    if (vm->uint8_sp == VM_MAX_ADDR) {
        return;
    }
    printf("   uint8 st: [ ");
    for (index = 0; index <= vm->uint8_sp; index++) {
        printf("0x%02x ", vm->uint8_stack[index]);
    }
    printf("]\n");
}

void print_uint16_stack(rendervm_t* vm) {
    uint16_t index = 0;
    if (vm->uint16_sp == VM_MAX_ADDR) {
        return;
    }
    printf("  uint16 st: [ ");
    for (index = 0; index <= vm->uint16_sp; index++) {
        printf("0x%02x ", vm->uint16_stack[index]);
    }
    printf("]\n");
}

void print_vm(rendervm_t* vm) {
    printf("         pc: 0x%02x\n", vm->pc);
    printf("last opcode: %s\n", rendervm_opcode2str(vm->last_opcode));
    printf("   uint8_sp: 0x%02x\n", vm->uint8_sp);
    printf("  uint16_sp: 0x%02x\n", vm->uint16_sp);
    print_ctrl_stack(vm);
    print_uint8_stack(vm);
    print_uint16_stack(vm);
    printf("\n");
}

void test_opcode_HALT(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_HALT};
    rendervm_exec(vm, program, 1);
    is_equal_uint8(test, vm->running, 0, "OP HALT: stops VM");
    rendervm_reset(vm);
}

void test_opcode_YIELD(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_YIELD};
    rendervm_exec(vm, program, 1);
    is_equal_uint8(test, vm->running, 0, "OP YIELD: stops VM");
    is_equal_uint16(test, vm->pc, 1, "OP YIELD: leaves pc");
    rendervm_reset(vm);
}

void test_opcode_RESET(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_RESET};
    rendervm_exec(vm, program, 1);
    is_equal_uint8(test, vm->running, 1, "OP RESET: leaves VM running");
    is_equal_uint16(test, vm->pc, 0, "OP RESET: zeros pc");
    rendervm_reset(vm);
}

void test_opcode_CALL(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_CALL, 0xbb, 0x01};
    rendervm_exec(vm, program, 3);
    is_equal_uint8(test, vm->running, 1, "OP CALL: leaves VM running");
    is_equal_uint16(test, vm->pc, 443, "OP CALL: pc correct");
    is_equal_uint16(test, vm->ctrl_stack[0], 3, "OP CALL: return address correct");
    rendervm_reset(vm);
}

void test_opcode_RETURN(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_CALL, 0x04, 0x00, VM_HALT, VM_RETURN};
    rendervm_exec(vm, program, 5);
    is_equal_uint16(test, vm->pc, 4, "OP RETURN: pc correct");
    rendervm_exec(vm, program, 5);
    is_equal_uint8(test, vm->last_opcode, VM_RETURN, "OP RETURN: last opcode is RETURN");
    is_equal_uint16(test, vm->pc, 3, "OP RETURN: pc correct");
    rendervm_reset(vm);
}

void test_opcode_JUMP(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_JUMP, 0xbb, 0x01};
    rendervm_exec(vm, program, 1);
    is_equal_uint16(test, vm->pc, 443, "OP JUMP: pc correct");
    rendervm_reset(vm);
}

void test_opcode_UINT8_POP(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_UINT8_POP};
    vm->uint8_sp++;
    vm->uint8_stack[0] = 0x05;
    rendervm_exec(vm, program, 1);
    is_equal_uint16(test, vm->uint8_sp, VM_MAX_ADDR, "OP UINT8_POP: uint8_sp correct");
    rendervm_reset(vm);
}

void test_opcode_UINT8_DUP(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_UINT8_DUP, 0x01, 0x00};
    vm->uint8_sp++;
    vm->uint8_stack[0] = 0x06;
    rendervm_exec(vm, program, 3);
    is_equal_uint16(test, vm->uint8_sp, 0x01, "OP UINT8_DUP: uint8_sp correct");
    is_equal_uint8(test, vm->uint8_stack[0], 0x06, "OP UINT8_DUP: first stack item correct");
    is_equal_uint8(test, vm->uint8_stack[1], 0x06, "OP UINT8_DUP: second stack item correct");
    rendervm_reset(vm);
}

void test_opcode_UINT8_SWAP(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_UINT8_SWAP};
    vm->uint8_sp++;
    vm->uint8_stack[0] = 0x07;
    vm->uint8_sp++;
    vm->uint8_stack[1] = 0x08;
    rendervm_exec(vm, program, 1);
    is_equal_uint16(test, vm->uint8_sp, 0x01, "OP UINT8_SWAP: uint8_sp correct");
    is_equal_uint8(test, vm->uint8_stack[0], 0x08, "OP UINT8_SWAP: first stack item correct");
    is_equal_uint8(test, vm->uint8_stack[1], 0x07, "OP UINT8_SWAP: second stack item correct");
    rendervm_reset(vm);
}

void test_opcode_UINT8_JUMPEM(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_UINT8_JUMPEM, 0x0a, 0x00};
    rendervm_exec(vm, program, 3);
    is_equal_uint16(test, vm->pc, 10, "OP UINT8_JUMPEM: pc correct");
    rendervm_reset(vm);
}

void test_opcode_UINT8_STORE(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_UINT8_STORE};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_UINT8_LOAD(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_UINT8_LOAD};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_UINT8_ADD(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_UINT8_ADD};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_UINT8_SUB(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_UINT8_SUB};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_UINT8_MUL(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_UINT8_MUL};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_UINT8_EQ(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_UINT8_EQ};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_UINT8_JUMPNZ(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_UINT8_JUMPNZ};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_UINT8_JUMPZ(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_UINT8_JUMPZ};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_UINT8_PUSH(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_UINT8_PUSH};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_UINT16_POP(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_UINT16_POP};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_UINT16_DUP(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_UINT16_DUP};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_UINT16_SWAP(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_UINT16_SWAP};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_UINT16_JUMPEM(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_UINT16_JUMPEM};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_UINT16_STORE(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_UINT16_STORE};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_UINT16_LOAD(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_UINT16_LOAD};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_UINT16_ADD(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_UINT16_ADD};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_UINT16_SUB(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_UINT16_SUB};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_UINT16_MUL(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_UINT16_MUL};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_UINT16_EQ(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_UINT16_EQ};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_UINT16_JUMPNZ(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_UINT16_JUMPNZ};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_UINT16_JUMPZ(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_UINT16_JUMPZ};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_UINT16_PUSH(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_UINT16_PUSH};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_UINT32_POP(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_UINT32_POP};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_UINT32_DUP(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_UINT32_DUP};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_UINT32_SWAP(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_UINT32_SWAP};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_UINT32_JUMPEM(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_UINT32_JUMPEM};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_UINT32_STORE(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_UINT32_STORE};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_UINT32_LOAD(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_UINT32_LOAD};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_UINT32_ADD(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_UINT32_ADD};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_UINT32_SUB(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_UINT32_SUB};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_UINT32_MUL(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_UINT32_MUL};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_UINT32_EQ(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_UINT32_EQ};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_UINT32_JUMPNZ(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_UINT32_JUMPNZ};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_UINT32_JUMPZ(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_UINT32_JUMPZ};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_UINT32_PUSH(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_UINT32_PUSH};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_FLOAT_POP(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_FLOAT_POP};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_FLOAT_DUP(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_FLOAT_DUP};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_FLOAT_SWAP(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_FLOAT_SWAP};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_FLOAT_JUMPEM(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_FLOAT_JUMPEM};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_FLOAT_STORE(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_FLOAT_STORE};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_FLOAT_LOAD(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_FLOAT_LOAD};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_FLOAT_ADD(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_FLOAT_ADD};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_FLOAT_SUB(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_FLOAT_SUB};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_FLOAT_MUL(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_FLOAT_MUL};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_FLOAT_EQ(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_FLOAT_EQ};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_FLOAT_JUMPNZ(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_FLOAT_JUMPNZ};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_FLOAT_JUMPZ(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_FLOAT_JUMPZ};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_FLOAT_PUSH(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_FLOAT_PUSH};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_VEC2_POP(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_VEC2_POP};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_VEC2_DUP(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_VEC2_DUP};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_VEC2_SWAP(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_VEC2_SWAP};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_VEC2_JUMPEM(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_VEC2_JUMPEM};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_VEC2_STORE(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_VEC2_STORE};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_VEC2_LOAD(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_VEC2_LOAD};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_VEC2_ADD(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_VEC2_ADD};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_VEC2_SUB(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_VEC2_SUB};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_VEC2_MUL(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_VEC2_MUL};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_VEC2_EQ(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_VEC2_EQ};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_VEC2_EXPLODE(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_VEC2_EXPLODE};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_VEC2_IMPLODE(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_VEC2_IMPLODE};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_VEC2_MULMAT2(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_VEC2_MULMAT2};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_VEC2_MULMAT3(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_VEC2_MULMAT3};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_VEC2_MULMAT4(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_VEC2_MULMAT4};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_VEC3_POP(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_VEC3_POP};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_VEC3_DUP(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_VEC3_DUP};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_VEC3_SWAP(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_VEC3_SWAP};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_VEC3_JUMPEM(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_VEC3_JUMPEM};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_VEC3_STORE(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_VEC3_STORE};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_VEC3_LOAD(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_VEC3_LOAD};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_VEC3_ADD(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_VEC3_ADD};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_VEC3_SUB(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_VEC3_SUB};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_VEC3_MUL(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_VEC3_MUL};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_VEC3_EQ(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_VEC3_EQ};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_VEC3_EXPLODE(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_VEC3_EXPLODE};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_VEC3_IMPLODE(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_VEC3_IMPLODE};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_VEC3_MULMAT3(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_VEC3_MULMAT3};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_VEC3_MULMAT4(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_VEC3_MULMAT4};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_VEC4_POP(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_VEC4_POP};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_VEC4_DUP(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_VEC4_DUP};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_VEC4_SWAP(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_VEC4_SWAP};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_VEC4_JUMPEM(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_VEC4_JUMPEM};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_VEC4_STORE(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_VEC4_STORE};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_VEC4_LOAD(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_VEC4_LOAD};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_VEC4_ADD(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_VEC4_ADD};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_VEC4_SUB(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_VEC4_SUB};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_VEC4_MUL(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_VEC4_MUL};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_VEC4_EQ(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_VEC4_EQ};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_VEC4_EXPLODE(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_VEC4_EXPLODE};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_VEC4_IMPLODE(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_VEC4_IMPLODE};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_VEC4_MULMAT4(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_VEC4_MULMAT4};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT2_POP(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT2_POP};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT2_DUP(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT2_DUP};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT2_SWAP(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT2_SWAP};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT2_JUMPEM(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT2_JUMPEM};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT2_STORE(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT2_STORE};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT2_LOAD(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT2_LOAD};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT2_ADD(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT2_ADD};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT2_SUB(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT2_SUB};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT2_MUL(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT2_MUL};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT2_EQ(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT2_EQ};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT2_EXPLODE(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT2_EXPLODE};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT2_IDENT(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT2_IDENT};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT2_IMPLODE(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT2_IMPLODE};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT2_ROTATE(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT2_ROTATE};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT2_SCALE(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT2_SCALE};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT2_TRANSP(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT2_TRANSP};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT3_POP(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT3_POP};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT3_DUP(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT3_DUP};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT3_SWAP(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT3_SWAP};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT3_JUMPEM(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT3_JUMPEM};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT3_STORE(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT3_STORE};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT3_LOAD(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT3_LOAD};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT3_ADD(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT3_ADD};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT3_SUB(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT3_SUB};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT3_MUL(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT3_MUL};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT3_EQ(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT3_EQ};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT3_EXPLODE(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT3_EXPLODE};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT3_IDENT(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT3_IDENT};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT3_IMPLODE(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT3_IMPLODE};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT3_ROTATE(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT3_ROTATE};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT3_SCALE(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT3_SCALE};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT3_TRANSL(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT3_TRANSL};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT3_TRANSP(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT3_TRANSP};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT4_POP(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT4_POP};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT4_DUP(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT4_DUP};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT4_SWAP(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT4_SWAP};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT4_JUMPEM(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT4_JUMPEM};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT4_STORE(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT4_STORE};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT4_LOAD(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT4_LOAD};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT4_ADD(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT4_ADD};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT4_SUB(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT4_SUB};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT4_MUL(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT4_MUL};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT4_EQ(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT4_EQ};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT4_EXPLODE(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT4_EXPLODE};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT4_IDENT(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT4_IDENT};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT4_IMPLODE(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT4_IMPLODE};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT4_ROTATEX(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT4_ROTATEX};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT4_ROTATEY(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT4_ROTATEY};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT4_ROTATEZ(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT4_ROTATEZ};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT4_SCALE(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT4_SCALE};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT4_TRANSL(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT4_TRANSL};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_opcode_MAT4_TRANSP(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_MAT4_TRANSP};
    rendervm_exec(vm, program, 1);
    rendervm_reset(vm);
}

void test_all_opcodes(test_harness_t* test, rendervm_t* vm) {
    test_opcode_HALT(test, vm);
    test_opcode_YIELD(test, vm);
    test_opcode_RESET(test, vm);
    test_opcode_CALL(test, vm);
    test_opcode_RETURN(test, vm);
    test_opcode_JUMP(test, vm);
    test_opcode_UINT8_POP(test, vm);
    test_opcode_UINT8_DUP(test, vm);
    test_opcode_UINT8_SWAP(test, vm);
    test_opcode_UINT8_JUMPEM(test, vm);
    return;
    test_opcode_UINT8_STORE(test, vm);
    test_opcode_UINT8_LOAD(test, vm);
    test_opcode_UINT8_ADD(test, vm);
    test_opcode_UINT8_SUB(test, vm);
    test_opcode_UINT8_MUL(test, vm);
    test_opcode_UINT8_EQ(test, vm);
    test_opcode_UINT8_JUMPNZ(test, vm);
    test_opcode_UINT8_JUMPZ(test, vm);
    test_opcode_UINT8_PUSH(test, vm);
    test_opcode_UINT16_POP(test, vm);
    test_opcode_UINT16_DUP(test, vm);
    test_opcode_UINT16_SWAP(test, vm);
    test_opcode_UINT16_JUMPEM(test, vm);
    test_opcode_UINT16_STORE(test, vm);
    test_opcode_UINT16_LOAD(test, vm);
    test_opcode_UINT16_ADD(test, vm);
    test_opcode_UINT16_SUB(test, vm);
    test_opcode_UINT16_MUL(test, vm);
    test_opcode_UINT16_EQ(test, vm);
    test_opcode_UINT16_JUMPNZ(test, vm);
    test_opcode_UINT16_JUMPZ(test, vm);
    test_opcode_UINT16_PUSH(test, vm);
    test_opcode_UINT32_POP(test, vm);
    test_opcode_UINT32_DUP(test, vm);
    test_opcode_UINT32_SWAP(test, vm);
    test_opcode_UINT32_JUMPEM(test, vm);
    test_opcode_UINT32_STORE(test, vm);
    test_opcode_UINT32_LOAD(test, vm);
    test_opcode_UINT32_ADD(test, vm);
    test_opcode_UINT32_SUB(test, vm);
    test_opcode_UINT32_MUL(test, vm);
    test_opcode_UINT32_EQ(test, vm);
    test_opcode_UINT32_JUMPNZ(test, vm);
    test_opcode_UINT32_JUMPZ(test, vm);
    test_opcode_UINT32_PUSH(test, vm);
    test_opcode_FLOAT_POP(test, vm);
    test_opcode_FLOAT_DUP(test, vm);
    test_opcode_FLOAT_SWAP(test, vm);
    test_opcode_FLOAT_JUMPEM(test, vm);
    test_opcode_FLOAT_STORE(test, vm);
    test_opcode_FLOAT_LOAD(test, vm);
    test_opcode_FLOAT_ADD(test, vm);
    test_opcode_FLOAT_SUB(test, vm);
    test_opcode_FLOAT_MUL(test, vm);
    test_opcode_FLOAT_EQ(test, vm);
    test_opcode_FLOAT_JUMPNZ(test, vm);
    test_opcode_FLOAT_JUMPZ(test, vm);
    test_opcode_FLOAT_PUSH(test, vm);
    test_opcode_VEC2_POP(test, vm);
    test_opcode_VEC2_DUP(test, vm);
    test_opcode_VEC2_SWAP(test, vm);
    test_opcode_VEC2_JUMPEM(test, vm);
    test_opcode_VEC2_STORE(test, vm);
    test_opcode_VEC2_LOAD(test, vm);
    test_opcode_VEC2_ADD(test, vm);
    test_opcode_VEC2_SUB(test, vm);
    test_opcode_VEC2_MUL(test, vm);
    test_opcode_VEC2_EQ(test, vm);
    test_opcode_VEC2_EXPLODE(test, vm);
    test_opcode_VEC2_IMPLODE(test, vm);
    test_opcode_VEC2_MULMAT2(test, vm);
    test_opcode_VEC2_MULMAT3(test, vm);
    test_opcode_VEC2_MULMAT4(test, vm);
    test_opcode_VEC3_POP(test, vm);
    test_opcode_VEC3_DUP(test, vm);
    test_opcode_VEC3_SWAP(test, vm);
    test_opcode_VEC3_JUMPEM(test, vm);
    test_opcode_VEC3_STORE(test, vm);
    test_opcode_VEC3_LOAD(test, vm);
    test_opcode_VEC3_ADD(test, vm);
    test_opcode_VEC3_SUB(test, vm);
    test_opcode_VEC3_MUL(test, vm);
    test_opcode_VEC3_EQ(test, vm);
    test_opcode_VEC3_EXPLODE(test, vm);
    test_opcode_VEC3_IMPLODE(test, vm);
    test_opcode_VEC3_MULMAT3(test, vm);
    test_opcode_VEC3_MULMAT4(test, vm);
    test_opcode_VEC4_POP(test, vm);
    test_opcode_VEC4_DUP(test, vm);
    test_opcode_VEC4_SWAP(test, vm);
    test_opcode_VEC4_JUMPEM(test, vm);
    test_opcode_VEC4_STORE(test, vm);
    test_opcode_VEC4_LOAD(test, vm);
    test_opcode_VEC4_ADD(test, vm);
    test_opcode_VEC4_SUB(test, vm);
    test_opcode_VEC4_MUL(test, vm);
    test_opcode_VEC4_EQ(test, vm);
    test_opcode_VEC4_EXPLODE(test, vm);
    test_opcode_VEC4_IMPLODE(test, vm);
    test_opcode_VEC4_MULMAT4(test, vm);
    test_opcode_MAT2_POP(test, vm);
    test_opcode_MAT2_DUP(test, vm);
    test_opcode_MAT2_SWAP(test, vm);
    test_opcode_MAT2_JUMPEM(test, vm);
    test_opcode_MAT2_STORE(test, vm);
    test_opcode_MAT2_LOAD(test, vm);
    test_opcode_MAT2_ADD(test, vm);
    test_opcode_MAT2_SUB(test, vm);
    test_opcode_MAT2_MUL(test, vm);
    test_opcode_MAT2_EQ(test, vm);
    test_opcode_MAT2_EXPLODE(test, vm);
    test_opcode_MAT2_IDENT(test, vm);
    test_opcode_MAT2_IMPLODE(test, vm);
    test_opcode_MAT2_ROTATE(test, vm);
    test_opcode_MAT2_SCALE(test, vm);
    test_opcode_MAT2_TRANSP(test, vm);
    test_opcode_MAT3_POP(test, vm);
    test_opcode_MAT3_DUP(test, vm);
    test_opcode_MAT3_SWAP(test, vm);
    test_opcode_MAT3_JUMPEM(test, vm);
    test_opcode_MAT3_STORE(test, vm);
    test_opcode_MAT3_LOAD(test, vm);
    test_opcode_MAT3_ADD(test, vm);
    test_opcode_MAT3_SUB(test, vm);
    test_opcode_MAT3_MUL(test, vm);
    test_opcode_MAT3_EQ(test, vm);
    test_opcode_MAT3_EXPLODE(test, vm);
    test_opcode_MAT3_IDENT(test, vm);
    test_opcode_MAT3_IMPLODE(test, vm);
    test_opcode_MAT3_ROTATE(test, vm);
    test_opcode_MAT3_SCALE(test, vm);
    test_opcode_MAT3_TRANSL(test, vm);
    test_opcode_MAT3_TRANSP(test, vm);
    test_opcode_MAT4_POP(test, vm);
    test_opcode_MAT4_DUP(test, vm);
    test_opcode_MAT4_SWAP(test, vm);
    test_opcode_MAT4_JUMPEM(test, vm);
    test_opcode_MAT4_STORE(test, vm);
    test_opcode_MAT4_LOAD(test, vm);
    test_opcode_MAT4_ADD(test, vm);
    test_opcode_MAT4_SUB(test, vm);
    test_opcode_MAT4_MUL(test, vm);
    test_opcode_MAT4_EQ(test, vm);
    test_opcode_MAT4_EXPLODE(test, vm);
    test_opcode_MAT4_IDENT(test, vm);
    test_opcode_MAT4_IMPLODE(test, vm);
    test_opcode_MAT4_ROTATEX(test, vm);
    test_opcode_MAT4_ROTATEY(test, vm);
    test_opcode_MAT4_ROTATEZ(test, vm);
    test_opcode_MAT4_SCALE(test, vm);
    test_opcode_MAT4_TRANSL(test, vm);
    test_opcode_MAT4_TRANSP(test, vm);
}

int main(void) {
    test_harness_t* test;
    rendervm_t* vm;
    
    test = test_harness_create();
    test->verbose = 1;

    vm = rendervm_create();
    test_all_opcodes(test, vm);

    test_harness_exit_with_status(test);
}
