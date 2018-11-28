#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include "render_vm.h"
#include "test_harness.h"

// Things the load_matrix() and draw() hooks always expect
#define MAGIC_MEMORY_ID     3
#define MAGIC_MEMORY_OFFSET 80
#define MAGIC_OBJECT_ID     2

float* load_matrix(vrms_render_vm_t* vm, uint32_t memory_id, uint32_t offset, void* user_data) {
    test_harness_t* test;
    test = (test_harness_t*)user_data;

    is_equal_uint32(test, memory_id, MAGIC_MEMORY_ID, "load_matrix call returns correct memory_id");
    is_equal_uint32(test, offset, MAGIC_MEMORY_OFFSET, "load_matrix call returns correct offset");

    return NULL;
}

void draw(vrms_render_vm_t* vm, float* matrix, uint32_t object_id, void* user_data) {
    test_harness_t* test;
    test = (test_harness_t*)user_data;

    is_equal_uint32(test, object_id, MAGIC_OBJECT_ID, "draw call returns correct object_id");
}

float _R4DP(float v) {
    v = v * 10000;
    v = floor(v + 0.5f);
    return v / 10000;
}

void test_opcode_VM_HALT(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t program[] = {VM_HALT};
    vrms_render_vm_exec(vm, program, 1);
    is_equal_uint8(test, vm->running, 0, "OP VM_HALT: stops VM");
    is_equal_uint32(test, vm->program_counter, 0, "OP VM_HALT: zeros program counter");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_FRWAIT(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t program[] = {VM_FRWAIT};
    vrms_render_vm_exec(vm, program, 1);
    is_equal_uint8(test, vm->running, 0, "OP VM_FRWAIT: stops VM");
    is_equal_uint32(test, vm->program_counter, 1, "OP VM_FRWAIT: program counter correct");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_RESET(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t program[] = {VM_RESET};
    vrms_render_vm_exec(vm, program, 1);
    is_equal_uint8(test, vm->running, 1, "OP VM_RESET: starts VM");
    is_equal_uint32(test, vm->program_counter, 0, "OP VM_RESET: program counter reset");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_JMP(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t program[] = {VM_JMP, 123};
    vrms_render_vm_exec(vm, program, 1);
    is_equal_uint32(test, vm->program_counter, 123, "OP VM_JMP: moves program_counter");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_JMPNZ(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t program[] = {VM_JMPNZ, VM_REG0, 45};

    // Test setting register to non-zero
    vrms_render_vm_iregister_set(vm, VM_REG0, 1);
    vrms_render_vm_exec(vm, program, 3);
    is_equal_uint32(test, vm->program_counter, 45, "OP VM_JMPNZ: moves program counter when not zero");
    vrms_render_vm_reset(vm);

    // Test setting register to zero
    vrms_render_vm_iregister_set(vm, VM_REG0, 0);
    vrms_render_vm_exec(vm, program, 3);
    is_equal_uint32(test, vm->program_counter, 3, "OP VM_JMPNZ: does not move program counter when zero");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_JMPZ(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t program[] = {VM_JMPZ, VM_REG0, 50};

    // Test setting register to non-zero
    vrms_render_vm_iregister_set(vm, VM_REG0, 1);
    vrms_render_vm_exec(vm, program, 3);
    is_equal_uint32(test, vm->program_counter, 3, "OP VM_JMPZ: does not move program counter when not zero");
    vrms_render_vm_reset(vm);

    // Test setting register to zero
    vrms_render_vm_iregister_set(vm, VM_REG0, 0);
    vrms_render_vm_exec(vm, program, 3);
    is_equal_uint32(test, vm->program_counter, 50, "OP VM_JMPZ: moves program counter when zero");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_JMPI(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t program[] = {VM_JMPI, VM_REG0};
    vrms_render_vm_iregister_set(vm, VM_REG0, 20);
    vrms_render_vm_exec(vm, program, 2);
    is_equal_uint32(test, vm->program_counter, 20, "OP VM_JMPI: moves program counter to register value");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_INTL(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t program[] = {VM_INTL, VM_REG0, 42};
    vrms_render_vm_exec(vm, program, 3);
    is_equal_uint32(test, vrms_render_vm_iregister_get(vm, VM_REG0), 42, "OP VM_INTL: sets register");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_INTLI(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t program[] = {VM_INTLI, VM_REG1, VM_REG0};
    vrms_render_vm_iregister_set(vm, VM_REG0, 55);
    vrms_render_vm_exec(vm, program, 3);
    is_equal_uint32(test, vrms_render_vm_iregister_get(vm, VM_REG1), 55, "OP VM_INTLI: register copied");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_INTLS(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t program[] = {VM_INTLS, VM_REG0, VM_REG0};
    vrms_render_vm_sysiregister_set(vm, VM_REG0, 5);
    vrms_render_vm_exec(vm, program, 1);
    is_equal_uint32(test, vrms_render_vm_iregister_get(vm, VM_REG0), 5, "OP VM_INTLS: register copied from sys");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_INTDEC(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t program[] = {VM_INTDEC, VM_REG0};
    vrms_render_vm_iregister_set(vm, VM_REG0, 50);
    vrms_render_vm_exec(vm, program, 2);
    is_equal_uint32(test, vrms_render_vm_iregister_get(vm, VM_REG0), 49, "OP VM_INTDEC: decremented");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_INTINC(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t program[] = {VM_INTINC, VM_REG0};
    vrms_render_vm_iregister_set(vm, VM_REG0, 50);
    vrms_render_vm_exec(vm, program, 2);
    is_equal_uint32(test, vrms_render_vm_iregister_get(vm, VM_REG0), 51, "OP VM_INTINC: incremented");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_INTMUL(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t program[] = {VM_INTMUL, VM_REG2, VM_REG0, VM_REG1};
    vrms_render_vm_iregister_set(vm, VM_REG0, 6);
    vrms_render_vm_iregister_set(vm, VM_REG1, 6);
    vrms_render_vm_exec(vm, program, 4);
    is_equal_uint32(test, vrms_render_vm_iregister_get(vm, VM_REG2), 36, "OP VM_INTMUL: multiplication");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_INTDIVF(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t program[] = {VM_INTDIVF, VM_REG0, VM_REG0, VM_REG1};
    vrms_render_vm_iregister_set(vm, VM_REG0, 7);
    vrms_render_vm_iregister_set(vm, VM_REG1, 2);
    vrms_render_vm_exec(vm, program, 4);
    is_equal_float(test, vrms_render_vm_fregister_get(vm, VM_REG0), 3.5f, "OP VM_INTDIVF: division into float");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_INTDIVII(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t program[] = {VM_INTDIVII, VM_REG0, VM_REG1, VM_REG2, VM_REG3};
    vrms_render_vm_iregister_set(vm, VM_REG2, 7);
    vrms_render_vm_iregister_set(vm, VM_REG3, 2);
    vrms_render_vm_exec(vm, program, 5);
    is_equal_uint32(test, vrms_render_vm_iregister_get(vm, VM_REG0), 3, "OP VM_INTDIVII: quotent correct");
    is_equal_uint32(test, vrms_render_vm_iregister_get(vm, VM_REG1), 5000, "OP VM_INTDIVII: remainder correct");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_INTLFI(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t program[] = {VM_INTLFI, VM_REG0, VM_REG0};
    vrms_render_vm_fregister_set(vm, VM_REG0, 8.34f);
    vrms_render_vm_exec(vm, program, 3);
    is_equal_uint32(test, vrms_render_vm_iregister_get(vm, VM_REG0), 8, "OP VM_INTLFI: correct");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_INTLFF(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t program[] = {VM_INTLFF, VM_REG0, VM_REG0};
    vrms_render_vm_fregister_set(vm, VM_REG0, 8.34f);
    vrms_render_vm_exec(vm, program, 3);
    is_equal_uint32(test, vrms_render_vm_iregister_get(vm, VM_REG0), 3400, "OP VM_INTLFF: correct");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_FLOL(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t program[] = {VM_FLOL, VM_REG0, 2};
    vrms_render_vm_exec(vm, program, 3);
    is_equal_float(test, vrms_render_vm_fregister_get(vm, VM_REG0), 2.0f, "OP VM_FLOL: correct");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_FLOADD(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t program[] = {VM_FLOADD, VM_REG0, VM_REG1, VM_REG2};
    vrms_render_vm_fregister_set(vm, VM_REG1, 8.34f);
    vrms_render_vm_fregister_set(vm, VM_REG2, 8.34f);
    vrms_render_vm_exec(vm, program, 4);
    is_equal_float(test, vrms_render_vm_fregister_get(vm, VM_REG0), 16.68f, "OP VM_FLOADD: correct");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_FLOMUL(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t program[] = {VM_FLOMUL, VM_REG0, VM_REG1, VM_REG2};
    vrms_render_vm_fregister_set(vm, VM_REG1, 9);
    vrms_render_vm_fregister_set(vm, VM_REG2, 9);
    vrms_render_vm_exec(vm, program, 4);
    is_equal_float(test, _R4DP(vrms_render_vm_fregister_get(vm, VM_REG0)), 81.0f, "OP VM_FLOMUL: float multiplication correct");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_FLODIV(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t program[] = {VM_FLODIV, VM_REG0, VM_REG1, VM_REG2};
    vrms_render_vm_fregister_set(vm, VM_REG1, 10);
    vrms_render_vm_fregister_set(vm, VM_REG2, 3);
    vrms_render_vm_exec(vm, program, 4);
    is_equal_float(test, _R4DP(vrms_render_vm_fregister_get(vm, VM_REG0)), 3.3333f, "OP VM_FLODIV: float division correct");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_FLOLII(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t program[] = {VM_FLOLII, VM_REG0, VM_REG0};
    vrms_render_vm_iregister_set(vm, VM_REG0, 15);
    vrms_render_vm_exec(vm, program, 3);
    is_equal_float(test, vrms_render_vm_fregister_get(vm, VM_REG0), 15.0f, "OP VM_FLOLII: load integer part of float correct");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_FLOLIF(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t program[] = {VM_FLOLIF, VM_REG0, VM_REG0};
    vrms_render_vm_iregister_set(vm, VM_REG0, 1453);
    vrms_render_vm_exec(vm, program, 3);
    is_equal_float(test, vrms_render_vm_fregister_get(vm, VM_REG0), 0.1453f, "OP VM_FLOLIF: load decimal part of float correct");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_VECL(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t program[] = {VM_VECL, VM_REG6, VM_REG1, VM_REG2, VM_REG3};
    vrms_render_vm_fregister_set(vm, VM_REG1, 1.23f);
    vrms_render_vm_fregister_set(vm, VM_REG2, 3.45f);
    vrms_render_vm_fregister_set(vm, VM_REG3, 5.67f);
    vrms_render_vm_exec(vm, program, 5);
    is_equal_float(test, vrms_render_vm_vregister_get(vm, VM_REG6, VM_VREG_X), 1.23f, "OP VM_VECL: loaded X part");
    is_equal_float(test, vrms_render_vm_vregister_get(vm, VM_REG6, VM_VREG_Y), 3.45f, "OP VM_VECL: loaded Y part");
    is_equal_float(test, vrms_render_vm_vregister_get(vm, VM_REG6, VM_VREG_Z), 5.67f, "OP VM_VECL: loaded Z part");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_VECLX(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t program[] = {VM_VECLX, VM_REG5, VM_REG1};
    vrms_render_vm_fregister_set(vm, VM_REG1, 1.3f);
    vrms_render_vm_exec(vm, program, 3);
    is_equal_float(test, vrms_render_vm_vregister_get(vm, VM_REG5, VM_VREG_X), 1.3f, "OP VM_VECLX: loaded X part");
    is_equal_float(test, vrms_render_vm_vregister_get(vm, VM_REG5, VM_VREG_Y), 0.0f, "OP VM_VECLX: Y part zero");
    is_equal_float(test, vrms_render_vm_vregister_get(vm, VM_REG5, VM_VREG_Z), 0.0f, "OP VM_VECLX: Z part zero");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_VECLY(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t program[] = {VM_VECLY, VM_REG5, VM_REG1};
    vrms_render_vm_fregister_set(vm, VM_REG1, 1.1f);
    vrms_render_vm_exec(vm, program, 3);
    is_equal_float(test, vrms_render_vm_vregister_get(vm, VM_REG5, VM_VREG_X), 0.0f, "OP VM_VECLY: X part zero");
    is_equal_float(test, vrms_render_vm_vregister_get(vm, VM_REG5, VM_VREG_Y), 1.1f, "OP VM_VECLY: loaded Y part");
    is_equal_float(test, vrms_render_vm_vregister_get(vm, VM_REG5, VM_VREG_Z), 0.0f, "OP VM_VECLY: Z part zero");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_VECLZ(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t program[] = {VM_VECLZ, VM_REG5, VM_REG1};
    vrms_render_vm_fregister_set(vm, VM_REG1, 1.6f);
    vrms_render_vm_exec(vm, program, 3);
    is_equal_float(test, vrms_render_vm_vregister_get(vm, VM_REG5, VM_VREG_X), 0.0f, "OP VM_VECLZ: X part zero");
    is_equal_float(test, vrms_render_vm_vregister_get(vm, VM_REG5, VM_VREG_Y), 0.0f, "OP VM_VECLZ: Y part zero");
    is_equal_float(test, vrms_render_vm_vregister_get(vm, VM_REG5, VM_VREG_Z), 1.6f, "OP VM_VECLZ: loaded Z part");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_VECLXI(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t program[] = {VM_VECLXI, VM_REG0, VM_REG0};
    vrms_render_vm_iregister_set(vm, VM_REG0, 4);
    vrms_render_vm_exec(vm, program, 3);
    is_equal_float(test, vrms_render_vm_vregister_get(vm, VM_REG0, VM_VREG_X), 4.0f, "OP VM_VECLXI: loaded X part");
    is_equal_float(test, vrms_render_vm_vregister_get(vm, VM_REG0, VM_VREG_Y), 0.0f, "OP VM_VECLXI: Y part zero");
    is_equal_float(test, vrms_render_vm_vregister_get(vm, VM_REG0, VM_VREG_Z), 0.0f, "OP VM_VECLXI: Z part zero");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_VECLYI(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t program[] = {VM_VECLYI, VM_REG0, VM_REG0};
    vrms_render_vm_iregister_set(vm, VM_REG0, 5);
    vrms_render_vm_exec(vm, program, 3);
    is_equal_float(test, vrms_render_vm_vregister_get(vm, VM_REG0, VM_VREG_X), 0.0f, "OP VM_VECLYI: X part zero");
    is_equal_float(test, vrms_render_vm_vregister_get(vm, VM_REG0, VM_VREG_Y), 5.0f, "OP VM_VECLYI: loaded Y part");
    is_equal_float(test, vrms_render_vm_vregister_get(vm, VM_REG0, VM_VREG_Z), 0.0f, "OP VM_VECLYI: Z part zero");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_VECLZI(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t program[] = {VM_VECLZI, VM_REG0, VM_REG0};
    vrms_render_vm_iregister_set(vm, VM_REG0, 6);
    vrms_render_vm_exec(vm, program, 3);
    is_equal_float(test, vrms_render_vm_vregister_get(vm, VM_REG0, VM_VREG_X), 0.0f, "OP VM_VECLZI: X part zero");
    is_equal_float(test, vrms_render_vm_vregister_get(vm, VM_REG0, VM_VREG_Y), 0.0f, "OP VM_VECLZI: Y part zero");
    is_equal_float(test, vrms_render_vm_vregister_get(vm, VM_REG0, VM_VREG_Z), 6.0f, "OP VM_VECLZI: loaded Z part");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_VECLXC(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t program[] = {VM_VECLXC, VM_REG0, 7};
    vrms_render_vm_exec(vm, program, 3);
    is_equal_float(test, vrms_render_vm_vregister_get(vm, VM_REG0, VM_VREG_X), 7.0f, "OP VM_VECLXC: loaded X part");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_VECLYC(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t program[] = {VM_VECLYC, VM_REG0, 8};
    vrms_render_vm_exec(vm, program, 3);
    is_equal_float(test, vrms_render_vm_vregister_get(vm, VM_REG0, VM_VREG_Y), 8.0f, "OP VM_VECLYC: loaded Y part");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_VECLZC(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t program[] = {VM_VECLZC, VM_REG0, 9};
    vrms_render_vm_exec(vm, program, 3);
    is_equal_float(test, vrms_render_vm_vregister_get(vm, VM_REG0, VM_VREG_Z), 9.0f, "OP VM_VECLZC: loaded Z part");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_VECINCX(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t program[] = {VM_VECINCX, VM_REG0, VM_REG0};
    vrms_render_vm_fregister_set(vm, VM_REG0, 2.3);
    vrms_render_vm_vregister_set(vm, VM_REG0, VM_VREG_X, 4.9);
    vrms_render_vm_exec(vm, program, 3);
    is_equal_float(test, vrms_render_vm_vregister_get(vm, VM_REG0, VM_VREG_X), 7.2f, "OP VM_VECINCX: incremented X from float");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_VECINCY(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t program[] = {VM_VECINCY, VM_REG0, VM_REG0};
    vrms_render_vm_fregister_set(vm, VM_REG0, 2.3);
    vrms_render_vm_vregister_set(vm, VM_REG0, VM_VREG_Y, 4.9);
    vrms_render_vm_exec(vm, program, 3);
    is_equal_float(test, vrms_render_vm_vregister_get(vm, VM_REG0, VM_VREG_Y), 7.2f, "OP VM_VECINCY: incremented Y from float");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_VECINCZ(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t program[] = {VM_VECINCZ, VM_REG0, VM_REG0};
    vrms_render_vm_fregister_set(vm, VM_REG0, 2.3);
    vrms_render_vm_vregister_set(vm, VM_REG0, VM_VREG_Z, 4.9);
    vrms_render_vm_exec(vm, program, 3);
    is_equal_float(test, vrms_render_vm_vregister_get(vm, VM_REG0, VM_VREG_Z), 7.2f, "OP VM_VECINCZ: incremented Z from float");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_VECDECX(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t program[] = {VM_VECDECX, VM_REG0, VM_REG0};
    vrms_render_vm_fregister_set(vm, VM_REG0, 4.9);
    vrms_render_vm_vregister_set(vm, VM_REG0, VM_VREG_X, 2.3);
    vrms_render_vm_exec(vm, program, 3);
    is_equal_float(test, _R4DP(vrms_render_vm_vregister_get(vm, VM_REG0, VM_VREG_X)), -2.6f, "OP VM_VECDECX: decremented X by float");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_VECDECY(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t program[] = {VM_VECDECY, VM_REG0, VM_REG0};
    vrms_render_vm_fregister_set(vm, VM_REG0, 2.3);
    vrms_render_vm_vregister_set(vm, VM_REG0, VM_VREG_Y, 4.9);
    vrms_render_vm_exec(vm, program, 3);
    is_equal_float(test, _R4DP(vrms_render_vm_vregister_get(vm, VM_REG0, VM_VREG_Y)), 2.6f, "OP VM_VECDECY: decremented Y by float");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_VECDECZ(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t program[] = {VM_VECDECZ, VM_REG0, VM_REG0};
    vrms_render_vm_fregister_set(vm, VM_REG0, 2.3);
    vrms_render_vm_vregister_set(vm, VM_REG0, VM_VREG_Z, 4.9);
    vrms_render_vm_exec(vm, program, 3);
    is_equal_float(test, _R4DP(vrms_render_vm_vregister_get(vm, VM_REG0, VM_VREG_Z)), 2.6f, "OP VM_VECDECZ: decremented Z by float");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_MATLM(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t program[] = {VM_MATLM, VM_REG0, VM_REG0, VM_REG1};
    vrms_render_vm_iregister_set(vm, VM_REG0, MAGIC_MEMORY_ID);
    vrms_render_vm_iregister_set(vm, VM_REG1, MAGIC_MEMORY_OFFSET);
    vrms_render_vm_exec(vm, program, 1);
    test_harness_make_note(test, "TODO Somehow return something from load_matrix() and test it here");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_MATLI(test_harness_t* test, vrms_render_vm_t* vm) {
    float* matrix;
    uint8_t program[] = {VM_MATLI, VM_REG0};
    vrms_render_vm_exec(vm, program, 2);
    matrix = vrms_render_vm_mregister_get(vm, VM_REG0);
    is_equal_float(test, matrix[0], 1.0f, "OP VM_MATLI: spot check index 0");
    is_equal_float(test, matrix[1], 0.0f, "OP VM_MATLI: spot check index 1");
    is_equal_float(test, matrix[2], 0.0f, "OP VM_MATLI: spot check index 2");
    is_equal_float(test, matrix[5], 1.0f, "OP VM_MATLI: spot check index 5");
    is_equal_float(test, matrix[8], 0.0f, "OP VM_MATLI: spot check index 8");
    is_equal_float(test, matrix[10], 1.0f, "OP VM_MATLI: spot check index 10");
    is_equal_float(test, matrix[15], 1.0f, "OP VM_MATLI: spot check index 15");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_MATLS(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t program[] = {VM_MATLS};
    vrms_render_vm_exec(vm, program, 1);
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_MATSL(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t program[] = {VM_MATSL};
    vrms_render_vm_exec(vm, program, 1);
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_MATM(test_harness_t* test, vrms_render_vm_t* vm) {
    float* matrix;
    uint8_t program[] = {VM_MATM, VM_REG0, VM_REG1};
    vrms_render_vm_mregister_set_identity(vm, VM_REG0);
    vrms_render_vm_mregister_set_identity(vm, VM_REG1);
    vrms_render_vm_exec(vm, program, 3);
    matrix = vrms_render_vm_mregister_get(vm, VM_REG0);
    is_equal_float(test, matrix[0], 1.0f, "OP VM_MATM: spot check index 0");
    is_equal_float(test, matrix[1], 0.0f, "OP VM_MATM: spot check index 1");
    is_equal_float(test, matrix[2], 0.0f, "OP VM_MATM: spot check index 2");
    is_equal_float(test, matrix[5], 1.0f, "OP VM_MATM: spot check index 5");
    is_equal_float(test, matrix[8], 0.0f, "OP VM_MATM: spot check index 8");
    is_equal_float(test, matrix[10], 1.0f, "OP VM_MATM: spot check index 10");
    is_equal_float(test, matrix[15], 1.0f, "OP VM_MATM: spot check index 15");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_MATMTV(test_harness_t* test, vrms_render_vm_t* vm) {
    float* matrix;
    uint8_t program[] = {VM_MATMTV, VM_REG0, VM_REG0};
    vrms_render_vm_mregister_set_identity(vm, VM_REG0);
    vrms_render_vm_vregister_set(vm, VM_REG0, VM_VREG_X, 4.1f);
    vrms_render_vm_vregister_set(vm, VM_REG0, VM_VREG_Y, 4.2f);
    vrms_render_vm_vregister_set(vm, VM_REG0, VM_VREG_Z, 4.3f);
    vrms_render_vm_exec(vm, program, 3);
    matrix = vrms_render_vm_mregister_get(vm, VM_REG0);
    is_equal_float(test, matrix[0], 1.0f, "OP VM_MATMTV: spot check index 0");
    is_equal_float(test, matrix[1], 0.0f, "OP VM_MATMTV: spot check index 1");
    is_equal_float(test, matrix[2], 0.0f, "OP VM_MATMTV: spot check index 2");
    is_equal_float(test, matrix[5], 1.0f, "OP VM_MATMTV: spot check index 5");
    is_equal_float(test, matrix[8], 0.0f, "OP VM_MATMTV: spot check index 8");
    is_equal_float(test, matrix[10], 1.0f, "OP VM_MATMTV: spot check index 10");
    is_equal_float(test, matrix[15], 1.0f, "OP VM_MATMTV: spot check index 15");
    is_equal_float(test, matrix[12], 4.1f, "OP VM_MATMTV: check translation X");
    is_equal_float(test, matrix[13], 4.2f, "OP VM_MATMTV: check translation Y");
    is_equal_float(test, matrix[14], 4.3f, "OP VM_MATMTV: check translation Z");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_MATMRX(test_harness_t* test, vrms_render_vm_t* vm) {
    float* matrix;
    uint8_t program[] = {VM_MATMRX, VM_REG0, VM_REG0};
    vrms_render_vm_mregister_set_identity(vm, VM_REG0);
    vrms_render_vm_fregister_set(vm, VM_REG0, 1.1f);
    vrms_render_vm_exec(vm, program, 3);
    matrix = vrms_render_vm_mregister_get(vm, VM_REG0);
    is_equal_float(test, _R4DP(matrix[0]), 1.0f, "OP VM_MATMRX: rotate X");
    is_equal_float(test, _R4DP(matrix[5]), 0.4536f, "OP VM_MATMRX: rotate X");
    is_equal_float(test, _R4DP(matrix[6]), 0.8912f, "OP VM_MATMRX: rotate X");
    is_equal_float(test, _R4DP(matrix[9]), -0.8912f, "OP VM_MATMRX: rotate X");
    is_equal_float(test, _R4DP(matrix[10]), 0.4536f, "OP VM_MATMRX: rotate X");
    is_equal_float(test, _R4DP(matrix[15]), 1.0f, "OP VM_MATMRX: rotate X");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_MATMRY(test_harness_t* test, vrms_render_vm_t* vm) {
    float* matrix;
    uint8_t program[] = {VM_MATMRY, VM_REG0, VM_REG0};
    vrms_render_vm_mregister_set_identity(vm, VM_REG0);
    vrms_render_vm_fregister_set(vm, VM_REG0, 1.1f);
    vrms_render_vm_exec(vm, program, 3);
    matrix = vrms_render_vm_mregister_get(vm, VM_REG0);
    is_equal_float(test, _R4DP(matrix[0]), 0.4536f, "OP VM_MATMRY: rotate Y");
    is_equal_float(test, _R4DP(matrix[2]), -0.8912f, "OP VM_MATMRY: rotate Y");
    is_equal_float(test, _R4DP(matrix[8]), 0.8912f, "OP VM_MATMRY: rotate Y");
    is_equal_float(test, _R4DP(matrix[10]), 0.4536f, "OP VM_MATMRY: rotate Y");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_MATMRZ(test_harness_t* test, vrms_render_vm_t* vm) {
    float* matrix;
    uint8_t program[] = {VM_MATMRZ, VM_REG0, VM_REG0};
    vrms_render_vm_mregister_set_identity(vm, VM_REG0);
    vrms_render_vm_fregister_set(vm, VM_REG0, 1.1f);
    vrms_render_vm_exec(vm, program, 3);
    matrix = vrms_render_vm_mregister_get(vm, VM_REG0);
    is_equal_float(test, _R4DP(matrix[0]), 0.4536f, "OP VM_MATMRZ: rotate Z");
    is_equal_float(test, _R4DP(matrix[1]), 0.8912f, "OP VM_MATMRZ: rotate Z");
    is_equal_float(test, _R4DP(matrix[4]), -0.8912f, "OP VM_MATMRZ: rotate Z");
    is_equal_float(test, _R4DP(matrix[5]), 0.4536f, "OP VM_MATMRZ: rotate Z");
    vrms_render_vm_reset(vm);
}

void test_opcode_VM_DRAW(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t program[] = {VM_DRAW, VM_REG0, VM_REG0};
    vrms_render_vm_iregister_set(vm, VM_REG0, MAGIC_OBJECT_ID);
    vrms_render_vm_exec(vm, program, 3);
    vrms_render_vm_reset(vm);
}

void test_all_opcodes(test_harness_t* test, vrms_render_vm_t* vm) {
    test_opcode_VM_HALT(test, vm);
    test_opcode_VM_FRWAIT(test, vm);
    test_opcode_VM_RESET(test, vm);
    test_opcode_VM_JMP(test, vm);
    test_opcode_VM_JMPNZ(test, vm);
    test_opcode_VM_JMPZ(test, vm);
    test_opcode_VM_JMPI(test, vm);
    test_opcode_VM_INTL(test, vm);
    test_opcode_VM_INTLI(test, vm);
    test_opcode_VM_INTLS(test, vm);
    test_opcode_VM_INTDEC(test, vm);
    test_opcode_VM_INTINC(test, vm);
    test_opcode_VM_INTMUL(test, vm);
    test_opcode_VM_INTDIVF(test, vm);
    test_opcode_VM_INTDIVII(test, vm);
    test_opcode_VM_INTLFI(test, vm);
    test_opcode_VM_INTLFF(test, vm);
    test_opcode_VM_FLOL(test, vm);
    test_opcode_VM_FLOADD(test, vm);
    test_opcode_VM_FLOMUL(test, vm);
    test_opcode_VM_FLODIV(test, vm);
    test_opcode_VM_FLOLII(test, vm);
    test_opcode_VM_FLOLIF(test, vm);
    test_opcode_VM_VECL(test, vm);
    test_opcode_VM_VECLX(test, vm);
    test_opcode_VM_VECLY(test, vm);
    test_opcode_VM_VECLZ(test, vm);
    test_opcode_VM_VECLXI(test, vm);
    test_opcode_VM_VECLYI(test, vm);
    test_opcode_VM_VECLZI(test, vm);
    test_opcode_VM_VECLXC(test, vm);
    test_opcode_VM_VECLYC(test, vm);
    test_opcode_VM_VECLZC(test, vm);
    test_opcode_VM_VECINCX(test, vm);
    test_opcode_VM_VECINCY(test, vm);
    test_opcode_VM_VECINCZ(test, vm);
    //test_opcode_VM_VECDECX(test, vm);
    //test_opcode_VM_VECDECY(test, vm);
    //test_opcode_VM_VECDECZ(test, vm);
    test_opcode_VM_MATLM(test, vm);
    test_opcode_VM_MATLI(test, vm);
    //test_opcode_VM_MATLS(test, vm);
    //test_opcode_VM_MATSL(test, vm);
    test_opcode_VM_MATM(test, vm);
    test_opcode_VM_MATMTV(test, vm);
    test_opcode_VM_MATMRX(test, vm);
    test_opcode_VM_MATMRY(test, vm);
    test_opcode_VM_MATMRZ(test, vm);
    test_opcode_VM_DRAW(test, vm);
}

void test_basic_program(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t running;

    uint8_t program[] = {
        VM_INTL, VM_REG0, 2,
        VM_INTL, VM_REG1, 3,
        VM_INTL, VM_REG2, 80,
        VM_INTL, VM_REG3, 16,
        VM_MATLM, VM_REG0, VM_REG1, VM_REG2,
        VM_DRAW, VM_REG0, VM_REG0,
        VM_FRWAIT,
        VM_JMP, 16
    };

    running = vrms_render_vm_exec(vm, program, 22);
    is_equal_uint8(test, running, 1, "running after INTL REG0");
    is_equal_uint32(test, vrms_render_vm_iregister_get(vm, VM_REG0), 2, "REG0 value is correct");
    is_equal_uint8(test, vrms_render_vm_last_opcode(vm), VM_INTL, "last opcode VM_INTL");

    running = vrms_render_vm_exec(vm, program, 22);
    is_equal_uint8(test, running, 1, "running after INTL REG1");
    is_equal_uint32(test, vrms_render_vm_iregister_get(vm, VM_REG1), 3, "REG1 value is correct");
    is_equal_uint8(test, vrms_render_vm_last_opcode(vm), VM_INTL, "last opcode VM_INTL");

    running = vrms_render_vm_exec(vm, program, 22);
    is_equal_uint8(test, running, 1, "running after INTL REG2");
    is_equal_uint32(test, vrms_render_vm_iregister_get(vm, VM_REG2), 80, "REG2 value is correct");
    is_equal_uint8(test, vrms_render_vm_last_opcode(vm), VM_INTL, "last opcode VM_INTL");

    running = vrms_render_vm_exec(vm, program, 22);
    is_equal_uint8(test, running, 1, "running after INTL REG3");
    is_equal_uint32(test, vrms_render_vm_iregister_get(vm, VM_REG3), 16, "REG3 value is correct");
    is_equal_uint8(test, vrms_render_vm_last_opcode(vm), VM_INTL, "last opcode VM_INTL");

    running = vrms_render_vm_exec(vm, program, 22);
    is_equal_uint8(test, running, 1, "running after LOADMM");
    is_equal_uint8(test, vrms_render_vm_last_opcode(vm), VM_MATLM, "last opcode VM_MATLM");

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
    is_equal_uint8(test, running, 1, "running after JMP");
    is_equal_uint8(test, vrms_render_vm_last_opcode(vm), VM_JMP, "last opcode VM_JMP");

    running = vrms_render_vm_exec(vm, program, 22);
    is_equal_uint8(test, running, 1, "running after DRAW (from JMP)");
    is_equal_uint8(test, vrms_render_vm_last_opcode(vm), VM_DRAW, "last opcode VM_DRAW");

    vrms_render_vm_reset(vm);
    is_equal_uint8(test, vrms_render_vm_last_opcode(vm), 0, "last opcode after reset zero");
    is_equal_uint32(test, vrms_render_vm_iregister_get(vm, VM_REG0), 0, "REG0 was reset");
    is_equal_uint32(test, vrms_render_vm_iregister_get(vm, VM_REG1), 0, "REG1 was reset");
    is_equal_uint32(test, vrms_render_vm_iregister_get(vm, VM_REG2), 0, "REG2 was reset");
    is_equal_uint32(test, vrms_render_vm_iregister_get(vm, VM_REG3), 0, "REG3 was reset");
}

void test_loop_program(test_harness_t* test, vrms_render_vm_t* vm) {
    uint8_t running;
    uint8_t nr_loops;
    uint8_t i;

    nr_loops = 5;
    uint8_t program[] = {
        VM_INTL, VM_REG0, 5,
        VM_INTDEC, VM_REG0,
        VM_JMPNZ, VM_REG0, 3
    };

    running = vrms_render_vm_exec(vm, program, 7);
    is_equal_uint8(test, running, 1, "running after INTL");
    is_equal_uint32(test, vrms_render_vm_iregister_get(vm, VM_REG0), nr_loops, "REG0 was set to 5");

    for (i = (nr_loops - 1); i > 0; i--) {
        running = vrms_render_vm_exec(vm, program, 7);
        is_equal_uint8(test, running, 1, "running after DEC");
        is_equal_uint32(test, vrms_render_vm_iregister_get(vm, VM_REG0), i, "REG0 value correct");

        running = vrms_render_vm_exec(vm, program, 7);
        is_equal_uint8(test, running, 1, "running after JMPNZ");
    }

    running = vrms_render_vm_exec(vm, program, 7);
    is_equal_uint8(test, running, 1, "running after DEC");
    is_equal_uint32(test, vrms_render_vm_iregister_get(vm, VM_REG0), 0, "REG0 0");

    running = vrms_render_vm_exec(vm, program, 7);
    is_equal_uint8(test, running, 1, "running after JMPNZ");

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

    test_all_opcodes(test, vm);
    test_basic_program(test, vm);
    test_loop_program(test, vm);

    test_harness_exit_with_status(test);
}
