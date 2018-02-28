#include <stdio.h>
#include <unistd.h>
#include "vrms_render_vm.h"
#include "test_harness.h"

float* load_matrix(vrms_render_vm_t* vm, uint32_t memory_id, uint32_t offset, void* user_data) {
    test_harness_t* test;
    test = (test_harness_t*)user_data;

    is_equal_uint32(test, memory_id, 3, "load_matrix call returns correct memory_id");
    is_equal_uint32(test, offset, 80, "load_matrix call returns correct offset");

    return NULL;
}

void draw(vrms_render_vm_t* vm, float* matrix, uint32_t object_id, void* user_data) {
    test_harness_t* test;
    test = (test_harness_t*)user_data;

    is_equal_uint32(test, object_id, 2, "draw call returns correct object_id");
}

void test_basic_program(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t running;

    uint8_t program[] = {
        VM_LOADI, VM_REG0, 2,
        VM_LOADI, VM_REG1, 3,
        VM_LOADI, VM_REG2, 80,
        VM_LOADI, VM_REG3, 16,
        VM_LOADMM, VM_REG0, VM_REG1, VM_REG2,
        VM_DRAW, VM_REG0, VM_REG0,
        VM_FRWAIT,
        VM_GOTO, 16
    };

    running = vrms_render_vm_exec(vm, program, 22);
    is_equal_uint8(test, running, 1, "running after LOADI REG0");
    is_equal_uint32(test, vrms_render_vm_iregister_value(vm, VM_REG0), 2, "REG0 value is correct");
    is_equal_uint8(test, vrms_render_vm_last_opcode(vm), VM_LOADI, "last opcode VM_LOADI");

    running = vrms_render_vm_exec(vm, program, 22);
    is_equal_uint8(test, running, 1, "running after LOADI REG1");
    is_equal_uint32(test, vrms_render_vm_iregister_value(vm, VM_REG1), 3, "REG1 value is correct");
    is_equal_uint8(test, vrms_render_vm_last_opcode(vm), VM_LOADI, "last opcode VM_LOADI");

    running = vrms_render_vm_exec(vm, program, 22);
    is_equal_uint8(test, running, 1, "running after LOADI REG2");
    is_equal_uint32(test, vrms_render_vm_iregister_value(vm, VM_REG2), 80, "REG2 value is correct");
    is_equal_uint8(test, vrms_render_vm_last_opcode(vm), VM_LOADI, "last opcode VM_LOADI");

    running = vrms_render_vm_exec(vm, program, 22);
    is_equal_uint8(test, running, 1, "running after LOADI REG3");
    is_equal_uint32(test, vrms_render_vm_iregister_value(vm, VM_REG3), 16, "REG3 value is correct");
    is_equal_uint8(test, vrms_render_vm_last_opcode(vm), VM_LOADI, "last opcode VM_LOADI");

    running = vrms_render_vm_exec(vm, program, 22);
    is_equal_uint8(test, running, 1, "running after LOADMM");
    is_equal_uint8(test, vrms_render_vm_last_opcode(vm), VM_LOADMM, "last opcode VM_LOADMM");

    running = vrms_render_vm_exec(vm, program, 22);
    is_equal_uint8(test, running, 1, "running after DRAW");
    is_equal_uint8(test, vrms_render_vm_last_opcode(vm), VM_DRAW, "last opcode VM_DRAW");

    running = vrms_render_vm_exec(vm, program, 22);
    is_equal_uint8(test, running, 0, "NOT running after FRWAIT");
    is_equal_uint8(test, vrms_render_vm_last_opcode(vm), VM_FRWAIT, "last opcode VM_FRWAIT");

    running = vrms_render_vm_exec(vm, program, 22);
    is_equal_uint8(test, running, 0, "still not running after FRWAIT");
    is_equal_uint8(test, vrms_render_vm_last_opcode(vm), VM_FRWAIT, "last opcode VM_FRWAIT");

    running = vrms_render_vm_resume(vm);
    running = vrms_render_vm_exec(vm, program, 22);
    is_equal_uint8(test, running, 1, "running after GOTO");
    is_equal_uint8(test, vrms_render_vm_last_opcode(vm), VM_GOTO, "last opcode VM_GOTO");

    running = vrms_render_vm_exec(vm, program, 22);
    is_equal_uint8(test, running, 1, "running after DRAW (from GOTO)");
    is_equal_uint8(test, vrms_render_vm_last_opcode(vm), VM_DRAW, "last opcode VM_DRAW");

    vrms_render_vm_reset(vm);
    is_equal_uint8(test, vrms_render_vm_last_opcode(vm), 0, "last opcode after reset zero");
    is_equal_uint32(test, vrms_render_vm_iregister_value(vm, VM_REG0), 0, "REG0 was reset");
    is_equal_uint32(test, vrms_render_vm_iregister_value(vm, VM_REG1), 0, "REG1 was reset");
    is_equal_uint32(test, vrms_render_vm_iregister_value(vm, VM_REG2), 0, "REG2 was reset");
    is_equal_uint32(test, vrms_render_vm_iregister_value(vm, VM_REG3), 0, "REG3 was reset");
}

void test_loop_program(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t running;
    uint8_t nr_loops;
    uint8_t i;

    nr_loops = 5;
    uint8_t program[] = {
        VM_LOADI, VM_REG0, 5,
        VM_DEC, VM_REG0,
        VM_JNZ, VM_REG0, 3
    };

    running = vrms_render_vm_exec(vm, program, 7);
    is_equal_uint8(test, running, 1, "running after LOADI");
    is_equal_uint32(test, vrms_render_vm_iregister_value(vm, VM_REG0), nr_loops, "REG0 was set to 5");

    for (i = (nr_loops - 1); i > 0; i--) {
        running = vrms_render_vm_exec(vm, program, 7);
        is_equal_uint8(test, running, 1, "running after DEC");
        is_equal_uint32(test, vrms_render_vm_iregister_value(vm, VM_REG0), i, "REG0 value correct");

        running = vrms_render_vm_exec(vm, program, 7);
        is_equal_uint8(test, running, 1, "running after JNZ");
    }

    running = vrms_render_vm_exec(vm, program, 7);
    is_equal_uint8(test, running, 1, "running after DEC");
    is_equal_uint32(test, vrms_render_vm_iregister_value(vm, VM_REG0), 0, "REG0 0");

    running = vrms_render_vm_exec(vm, program, 7);
    is_equal_uint8(test, running, 1, "running after JNZ");

    // Program falls off the end here and should stop
    running = vrms_render_vm_exec(vm, program, 7);
    is_equal_uint8(test, running, 0, "program correctly stopped after loop ends");
}

int main(void) {
    test_harness_t* test;
    vrms_render_vm_t* vm;
    
    test = test_harness_create();
    test->verbose = 1;

    vm = vrms_render_vm_create();
    vm->load_matrix = &load_matrix;
    vm->draw = &draw;
    vm->user_data = (void*)test;

    test_basic_program(test, vm);
    test_loop_program(test, vm);

    test_harness_exit_with_status(test);
}
