#include <stdint.h>

#define NUM_REGS 8
#define INT_REG_SIZE NUM_REGS
#define FLO_REG_SIZE NUM_REGS
#define VEC_REG_SIZE NUM_REGS * 3
#define MAT_REG_SIZE NUM_REGS * 16

#define VM_REG0     0x00
#define VM_REG1     0x01
#define VM_REG2     0x02
#define VM_REG3     0x03
#define VM_REG4     0x04
#define VM_REG5     0x05
#define VM_REG6     0x06
#define VM_REG7     0x07

typedef enum vrms_render_vm_opcode {

    // VM run state instructions
    VM_HALT,    // Halt the program immediately. Does not reset the VM
    VM_FRWAIT,  // Signal to the VM that the frame drawing is complete
    VM_RESET,   // Reset the VM and move the instruction pointer to 0

    //Control flow instructions
    VM_JMP,     // Move the instruction pointer to a constant
    VM_JMPNZ,   // Move the instruction pointer to a constant, if a register is not zero
    VM_JMPZ,    // Move the instruction pointer to a constant, if a register is zero
    VM_JMPI,    // Move the instruction pointer to a value stored in an integer register

    // Integer register instructions
    VM_INTL,    // Load an integer register with a constant value
    VM_INTLI,   // Load an integer register with a value from another integer register
    VM_INTLS,   // Load an integer register from one of the system integer registers
    VM_INTDEC,  // Decrement an integer register
    VM_INTINC,  // Increment an integer register
    VM_INTMUL,  // Multiply two integer registers together and store the result in another integer register
    VM_INTDIVF, // Divide an integer register with another integer register and store in a float register
    VM_INTDIVII, // Divide an integer register with another integer register and store the quotent in one register and the remainder in another register
    VM_INTLFI,  // Load an integer register with the integer part of a float register
    VM_INTLFF,  // Load an integer register with the fractional part of a float register (4 decimal places, 0.9999 becomes 9999)

    // Float register instructions
    VM_FLOL,    // Load a float register with an integer constant
    VM_FLOADD,  // Add two float registers together and store in another float register
    VM_FLOMUL,  // Multiply two float registers together and store in another float register
    VM_FLODIV,  // Divide two float registers and store the result in another float register
    VM_FLOLII,  // Load the integer component of a float register from an integer register (zeros fractional part)
    VM_FLOLIF,  // Load the fractional component of a float register from an integer register (4 decimal places, if the integer is larger than 9999 it is floored to 9999)

    // Vector register instructions
    VM_VECL,    // Load a vector register with the values of three float registers
    VM_VECLX,   // Load the X component of a vector from a float register
    VM_VECLY,   // Load the Y component of a vector from a float register
    VM_VECLZ,   // Load the Z component of a vector from a float register
    VM_VECLXI,  // Load the X component of a vector from an integer register
    VM_VECLYI,  // Load the Y component of a vector from an integer register
    VM_VECLZI,  // Load the Z component of a vector from an integer register
    VM_VECLXC,  // Load the X component of a vector from an integer constant
    VM_VECLYC,  // Load the Y component of a vector from an integer constant
    VM_VECLZC,  // Load the Z component of a vector from an integer constant
    VM_VECINCX, // Increment the X component of a vector by a float register
    VM_VECINCY, // Increment the Y component of a vector by a float register
    VM_VECINCZ, // Increment the Z component of a vector by a float register
    VM_VECDECX, // Decrement the X component of a vector by a float register
    VM_VECDECY, // Decrement the X component of a vector by a float register
    VM_VECDECZ, // Decrement the X component of a vector by a float register

    // Matrix register instructions
    VM_MATLM,   // Load a matrix register with data from a matrix data object
    VM_MATLI,   // Load a matrix register with the identity matrix
    VM_MATLS,   // Load a matrix register from one of the system matrix registers
    VM_MATSL,   // Load one of the system matrix registers from a matrix register (needs a permission)
    VM_MATM,    // Multiply a matrix register with another matrix register and store in matrix register
    VM_MATMTV,  // Translate a matrix register by a vector register
    VM_MATMRX,  // Rotate a matrix around the X axis by a float register
    VM_MATMRY,  // Rotate a matrix around the Y axis by a float register
    VM_MATMRZ,  // Rotate a matrix around the Z axis by a float register

    // Drawing instructions
    VM_DRAW,    // Draw an object from a matrix register and an integer register
} vrms_render_vm_opcode_t;

