#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "vrms_render_vm.h"

/*
        loadi r0 2      ; Drawable object
        loadi r1 3      ; Matrix data object
        loadi r2 0      ; Index into matrix data
        loadmm r0 r1 r2
draw:   draw r0 r0
        framewait
        goto draw
*/

vrms_render_vm_t* vrms_render_vm_create() {
    vrms_render_vm_t* vm;
    vm = malloc(sizeof(vrms_render_vm_t));
    memset(vm, 0, sizeof(vrms_render_vm_t));
    vm->running = 1;
    return vm;
}

void vrms_render_vm_destroy(vrms_render_vm_t* vm) {
    free(vm);
}

uint32_t vrms_render_vm_iregister_value(vrms_render_vm_t* vm, uint8_t reg) {
    if (reg >= NUM_REGS) {
        return 0;
    }
    return vm->iregister[reg];
}

void vrms_render_vm_reset(vrms_render_vm_t* vm) {
    memset(vm, 0, sizeof(vrms_render_vm_t));
    vm->running = 1;
}

uint8_t vrms_render_vm_last_opcode(vrms_render_vm_t* vm) {
    return vm->last_opcode;
}

uint8_t vrms_render_vm_resume(vrms_render_vm_t* vm) {
    vm->running = 1;
    return 1;
}

void vrms_render_vm_load_matrix(vrms_render_vm_t* vm, uint8_t reg_mat, uint8_t reg_mem, uint8_t reg_off) {
    uint32_t memory_id;
    uint32_t offset;
    float* matrix;

    matrix = &vm->mregister[reg_mat * 16];
    memory_id = vm->iregister[reg_mem];
    offset = vm->iregister[reg_off];

    vm->load_matrix(matrix, memory_id, offset, vm->user_data);
}

void vrms_render_vm_draw(vrms_render_vm_t* vm, uint8_t reg_mat, uint8_t obj_reg) {
    uint32_t object_id;
    float* matrix;

    matrix = &vm->mregister[reg_mat * 16];
    object_id = vm->iregister[obj_reg];

    vm->draw(matrix, object_id, vm->user_data);
}

uint8_t vrms_render_vm_exec(vrms_render_vm_t* vm, uint8_t* program, uint32_t length) {
    uint8_t reg1;
    uint8_t reg2;
    uint8_t reg3;
    uint32_t ival1;
    uint8_t opcode;
    uint32_t PC;

    if (!vm->running) {
        return vm->running;
    }

    vm->running = 1;
    PC = vm->program_counter;
    opcode = program[PC];
    PC++;

    switch (opcode) {
        case VM_HALT:
            PC = 0;
            vm->running = 0;
            break;
        case VM_LOADI:
            reg1 = program[PC];
            PC++;
            ival1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                PC = 0;
                vm->running = 0;
                break;
            }
            vm->iregister[reg1] = ival1;
            break;
        case VM_LOADMM:
            reg1 = program[PC];
            PC++;
            reg2 = program[PC];
            PC++;
            reg3 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                PC = 0;
                vm->running = 0;
                break;
            }
            if (reg2 >= NUM_REGS) {
                PC = 0;
                vm->running = 0;
                break;
            }
            if (reg3 >= NUM_REGS) {
                PC = 0;
                vm->running = 0;
                break;
            }
            vrms_render_vm_load_matrix(vm, reg1, reg2, reg3);
            break;
        case VM_DRAW:
            reg1 = program[PC];
            PC++;
            reg2 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                PC = 0;
                vm->running = 0;
                break;
            }
            if (reg2 >= NUM_REGS) {
                PC = 0;
                vm->running = 0;
                break;
            }
            vrms_render_vm_draw(vm, reg1, reg2);
            break;
        case VM_FRWAIT:
            vm->running = 0;
            break;
        case VM_GOTO:
            reg1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                PC = 0;
                vm->running = 0;
                break;
            }
            ival1 = vm->iregister[reg1];
            PC = ival1;
            break;
    }

    vm->last_opcode = opcode;
    vm->program_counter = PC;

    return vm->running;
}
