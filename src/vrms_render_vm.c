#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <esm.h>
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

uint32_t vrms_render_vm_iregister_get(vrms_render_vm_t* vm, uint8_t reg) {
    if (reg >= NUM_REGS) {
        return 0;
    }
    return vm->iregister[reg];
}

void vrms_render_vm_iregister_set(vrms_render_vm_t* vm, uint8_t reg, uint32_t value) {
    if (reg >= NUM_REGS) {
        return;
    }
    vm->iregister[reg] = value;
}

float vrms_render_vm_fregister_get(vrms_render_vm_t* vm, uint8_t reg) {
    if (reg >= NUM_REGS) {
        return 0;
    }
    return vm->fregister[reg];
}

void vrms_render_vm_fregister_set(vrms_render_vm_t* vm, uint8_t reg, float value) {
    if (reg >= NUM_REGS) {
        return;
    }
    vm->fregister[reg] = value;
}

float vrms_render_vm_vregister_get(vrms_render_vm_t* vm, uint8_t reg, vrms_render_vm_vreg_part_t part) {
    if (reg >= NUM_REGS) {
        return 0;
    }
    reg = reg * 3;
    return vm->vregister[reg + part];
}

void vrms_render_vm_vregister_set(vrms_render_vm_t* vm, uint8_t reg, vrms_render_vm_vreg_part_t part, float value) {
    if (reg >= NUM_REGS) {
        return;
    }
    reg = reg * 3;
    vm->vregister[reg + part] = value;
}

uint32_t vrms_render_vm_sysiregister_get(vrms_render_vm_t* vm, vrms_render_vm_sysireg_t reg) {
    if ((uint8_t)reg >= NUM_REGS) {
        return 0;
    }
    return vm->sysiregister[(uint8_t)reg];
}

void vrms_render_vm_sysiregister_set(vrms_render_vm_t* vm, vrms_render_vm_sysireg_t reg, uint32_t value) {
    if ((uint8_t)reg >= NUM_REGS) {
        return;
    }
    vm->sysiregister[(uint8_t)reg] = value;
}

float* vrms_render_vm_mregister_get(vrms_render_vm_t* vm, uint8_t reg) {
    if (reg >= NUM_REGS) {
        return 0;
    }
    return &vm->mregister[reg * 16];
}

void vrms_render_vm_mregister_set_identity(vrms_render_vm_t* vm, uint8_t reg) {
    if (reg >= NUM_REGS) {
        return;
    }
    esmLoadIdentity(&vm->mregister[reg * 16]);
}

float* vrms_render_vm_sysmregister_get(vrms_render_vm_t* vm, vrms_render_vm_sysmreg_t reg) {
    if ((uint8_t)reg >= NUM_REGS) {
        return 0;
    }
    return vm->sysmregister[(uint8_t)reg];
}

void vrms_render_vm_sysmregister_set(vrms_render_vm_t* vm, vrms_render_vm_sysmreg_t reg, float* matrix) {
    if ((uint8_t)reg >= NUM_REGS) {
        return;
    }
    vm->sysmregister[(uint8_t)reg] = matrix;
    return;
}

void vrms_render_vm_alloc_ex_interrupt(vrms_render_vm_t* vm) {
    // TODO: Set the instruction pointer to the time allocation interrupt
}

uint8_t vrms_render_vm_has_exception(vrms_render_vm_t* vm) {
    if (vm->exception) {
        return 1;
    }
    return 0;
}