typedef enum vrms_render_vm_exception {
    VM_X_ALL_OK             = 0x00,
    VM_X_INV_OPCODE         = 0x01,
    VM_X_REG_OUT_OF_BOUNDS  = 0x02
} vrms_render_vm_exception_t;

typedef enum vrms_render_vm_sysireg {
    VM_SYSIREG_USEC_ELAPSED = 0x00, // microseconds elapsed during frame
    VM_SYSIREG_USEC_ALLOC   = 0x01  // total allocation microseconds for frame
} vrms_render_vm_sysireg_t;

typedef enum vrms_render_vm_sysmreg {
    VM_SYSMREG_PROJECTION   = 0x00, // projection matrix
    VM_SYSMREG_VIEW         = 0x01  // view matrix
} vrms_render_vm_sysmreg_t;

typedef enum vrms_render_vm_vreg_part {
    VM_VREG_X = 0x00,
    VM_VREG_Y = 0x01,
    VM_VREG_Z = 0x02
} vrms_render_vm_vreg_part_t;

typedef struct vrms_render_vm vrms_render_vm_t;
struct vrms_render_vm {
    uint8_t running;
    uint32_t program_counter;
    uint8_t last_opcode;
    float* sysmregister[2];
    vrms_render_vm_sysireg_t sysiregister[1];
    uint32_t iregister[INT_REG_SIZE];
    float fregister[FLO_REG_SIZE];
    float vregister[VEC_REG_SIZE];
    float mregister[MAT_REG_SIZE];
    vrms_render_vm_exception_t exception;
    float* (*load_matrix)(vrms_render_vm_t* vm, uint32_t memory_id, uint32_t offset, void* user_data);
    void (*draw)(vrms_render_vm_t* vm, float* matrix, uint32_t object_id, void* user_data);
    void* user_data;
};

vrms_render_vm_t* vrms_render_vm_create();
void vrms_render_vm_destroy(vrms_render_vm_t* vm);
uint32_t vrms_render_vm_iregister_get(vrms_render_vm_t* vm, uint8_t reg);
void vrms_render_vm_iregister_set(vrms_render_vm_t* vm, uint8_t reg, uint32_t value);
float vrms_render_vm_fregister_get(vrms_render_vm_t* vm, uint8_t reg);
void vrms_render_vm_fregister_set(vrms_render_vm_t* vm, uint8_t reg, float value);
float vrms_render_vm_vregister_get(vrms_render_vm_t* vm, uint8_t reg, vrms_render_vm_vreg_part_t part);
void vrms_render_vm_vregister_set(vrms_render_vm_t* vm, uint8_t reg, vrms_render_vm_vreg_part_t part, float value);
float* vrms_render_vm_mregister_get(vrms_render_vm_t* vm, uint8_t reg);
void vrms_render_vm_mregister_set_identity(vrms_render_vm_t* vm, uint8_t reg);
uint32_t vrms_render_vm_sysiregister_get(vrms_render_vm_t* vm, vrms_render_vm_sysireg_t reg);
void vrms_render_vm_sysiregister_set(vrms_render_vm_t* vm, vrms_render_vm_sysireg_t reg, uint32_t value);
float* vrms_render_vm_sysmregister_get(vrms_render_vm_t* vm, vrms_render_vm_sysmreg_t reg);
void vrms_render_vm_sysmregister_set(vrms_render_vm_t* vm, vrms_render_vm_sysmreg_t reg, float* matrix);
uint8_t vrms_render_vm_last_opcode(vrms_render_vm_t* vm);
uint8_t vrms_render_vm_resume(vrms_render_vm_t* vm);
void vrms_render_vm_reset(vrms_render_vm_t* vm);
uint8_t vrms_render_vm_exec(vrms_render_vm_t* vm, uint8_t* program, uint32_t length);
void vrms_render_vm_alloc_ex_interrupt(vrms_render_vm_t* vm);
uint8_t vrms_render_vm_has_exception(vrms_render_vm_t* vm);
