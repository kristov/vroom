#include <stdint.h>

#define NUM_REGS 8
#define MATRIX_FLOATS NUM_REGS * 16

#define VM_HALT     0x00
#define VM_LOADI    0x01
#define VM_LOADMM   0x02
#define VM_DRAW     0x03
#define VM_FRWAIT   0x04
#define VM_GOTO     0x05

#define VM_REG0     0x00
#define VM_REG1     0x01
#define VM_REG2     0x02
#define VM_REG3     0x03
#define VM_REG4     0x04
#define VM_REG5     0x05
#define VM_REG6     0x06
#define VM_REG7     0x07

typedef struct vrms_render_vm vrms_render_vm_t;
struct vrms_render_vm {
    uint8_t running;
    uint32_t program_counter;
    uint8_t last_opcode;
    float* sysmregister[2];
    float* mregister[NUM_REGS];
    uint8_t iregister[NUM_REGS];
    float* (*load_matrix)(vrms_render_vm_t* vm, uint32_t memory_id, uint32_t offset, void* user_data);
    void (*draw)(vrms_render_vm_t* vm, float* matrix, uint32_t object_id, void* user_data);
    void* user_data;
};

vrms_render_vm_t* vrms_render_vm_create();
void vrms_render_vm_destroy(vrms_render_vm_t* vm);
uint32_t vrms_render_vm_iregister_value(vrms_render_vm_t* vm, uint8_t reg);
float* vrms_render_vm_mregister_value(vrms_render_vm_t* vm, uint8_t reg);
float* vrms_render_vm_sysmregister_value(vrms_render_vm_t* vm, uint8_t reg);
void vrms_render_vm_sysmregister_set(vrms_render_vm_t* vm, uint8_t reg, float* matrix);
uint8_t vrms_render_vm_last_opcode(vrms_render_vm_t* vm);
uint8_t vrms_render_vm_resume(vrms_render_vm_t* vm);
void vrms_render_vm_reset(vrms_render_vm_t* vm);
uint8_t vrms_render_vm_exec(vrms_render_vm_t* vm, uint8_t* program, uint32_t length);