void vrms_render_vm_reset(vrms_render_vm_t* vm) {
    vm->running = 1;
    vm->program_counter = 0;
    vm->last_opcode = 0;
    memset(&vm->iregister, 0, INT_REG_SIZE * sizeof(uint32_t));
    memset(&vm->fregister, 0, FLO_REG_SIZE * sizeof(float));
    memset(&vm->vregister, 0, VEC_REG_SIZE * sizeof(float));
    memset(&vm->mregister, 0, MAT_REG_SIZE * sizeof(float));
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

    memory_id = vm->iregister[reg_mem];
    offset = vm->iregister[reg_off];

    if (!vm->load_matrix) {
        return;
    }

    matrix = vm->load_matrix(vm, memory_id, offset, vm->user_data);
    if (!matrix) {
        return;
    }

    // TODO: BAD - load_matrix() is probably called a lot, meaning memcpy()
    // calls all over the show. It might be better to mount a block of memory
    // into the VM from a matrix data object, and then use an integer register
    // as an index into that mounted block for the draw calls
    memcpy(&vm->mregister[reg_mat * 16], matrix, 16 * sizeof(float));
}

void vrms_render_vm_draw(vrms_render_vm_t* vm, uint8_t reg_mat, uint8_t obj_reg) {
    uint32_t object_id;
    float* matrix;

    matrix = &vm->mregister[reg_mat * 16];
    object_id = vm->iregister[obj_reg];

    if (!vm->draw) {
        return;
    }

    vm->draw(vm, matrix, object_id, vm->user_data);
}

void vrms_render_vm_exception(vrms_render_vm_t* vm, vrms_render_vm_exception_t exception) {
    vm->running = 0;
    vm->exception = exception;
}

