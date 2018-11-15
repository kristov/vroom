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

void print_uint8_stack(rendervm_t* vm) {
    uint16_t index = 0;
    if (vm->uint8_sp == 65535) {
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
    if (vm->uint16_sp == 65535) {
        return;
    }
    printf("  uint16 st: [ ");
    for (index = 0; index <= vm->uint16_sp; index++) {
        printf("0x%02x ", vm->uint16_stack[index]);
    }
    printf("]\n");
}

void print_memory(uint8_t* memory) {
    uint8_t x, y;
    uint8_t idx, val;
    idx = 0;
    for (y = 0; y < 5; y++) {
        for (x = 0; x < 5; x++) {
            idx = (y * 5) + x;
            val = memory[idx];
            switch (val) {
                case 1:
                    printf("1 ");
                    break;
                case 0:
                    printf(". ");
                    break;
                case 10:
                    printf("x ");
                    break;
                default:
                    break;
            }
        }
        printf("\n");
    }
    printf("\n");
}

void print_vm(rendervm_t* vm) {
    printf("         pc: 0x%02x\n", vm->pc);
    printf("last opcode: %s\n", rendervm_opcode2str(vm->last_opcode));
    printf("   uint8_sp: 0x%02x\n", vm->uint8_sp);
    printf("  uint16_sp: 0x%02x\n", vm->uint16_sp);
    print_uint8_stack(vm);
    print_uint16_stack(vm);
    printf("\n");
}

void test_flood_fill(test_harness_t* test, rendervm_t* vm) {
	// A 5x5 map, plus 5 variables. The first two are the
	// player location (4,4), the second two tmp coords, and
	// The final is a tmp variable for the tile value.
    uint8_t uint8_memory[30] = {
        1, 1, 1, 1, 1,
        1, 0, 0, 0, 1,
        1, 0, 0, 0, 1,
        1, 0, 0, 0, 1,
        1, 1, 1, 1, 1,
        2, 2, 0, 0, 0
    };

    uint8_t program[201] = {
        0x03, 0x19, 0x00, 0x01, 0x19, 0x02, 0x00, 0x09,
        0x05, 0x05, 0x00, 0x08, 0x09, 0x07, 0x17, 0x01,
        0x00, 0x0f, 0x0c, 0x1d, 0x00, 0x0a, 0x0a, 0x18,
        0x04, 0x0d, 0x19, 0x00, 0x0d, 0x1a, 0x00, 0x03,
        0x04, 0x00, 0x16, 0xc8, 0x00, 0x0c, 0x1b, 0x00,
        0x0c, 0x1c, 0x00, 0x0d, 0x1b, 0x00, 0x0d, 0x1c,
        0x00, 0x0a, 0x01, 0x11, 0x03, 0x04, 0x00, 0x0d,
        0x1d, 0x00, 0x0a, 0x00, 0x12, 0x13, 0x51, 0x00,
        0x0d, 0x1d, 0x00, 0x0a, 0x0a, 0x12, 0x13, 0x4f,
        0x00, 0x05, 0x04, 0x00, 0x15, 0x02, 0x00, 0x0b,
        0x0b, 0x0d, 0x1b, 0x00, 0x0d, 0x1c, 0x00, 0x0a,
        0x01, 0x0e, 0x03, 0x04, 0x00, 0x0d, 0x1d, 0x00,
        0x0a, 0x00, 0x12, 0x13, 0x77, 0x00, 0x0d, 0x1d,
        0x00, 0x0a, 0x0a, 0x12, 0x13, 0x75, 0x00, 0x05,
        0x01, 0x00, 0x15, 0x02, 0x00, 0x0b, 0x0b, 0x0d,
        0x1c, 0x00, 0x0d, 0x1b, 0x00, 0x0a, 0x01, 0x11,
        0x14, 0x03, 0x04, 0x00, 0x0d, 0x1d, 0x00, 0x0a,
        0x00, 0x12, 0x13, 0x9e, 0x00, 0x0d, 0x1d, 0x00,
        0x0a, 0x0a, 0x12, 0x13, 0x9c, 0x00, 0x05, 0x02,
        0x00, 0x15, 0x02, 0x00, 0x0b, 0x0b, 0x0d, 0x1c,
        0x00, 0x0d, 0x1b, 0x00, 0x0a, 0x01, 0x0e, 0x14,
        0x03, 0x04, 0x00, 0x0d, 0x1d, 0x00, 0x0a, 0x00,
        0x12, 0x13, 0x22, 0x00, 0x0d, 0x1d, 0x00, 0x0a,
        0x0a, 0x12, 0x13, 0xc3, 0x00, 0x05, 0x08, 0x00,
        0x15, 0x02, 0x00, 0x0b, 0x0b, 0x1a, 0x22, 0x00,
        0x04
    };

    vm->uint8_memory = &uint8_memory[0];

    uint32_t cycle = 0;
    for (cycle = 0; cycle < 1018; cycle++) {
        rendervm_exec(vm, program, 201);
		//if (cycle > 1000) {
        //	printf("CYCLE: %d\n", cycle);
        //	print_vm(vm);
        //    print_memory(vm->uint8_memory);
        //}
    }
    rendervm_reset(vm);
}

void test_opcode_VM_HALT(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_HALT};
    rendervm_exec(vm, program, 1);
    is_equal_uint8(test, vm->running, 0, "OP VM_HALT: stops VM");
    is_equal_uint16(test, vm->pc, 0, "OP VM_HALT: zeros pc");
    rendervm_reset(vm);
}

void test_opcode_VM_YIELD(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_YIELD};
    rendervm_exec(vm, program, 1);
    is_equal_uint8(test, vm->running, 0, "OP VM_YIELD: stops VM");
    is_equal_uint16(test, vm->pc, 1, "OP VM_YIELD: leaves pc");
    rendervm_reset(vm);
}

void test_opcode_VM_RESET(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_RESET};
    rendervm_exec(vm, program, 1);
    is_equal_uint8(test, vm->running, 1, "OP VM_RESET: leaves VM running");
    is_equal_uint16(test, vm->pc, 0, "OP VM_RESET: zeros pc");
    rendervm_reset(vm);
}

void test_opcode_VM_CALL(test_harness_t* test, rendervm_t* vm) {
    uint8_t program[] = {VM_CALL, 0xbb, 0x01};
    rendervm_exec(vm, program, 3);
    is_equal_uint8(test, vm->running, 1, "OP VM_CALL: leaves VM running");
    is_equal_uint16(test, vm->pc, 443, "OP VM_CALL: pc correct");
    rendervm_reset(vm);
}

void test_all_opcodes(test_harness_t* test, rendervm_t* vm) {
    test_opcode_VM_HALT(test, vm);
    test_opcode_VM_YIELD(test, vm);
    test_opcode_VM_RESET(test, vm);
    test_opcode_VM_CALL(test, vm);
}

int main(void) {
    test_harness_t* test;
    rendervm_t* vm;
    
    test = test_harness_create();
    test->verbose = 1;

    vm = rendervm_create();
    test_all_opcodes(test, vm);
    test_flood_fill(test, vm);

    test_harness_exit_with_status(test);
}