uint8_t vrms_render_vm_exec(vrms_render_vm_t* vm, uint8_t* program, uint32_t length) {
    uint8_t reg1;
    uint8_t reg2;
    uint8_t reg3;
    uint8_t reg4;
    uint32_t ival1;
    uint32_t ival2;
    uint8_t opcode;
    uint32_t PC;

    if (!vm->running) {
        return vm->running;
    }

    PC = vm->program_counter;
    if (PC >= length) {
        vrms_render_vm_reset(vm);
        return 0;
    }

    opcode = (vrms_render_vm_opcode_t)program[PC];
    PC++;

    switch (opcode) {
        case VM_HALT:
            // Halt the program immediately. Does not reset the VM
            PC = 0;
            vm->running = 0;
            break;
        case VM_FRWAIT:
            // Signal to the VM that the frame drawing is complete
            vm->running = 0;
            break;
        case VM_RESET:
            // Reset the VM and move the instruction pointer to 0
            PC = 0;
            vrms_render_vm_reset(vm);
            break;

        //Control flow instructions
        case VM_JMP:
            // Move the instruction pointer to a constant
            ival1 = program[PC];
            PC = ival1;
            break;
        case VM_JMPNZ:
            // Move the instruction pointer to a constant: if a register is not zero
            reg1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            ival1 = vm->iregister[reg1];
            ival2 = program[PC];
            PC++;
            if (ival1 != 0) {
                PC = ival2;
            }
            break;
        case VM_JMPZ:
            // Move the instruction pointer to a constant: if a register is zero
            reg1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            ival1 = vm->iregister[reg1];
            ival2 = program[PC];
            PC++;
            if (ival1 == 0) {
                PC = ival2;
            }
            break;
        case VM_JMPI:
            // Move the instruction pointer to a value stored in an integer register
            reg1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            PC = vm->iregister[reg1];
            break;

        // Integer register instructions
        case VM_INTL:
            // Load an integer register with a constant value
            reg1 = program[PC];
            PC++;
            ival1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            vm->iregister[reg1] = ival1;
            break;
        case VM_INTLI:
            // Load an integer register with a value from another integer register
            reg1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg2 = program[PC];
            PC++;
            if (reg2 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            vm->iregister[reg1] = vm->iregister[reg2];
            break;
        case VM_INTLS:
            // Load an integer register from one of the system integer registers
            reg1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg2 = program[PC];
            PC++;
            if (reg2 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            vm->iregister[reg1] = vm->sysiregister[reg2];
            break;
        case VM_INTDEC:
            // Decrement an integer register
            reg1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            vm->iregister[reg1]--;
            break;
        case VM_INTINC:
            // Increment an integer register
            reg1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            vm->iregister[reg1]++;
            break;
        case VM_INTMUL:
            // Multiply two integer registers together and store the result in another integer register
            reg1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg2 = program[PC];
            PC++;
            if (reg2 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg3 = program[PC];
            PC++;
            if (reg3 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            vm->iregister[reg1] = vm->iregister[reg2] * vm->iregister[reg3];
            break;
        case VM_INTDIVF:
            // Divide an integer register with another integer register and store in a float register
            reg1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg2 = program[PC];
            PC++;
            if (reg2 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg3 = program[PC];
            PC++;
            if (reg3 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            vm->fregister[reg1] = (float)vm->iregister[reg2] / vm->iregister[reg3];
            break;
        case VM_INTDIVII:
            // Divide an integer register with another integer register and store the quotent in one register and the remainder in another register
            reg1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg2 = program[PC];
            PC++;
            if (reg2 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg3 = program[PC];
            PC++;
            if (reg3 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg4 = program[PC];
            PC++;
            if (reg4 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            vm->iregister[reg1] = vm->iregister[reg3] / vm->iregister[reg4];
            vm->iregister[reg2] = (((float)vm->iregister[reg3] / vm->iregister[reg4]) * 10000) - ((vm->iregister[reg3] / vm->iregister[reg4]) * 10000);
            break;
        case VM_INTLFI:
            // Load an integer register with the integer part of a float register
            reg1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg2 = program[PC];
            PC++;
            if (reg2 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            vm->iregister[reg1] = (uint32_t)vm->fregister[reg2];
            break;
        case VM_INTLFF:
            // Load an integer register with the fractional part of a float register (4 decimal places: 0.9999 becomes 9999)
            reg1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg2 = program[PC];
            PC++;
            if (reg2 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            ival1 = (uint32_t)vm->fregister[reg2];
            vm->iregister[reg1] = (uint32_t)((vm->fregister[reg2] * 10000) - (ival1 * 10000));
            break;

        // Float register instructions
        case VM_FLOL:
            // Load a float register with an integer constant
            reg1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            ival1 = program[PC];
            PC++;
            vm->fregister[reg1] = (float)ival1;
            break;
        case VM_FLOADD:
            // Add two float registers together and store in another float register
            reg1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg2 = program[PC];
            PC++;
            if (reg2 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg3 = program[PC];
            PC++;
            if (reg3 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            vm->fregister[reg1] = vm->fregister[reg2] + vm->fregister[reg3];
            break;
        case VM_FLOMUL:
            // Multiply two float registers together and store in another float register
            reg1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg2 = program[PC];
            PC++;
            if (reg2 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg3 = program[PC];
            PC++;
            if (reg3 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            vm->fregister[reg1] = vm->fregister[reg2] * vm->fregister[reg3];
            break;
        case VM_FLODIV:
            // Divide two float registers and store the result in another float register
            reg1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg2 = program[PC];
            PC++;
            if (reg2 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg3 = program[PC];
            PC++;
            if (reg3 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            vm->fregister[reg1] = vm->fregister[reg2] / vm->fregister[reg3];
            break;
        case VM_FLOLII:
            // Load the integer component of a float register from an integer register (zeros fractional part)
            reg1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg2 = program[PC];
            PC++;
            if (reg2 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            vm->fregister[reg1] = (float)vm->iregister[reg2];
            break;
        case VM_FLOLIF:
            // Load the fractional component of a float register from an integer register (4 decimal places: if the integer is larger than 9999 it is floored to 9999)
            reg1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg2 = program[PC];
            PC++;
            if (reg2 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            ival1 = vm->iregister[reg2];
            if (ival1 > 9999) {
                ival1 = 9999;
            }
            vm->fregister[reg1] = (float)ival1 / 10000;
            break;

        // Vector register instructions
        case VM_VECL:
            // Load a vector register with the values of three float registers
            reg1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg2 = program[PC];
            PC++;
            if (reg2 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg3 = program[PC];
            PC++;
            if (reg3 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg4 = program[PC];
            PC++;
            if (reg4 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg1 = reg1 * 3;
            vm->vregister[reg1 + VM_VREG_X] = vm->fregister[reg2];
            vm->vregister[reg1 + VM_VREG_Y] = vm->fregister[reg3];
            vm->vregister[reg1 + VM_VREG_Z] = vm->fregister[reg4];
            break;
        case VM_VECLX:
            // Load the X component of a vector from a float register
            reg1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg2 = program[PC];
            PC++;
            if (reg2 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg1 = reg1 * 3;
            vm->vregister[reg1 + VM_VREG_X] = vm->fregister[reg2];
            break;
        case VM_VECLY:
            // Load the Y component of a vector from a float register
            reg1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg2 = program[PC];
            PC++;
            if (reg2 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg1 = reg1 * 3;
            vm->vregister[reg1 + VM_VREG_Y] = vm->fregister[reg2];
            break;
        case VM_VECLZ:
            // Load the Z component of a vector from a float register
            reg1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg2 = program[PC];
            PC++;
            if (reg2 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg1 = reg1 * 3;
            vm->vregister[reg1 + VM_VREG_Z] = vm->fregister[reg2];
            break;
        case VM_VECLXI:
            // Load the X component of a vector from an integer register
            reg1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg2 = program[PC];
            PC++;
            if (reg2 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg1 = reg1 * 3;
            vm->vregister[reg1 + VM_VREG_X] = (float)vm->iregister[reg2];
            break;
        case VM_VECLYI:
            // Load the Y component of a vector from an integer register
            reg1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg2 = program[PC];
            PC++;
            if (reg2 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg1 = reg1 * 3;
            vm->vregister[reg1 + VM_VREG_Y] = (float)vm->iregister[reg2];
            break;
        case VM_VECLZI:
            // Load the Z component of a vector from an integer register
            reg1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg2 = program[PC];
            PC++;
            if (reg2 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg1 = reg1 * 3;
            vm->vregister[reg1 + VM_VREG_Z] = (float)vm->iregister[reg2];
            break;
        case VM_VECLXC:
            // Load the X component of a vector from an integer constant
            reg1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            ival1 = program[PC];
            PC++;
            reg1 = reg1 * 3;
            vm->vregister[reg1 + VM_VREG_X] = (float)ival1;
            break;
        case VM_VECLYC:
            // Load the Y component of a vector from an integer constant
            reg1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            ival1 = program[PC];
            PC++;
            reg1 = reg1 * 3;
            vm->vregister[reg1 + VM_VREG_Y] = (float)ival1;
            break;
        case VM_VECLZC:
            // Load the Z component of a vector from an integer constant
            reg1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            ival1 = program[PC];
            PC++;
            reg1 = reg1 * 3;
            vm->vregister[reg1 + VM_VREG_Z] = (float)ival1;
            break;
        case VM_VECINCX:
            // Increment the X component of a vector by a float register
            reg1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg2 = program[PC];
            PC++;
            if (reg2 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg1 = reg1 * 3;
            vm->vregister[reg1 + VM_VREG_X] += vm->fregister[reg2];
            break;
        case VM_VECINCY:
            // Increment the Y component of a vector by a float register
            reg1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg2 = program[PC];
            PC++;
            if (reg2 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg1 = reg1 * 3;
            vm->vregister[reg1 + VM_VREG_Y] += vm->fregister[reg2];
            break;
        case VM_VECINCZ:
            // Increment the Z component of a vector by a float register
            reg1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg2 = program[PC];
            PC++;
            if (reg2 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg1 = reg1 * 3;
            vm->vregister[reg1 + VM_VREG_Z] += vm->fregister[reg2];
            break;
        case VM_VECDECX:
            // Decrement the X component of a vector by a float register
            reg1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg2 = program[PC];
            PC++;
            if (reg2 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg1 = reg1 * 3;
            vm->vregister[reg1 + VM_VREG_X] -= vm->fregister[reg2];
            break;
        case VM_VECDECY:
            // Decrement the Y component of a vector by a float register
            reg1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg2 = program[PC];
            PC++;
            if (reg2 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg1 = reg1 * 3;
            vm->vregister[reg1 + VM_VREG_Y] -= vm->fregister[reg2];
            break;
        case VM_VECDECZ:
            // Decrement the Z component of a vector by a float register
            reg1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg2 = program[PC];
            PC++;
            if (reg2 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg1 = reg1 * 3;
            vm->vregister[reg1 + VM_VREG_Z] -= vm->fregister[reg2];
            break;

        // Matrix register instructions
        case VM_MATLM:
            // Load a matrix register with data from a matrix data object
            reg1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg2 = program[PC];
            PC++;
            if (reg2 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg3 = program[PC];
            PC++;
            if (reg3 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            vrms_render_vm_load_matrix(vm, reg1, reg2, reg3);
            break;
        case VM_MATLI:
            // Load a matrix register with the identity matrix
            reg1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg1 = reg1 * 16;
            esmLoadIdentity(&vm->mregister[reg1]);
            break;
        case VM_MATLS:
            // Load a matrix register from one of the system matrix registers
            break;
        case VM_MATSL:
            // Load one of the system matrix registers from a matrix register (needs a permission)
            break;
        case VM_MATM:
            // Multiply a matrix register with another matrix register
            reg1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg2 = program[PC];
            PC++;
            if (reg2 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg1 = reg1 * 16;
            reg2 = reg2 * 16;
            esmMultiply(&vm->mregister[reg1], &vm->mregister[reg2]);
            break;
        case VM_MATMTV:
            // Translate a matrix register by a vector register
            reg1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg2 = program[PC];
            PC++;
            if (reg2 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg1 = reg1 * 16;
            reg2 = reg2 * 3;
            esmTranslatef(&vm->mregister[reg1], vm->vregister[reg2 + VM_VREG_X], vm->vregister[reg2 + VM_VREG_Y], vm->vregister[reg2 + VM_VREG_Z]);
            break;
        case VM_MATMRX:
            // Rotate a matrix around the X axis by a float register
            reg1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg2 = program[PC];
            PC++;
            if (reg2 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg1 = reg1 * 16;
            esmRotatef(&vm->mregister[reg1], vm->fregister[reg2], 1, 0, 0);
            break;
        case VM_MATMRY:
            // Rotate a matrix around the Y axis by a float register
            reg1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg2 = program[PC];
            PC++;
            if (reg2 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg1 = reg1 * 16;
            esmRotatef(&vm->mregister[reg1], vm->fregister[reg2], 0, 1, 0);
            break;
        case VM_MATMRZ:
            // Rotate a matrix around the Z axis by a float register
            reg1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg2 = program[PC];
            PC++;
            if (reg2 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg1 = reg1 * 16;
            esmRotatef(&vm->mregister[reg1], vm->fregister[reg2], 0, 0, 1);
            break;

        // Drawing instructions
        case VM_DRAW:
            // Draw an object from a matrix register and an integer register
            reg1 = program[PC];
            PC++;
            if (reg1 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            reg2 = program[PC];
            PC++;
            if (reg2 >= NUM_REGS) {
                vrms_render_vm_exception(vm, VM_X_REG_OUT_OF_BOUNDS);
                PC = 0;
                break;
            }
            vrms_render_vm_draw(vm, reg1, reg2);
            break;
        default:
            vrms_render_vm_exception(vm, VM_X_INV_OPCODE);
            vm->running = 0;
            break;
    }

    vm->last_opcode = opcode;
    vm->program_counter = PC;

    return vm->running;
}
