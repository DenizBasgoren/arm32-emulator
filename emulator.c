#include "emulib.h"

// start (in order of documentation), offset ( number of bits from start to direction of 0)
#define GET_BITS(bits, start, offset) (((bits) >> ((start) - (offset) + 1)) & ((1 << (offset)) - 1))
#define SET_BIT(bits, n) ((bits) |= (1 << (n)))
#define RESET_BIT(a,b) ((a) &= ~(1ULL<<(b)))

#define R0  (cpu.reg[ 0])
#define R1  (cpu.reg[ 1])
#define R2  (cpu.reg[ 2])
#define R3  (cpu.reg[ 3])
#define R4  (cpu.reg[ 4])
#define R5  (cpu.reg[ 5])
#define R6  (cpu.reg[ 6])
#define R7  (cpu.reg[ 7])
#define R8  (cpu.reg[ 8])
#define R9  (cpu.reg[ 9])
#define R10 (cpu.reg[10])
#define R11 (cpu.reg[11])
#define R12 (cpu.reg[12])
#define SP  (cpu.reg[13])
#define LR  (cpu.reg[14])
#define PC  (cpu.reg[15])
#define FLG (cpu.cpsr)      // This register is called the Program Status Register
#define FLG_N (31)
#define FLG_Z (30)
#define FLG_C (29)
#define FLG_V (28)

#define ROM_MAX 0x1FFFFFFF
#define ROM_MIN 0x00000000

#define RAM_MAX 0x3FFFFFFF
#define RAM_MIN 0x20000000

#define PER_MAX 0x5FFFFFFF
#define PER_MIN 0x40000000



typedef struct {
    int32_t reg[16];
    int32_t cpsr;
}tCPU;

uint8_t rom[0x200000];
uint8_t ram[0x100000];
tCPU cpu;


int32_t execute_next(void);
void update_nz_flags(int32_t reg);


//Emulator main function
int32_t main(int32_t argc, char* argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "input assembly file not specified\n");
        return(1);
    }

    memset(rom, 0xFF, sizeof(rom));

    system_init();

    if (load_program(argv[1], rom) < 0)
    {
        return(1);
    }
    
    memset(ram, 0xFF, sizeof(ram));

    cpu.cpsr = 0;

    cpu.reg[14] = 0xFFFFFFFF;
    //First 4 bytes in ROM specifies initializes the stack pointer
    cpu.reg[13] = rom[0] | rom[1] << 8 | rom[2] << 16 | rom[3] << 24;
    //Following 4 bytes in ROM initializes the PC
    cpu.reg[15] = rom[4] | rom[5] << 8 | rom[6] << 16 | rom[7] << 24;
    cpu.reg[15] += 2;

    while (1)
    {
        if (execute_next()) break;
    }

    system_deinit();

    return(0);
}


void update_nz_flags(int32_t reg) {
        if(reg == 0) SET_BIT(FLG, FLG_Z);
        else RESET_BIT(FLG, FLG_Z);
        if (reg >= (1 << 31)) SET_BIT(FLG, FLG_N);
        else RESET_BIT(FLG, FLG_N);
}

//Fetches an instruction from ROM, decodes and executes it
int32_t execute_next(void)
{

    uint16_t inst = rom[PC - 2] | rom[PC - 1] << 8;
    PC += 2;

    // TODO: operands should be uint32_t or int32_t?

    // todo: check operator precedence: cast, >> 
    // LSL
    if (GET_BITS(inst, 15, 5) == 0b00000) {
        uint8_t immed = GET_BITS(inst, 10, 5);
        uint8_t rm = GET_BITS(inst, 5, 3);
        uint8_t rd = GET_BITS(inst, 2, 3);

        uint32_t rc = (uint32_t) cpu.reg[rm] << immed;
        cpu.reg[rd] = rc;

        update_nz_flags(rc);
        // todo: impl carry, conditions..
        return 0;
    }

    // LSR
    else if (GET_BITS(inst, 15, 5) == 0b00000) {
        uint8_t immed = GET_BITS(inst, 10, 5);
        uint8_t rm = GET_BITS(inst, 5, 3);
        uint8_t rd = GET_BITS(inst, 2, 3);

        uint32_t rc = (uint32_t) cpu.reg[rm] >> immed;
        cpu.reg[rd] = rc;

        update_nz_flags(rc);
        // todo: impl carry, conditions..
        return 0;
    }

    // ASR
    else if (GET_BITS(inst, 15, 5) == 0b00010) {
        uint8_t immed = GET_BITS(inst, 10, 5);
        uint8_t rm = GET_BITS(inst, 5, 3);
        uint8_t rd = GET_BITS(inst, 2, 3);

        int32_t rc = cpu.reg[rm] >> immed;
        cpu.reg[rd] = rc;

        update_nz_flags(rc);
        // todo: impl carry, conditions..
        return 0;
    }

    // ADD
    else if (GET_BITS(inst, 15, 7) == 0b0001100) {
        uint8_t rm = GET_BITS(inst, 8, 3);
        uint8_t rn = GET_BITS(inst, 5, 3);
        uint8_t rd = GET_BITS(inst, 2, 3);

        uint32_t ra = cpu.reg[rm];
        uint32_t rb = cpu.reg[rn];
        uint32_t rc = ra + rb;
        cpu.reg[rd] = rc;

        update_nz_flags(rc);
        // todo: impl carry, overflo, conditions..
        return 0;
    }

    // SUB
    else if (GET_BITS(inst, 15, 7) == 0b0001101) {
        uint8_t rm = GET_BITS(inst, 8, 3);
        uint8_t rn = GET_BITS(inst, 5, 3);
        uint8_t rd = GET_BITS(inst, 2, 3);

        uint32_t ra = cpu.reg[rm];
        uint32_t rb = cpu.reg[rn];
        uint32_t rc = ra - rb;
        cpu.reg[rd] = rc;

        update_nz_flags(rc);
        // todo: impl carry, overflo, conditions..
        return 0;
    }

    // ADD
    else if (GET_BITS(inst, 15, 7) == 0b0001110) {
        uint8_t immed = GET_BITS(inst, 8, 3);
        uint8_t rn = GET_BITS(inst, 5, 3);
        uint8_t rd = GET_BITS(inst, 2, 3);

        uint32_t ra = cpu.reg[rn];
        uint32_t rc = ra + immed;
        cpu.reg[rd] = rc;

        update_nz_flags(rc);
        // todo: impl carry, overflo, conditions..
        return 0;
    }

    // SUB
    else if (GET_BITS(inst, 15, 7) == 0b0001111) {
        uint8_t immed = GET_BITS(inst, 8, 3);
        uint8_t rn = GET_BITS(inst, 5, 3);
        uint8_t rd = GET_BITS(inst, 2, 3);

        uint32_t ra = cpu.reg[rn];
        uint32_t rc = ra - immed;
        cpu.reg[rd] = rc;

        update_nz_flags(rc);
        // todo: impl carry, overflo, conditions..
        return 0;
    }

    // MOV
    else if (GET_BITS(inst, 15, 5) == 0b00100) {
        uint8_t rd = GET_BITS(inst, 10, 3);
        uint8_t immed = GET_BITS(inst, 7, 8);

        cpu.reg[rd] = immed;

        // nz flags?
        // todo: impl carry, overflo, conditions..
        return 0;
    }

    // CMP
    else if (GET_BITS(inst, 15, 5) == 0b00101) {
        uint8_t rn = GET_BITS(inst, 10, 3);
        uint8_t immed = GET_BITS(inst, 7, 8);

        uint32_t dif = cpu.reg[rn] - immed;

        update_nz_flags(dif);
        // todo: impl carry, overflo, conditions..
        return 0;
    }

    // ADD
    else if (GET_BITS(inst, 15, 5) == 0b00110) {
        uint8_t rd = GET_BITS(inst, 10, 3);
        uint8_t immed = GET_BITS(inst, 7, 8);

        uint32_t ra = cpu.reg[rd];
        ra += immed;

        cpu.reg[rd] = ra;
        update_nz_flags(ra);
        // todo: impl carry, overflo, conditions..
        return 0;
    }

    // SUB
    else if (GET_BITS(inst, 15, 5) == 0b00111) {
        uint8_t rd = GET_BITS(inst, 10, 3);
        uint8_t immed = GET_BITS(inst, 7, 8);

        uint32_t ra = cpu.reg[rd];
        ra -= immed;

        cpu.reg[rd] = ra;
        update_nz_flags(ra);
        // todo: impl carry, overflo, conditions..
        return 0;
    }

    // ANDS
    else if (GET_BITS(inst, 15, 10) == 0b0100000000)
    {
        uint8_t rd = GET_BITS(inst, 2, 3);
        uint8_t rm = GET_BITS(inst, 5, 3);
        uint32_t ra = cpu.reg[rd];
        uint32_t rb = cpu.reg[rm];
        uint32_t rc = ra & rb;
        cpu.reg[rd] = rc;
        
        update_nz_flags(rc);


        return 0;
    }
    // EORS
    else if (GET_BITS(inst, 15, 10) == 0b0100000001)
    {
        uint8_t rd = GET_BITS(inst, 2, 3);
        uint8_t rm = GET_BITS(inst, 5, 3);
        uint32_t ra = cpu.reg[rd];
        uint32_t rb = cpu.reg[rm];
        uint32_t rc = ra ^ rb;
        cpu.reg[rd] = rc;
        
        update_nz_flags(rc);
        return 0;
    }
    // LSLS
    else if (GET_BITS(inst, 15, 10) == 0b0100000010)
    {
        uint8_t rd = GET_BITS(inst, 2, 3);
        uint8_t rm = GET_BITS(inst, 5, 3);
        uint32_t ra = cpu.reg[rd];
        uint32_t rb = cpu.reg[rm];
        uint32_t rc = ra << rb;
        cpu.reg[rd] = rc;
        
        update_nz_flags(rc);
        return 0;
    }
    // LSRS
    else if (GET_BITS(inst, 15, 10) == 0b0100000011)
    {
        uint8_t rd = GET_BITS(inst, 2, 3);
        uint8_t rm = GET_BITS(inst, 5, 3);
        uint32_t ra = cpu.reg[rd];
        uint32_t rb = cpu.reg[rm];
        uint32_t rc = ra >> rb;
        cpu.reg[rd] = rc;
        
        update_nz_flags(rc);
        return 0;
    }
    // ASRS
    else if (GET_BITS(inst, 15, 10) == 0b0100000100)
    {
        uint8_t rd = GET_BITS(inst, 2, 3);
        uint8_t rm = GET_BITS(inst, 5, 3);
        int32_t ra = cpu.reg[rd];
        int32_t rb = cpu.reg[rm];
        int32_t rc = ra >> rb;
        cpu.reg[rd] = rc;
        
        update_nz_flags(rc);
        return 0;
    }
    // ADCS
    else if (GET_BITS(inst, 15, 10) == 0b0100000101)
    {
        uint8_t rd = GET_BITS(inst, 2, 3);
        uint8_t rm = GET_BITS(inst, 5, 3);
        int32_t ra = cpu.reg[rd];
        int32_t rb = cpu.reg[rm];
        int32_t rc = ra + rb;
        cpu.reg[rd] = rc + GET_BITS(FLG, FLG_C, 1);
        
        update_nz_flags(rc);
        // TODO: Add the carry flag code

        if(ra > 0 && rb > 0 && rc < 0) SET_BIT(FLG, FLG_V);
        else if(ra < 0 && rb < 0 && rc > 0) SET_BIT(FLG, FLG_V);
        else RESET_BIT(FLG, FLG_V);

        return 0;
    }
    // SBCS
    else if (GET_BITS(inst, 15, 10) == 0b0100000110)
    {
        uint8_t rd = GET_BITS(inst, 2, 3);
        uint8_t rm = GET_BITS(inst, 5, 3);
        int32_t ra = cpu.reg[rd];
        int32_t rb = cpu.reg[rm];
        int32_t rc = rb - ra;
        cpu.reg[rd] = rc - GET_BITS(FLG, FLG_C, 1);
        
        update_nz_flags(rc);
        // TODO: Add the overflow and carry flag code

        return 0;
    }
    // RORS
    else if (GET_BITS(inst, 15, 10) == 0b0100000111)
    {
        uint8_t rd = GET_BITS(inst, 2, 3);
        uint8_t rm = GET_BITS(inst, 5, 3);
        uint32_t ra = cpu.reg[rd];
        uint32_t rb = cpu.reg[rm] % 32;
        uint32_t rc = (ra >> rb) | (ra << (32 - rb));
        cpu.reg[rd] = rc;
        
        update_nz_flags(rc);
        // TODO: Add the carry flag code

        return 0;
    }
    // TSTS
    else if (GET_BITS(inst, 15, 10) == 0b0100001000)
    {
        uint8_t rd = GET_BITS(inst, 2, 3);
        uint8_t rm = GET_BITS(inst, 5, 3);
        uint32_t ra = cpu.reg[rd];
        uint32_t rb = cpu.reg[rm];
        uint32_t rc = ra & rb;
        cpu.reg[rd] = rc;
        
        update_nz_flags(rc);
        return 0;
    }

    // todo: NEG is same as MVN??
    // NEG
    else if (GET_BITS(inst, 15, 10) == 0b0100001001)
    {
        uint8_t rd = GET_BITS(inst, 2, 3);
        uint8_t rm = GET_BITS(inst, 5, 3);
        uint32_t ra = cpu.reg[rm];
        uint32_t rc = ~ra;
        cpu.reg[rd] = rc;
        
        update_nz_flags(rc);
        return 0;
    }
    // CMP
    else if (GET_BITS(inst, 15, 10) == 0b0100001010)
    {
        uint8_t rd = GET_BITS(inst, 2, 3);
        uint8_t rm = GET_BITS(inst, 5, 3);
        int32_t ra = cpu.reg[rd];
        int32_t rb = cpu.reg[rm];
        int32_t rc = ra - rb;
        
        update_nz_flags(rc);
        // TODO: Add the overflow and carry flag code
        
        return 0;
    }
    // CMN
    else if (GET_BITS(inst, 15, 10) == 0b0100001011)
    {
        uint8_t rd = GET_BITS(inst, 2, 3);
        uint8_t rm = GET_BITS(inst, 5, 3);
        int32_t ra = cpu.reg[rd];
        int32_t rb = cpu.reg[rm];
        int32_t rc = ra + rb;
        
        update_nz_flags(rc);
        // TODO: Add the overflow and carry flag code
        
        return 0;
    }
    // ORRS
    else if (GET_BITS(inst, 15, 10) == 0b0100001100)
    {
        uint8_t rd = GET_BITS(inst, 2, 3);
        uint8_t rm = GET_BITS(inst, 5, 3);
        uint32_t ra = cpu.reg[rd];
        uint32_t rb = cpu.reg[rm];
        uint32_t rc = ra | rb;
        cpu.reg[rd] = rc;
        
        update_nz_flags(rc);
        return 0;
    }
    // MUL
    else if (GET_BITS(inst, 15, 10) == 0b0100001101)
    {
        uint8_t rd = GET_BITS(inst, 2, 3);
        uint8_t rm = GET_BITS(inst, 5, 3);
        uint32_t ra = cpu.reg[rd];
        uint32_t rb = cpu.reg[rm];
        uint32_t rc = ra * rb;
        cpu.reg[rd] = rc;
        
        update_nz_flags(rc);
        return 0;
    }
    // BICS
    else if (GET_BITS(inst, 15, 10) == 0b0100001110)
    {
        uint8_t rd = GET_BITS(inst, 2, 3);
        uint8_t rm = GET_BITS(inst, 5, 3);
        uint32_t ra = cpu.reg[rd];
        uint32_t rb = cpu.reg[rm];
        uint32_t rc = ra & ~rb;
        cpu.reg[rd] = rc;
        
        update_nz_flags(rc);
        return 0;
    }

    // MVNS
    else if (GET_BITS(inst, 15, 10) == 0b0100001111)
    {
        uint8_t rd = GET_BITS(inst, 2, 3);
        uint8_t rm = GET_BITS(inst, 5, 3);
        uint32_t ra = cpu.reg[rm];
        uint32_t rc = ~ra;
        cpu.reg[rd] = rc;
        
        update_nz_flags(rc);
        return 0;
    }

    // CPY // ???????
    else if (GET_BITS(inst, 15, 10) == 0b0100011000)
    {
        uint8_t rd = GET_BITS(inst, 2, 3);
        uint8_t rm = GET_BITS(inst, 5, 3);
        uint32_t ra = cpu.reg[rm];
        cpu.reg[rd] = ra;
        
        update_nz_flags(ra);
        // TODO; CARRY, V FLAGS
        return 0;
    }

    // ADD
    else if (GET_BITS(inst, 15, 10) == 0b0100010001)
    {
        uint8_t rd = GET_BITS(inst, 2, 3);
        uint8_t rm = GET_BITS(inst, 5, 3);
        uint32_t ra = cpu.reg[rd];
        uint32_t rb = cpu.reg[rm+8];
        
        ra += rb;
        cpu.reg[rd] = ra;
        
        update_nz_flags(ra);
        // TODO; CARRY, V FLAGS

        return 0;
    }

    // MOV
    else if (GET_BITS(inst, 15, 10) == 0b0100011001)
    {
        uint8_t rd = GET_BITS(inst, 2, 3);
        uint8_t rm = GET_BITS(inst, 5, 3);
        uint32_t ra = cpu.reg[rd];
        uint32_t rb = cpu.reg[rm+8];
        
        ra = rb;
        cpu.reg[rd] = ra;
        
        update_nz_flags(ra);
        // TODO; CARRY, V FLAGS

        return 0;
    }

    // ADD
    else if (GET_BITS(inst, 15, 10) == 0b0100010010)
    {
        uint8_t rd = GET_BITS(inst, 2, 3);
        uint8_t rm = GET_BITS(inst, 5, 3);
        uint32_t ra = cpu.reg[rd+8];
        uint32_t rb = cpu.reg[rm];
        
        ra += rb;
        cpu.reg[rd+8] = ra;
        
        update_nz_flags(ra);
        // TODO; CARRY, V FLAGS

        return 0;
    }

    // MOV
    else if (GET_BITS(inst, 15, 10) == 0b0100011010)
    {
        uint8_t rd = GET_BITS(inst, 2, 3);
        uint8_t rm = GET_BITS(inst, 5, 3);
        uint32_t ra = cpu.reg[rd+8];
        uint32_t rb = cpu.reg[rm];
        
        ra = rb;
        cpu.reg[rd+8] = ra;
        
        update_nz_flags(ra);
        // TODO; CARRY, V FLAGS

        return 0;
    }

    // ADD
    else if (GET_BITS(inst, 15, 10) == 0b0100010011)
    {
        uint8_t rd = GET_BITS(inst, 2, 3);
        uint8_t rm = GET_BITS(inst, 5, 3);
        uint32_t ra = cpu.reg[rd+8];
        uint32_t rb = cpu.reg[rm+8];
        
        ra += rb;
        cpu.reg[rd+8] = ra;
        
        update_nz_flags(ra);
        // TODO; CARRY, V FLAGS

        return 0;
    }

    // MOV
    else if (GET_BITS(inst, 15, 10) == 0b0100011011)
    {
        uint8_t rd = GET_BITS(inst, 2, 3);
        uint8_t rm = GET_BITS(inst, 5, 3);
        uint32_t ra = cpu.reg[rd+8];
        uint32_t rb = cpu.reg[rm+8];
        
        ra = rb;
        cpu.reg[rd+8] = ra;
        
        update_nz_flags(ra);
        // TODO; CARRY, V FLAGS

        return 0;
    }

    // CMP
    else if (GET_BITS(inst, 15, 10) == 0b0100010101)
    {
        uint8_t rd = GET_BITS(inst, 2, 3);
        uint8_t rm = GET_BITS(inst, 5, 3);
        uint32_t ra = cpu.reg[rd];
        uint32_t rb = cpu.reg[rm+8];
        
        uint32_t dif = ra - rb;

        update_nz_flags(dif);
        // TODO; CARRY, V FLAGS

        return 0;
    }

    // CMP
    else if (GET_BITS(inst, 15, 10) == 0b0100010110)
    {
        uint8_t rd = GET_BITS(inst, 2, 3);
        uint8_t rm = GET_BITS(inst, 5, 3);
        uint32_t ra = cpu.reg[rd+8];
        uint32_t rb = cpu.reg[rm];
        
        uint32_t dif = ra - rb;

        update_nz_flags(dif);
        // TODO; CARRY, V FLAGS

        return 0;
    }

    // CMP
    else if (GET_BITS(inst, 15, 10) == 0b0100010111)
    {
        uint8_t rd = GET_BITS(inst, 2, 3);
        uint8_t rm = GET_BITS(inst, 5, 3);
        uint32_t ra = cpu.reg[rd+8];
        uint32_t rb = cpu.reg[rm+8];
        
        uint32_t dif = ra - rb;

        update_nz_flags(dif);
        // TODO; CARRY, V FLAGS

        return 0;
    }
    
    // BX
    else if(GET_BITS(inst, 15, 9) == 0b010001110){
        uint8_t rm = GET_BITS(inst, 6, 4);
        PC = cpu.reg[rm];

        return 0;
    }

    // BLX
    else if(GET_BITS(inst, 15, 9) == 0b010001111){
        uint8_t rm = GET_BITS(inst, 6, 4);
        LR = PC;
        PC += cpu.reg[rm];

        return 0;
    }

    // LDR
    else if (GET_BITS(inst, 15, 5) == 0b01001)
    {
        uint8_t rd = GET_BITS(inst, 10, 3);
        uint8_t immed = GET_BITS(inst, 7, 8);
        
        cpu.reg[rd] = rom[PC + 4 * immed];

        return 0;
    }

    // STR
    else if (GET_BITS(inst, 15, 7) == 0b0101000)
    {
        uint8_t rm = GET_BITS(inst, 8, 3);
        uint8_t rn = GET_BITS(inst, 5, 3);
        uint8_t rd = GET_BITS(inst, 2, 3);

        uint32_t addr = cpu.reg[rm] + cpu.reg[rn];

        if(addr >= ROM_MIN && addr <= ROM_MAX)
            rom[addr - ROM_MIN] = cpu.reg[rd];
        else if(addr >= RAM_MIN && addr <= RAM_MAX)
            ram[addr - RAM_MIN] = cpu.reg[rd];
        else if(addr >= PER_MIN && addr <= PER_MAX)
            return peripheral_write(addr, cpu.reg[rd]);

        return 0;
    }

    // LDR
    else if (GET_BITS(inst, 15, 7) == 0b0101100)
    {
        uint8_t rm = GET_BITS(inst, 8, 3);
        uint8_t rn = GET_BITS(inst, 5, 3);
        uint8_t rd = GET_BITS(inst, 2, 3);

        uint32_t addr = cpu.reg[rm] + cpu.reg[rn];

        if(addr >= ROM_MIN && addr <= ROM_MAX)
            cpu.reg[rd] = rom[addr - ROM_MIN];
        else if(addr >= RAM_MIN && addr <= RAM_MAX)
            cpu.reg[rd] = ram[addr - RAM_MIN];
        else if(addr >= PER_MIN && addr <= PER_MAX)
            return peripheral_read(addr, &cpu.reg[rd]);

        return 0;
    }

    // STR
    else if (GET_BITS(inst, 15, 5) == 0b01100)
    {
        uint8_t immed = GET_BITS(inst, 10, 5);
        uint8_t rn = GET_BITS(inst, 5, 3);
        uint8_t rd = GET_BITS(inst, 2, 3);

        uint32_t addr = cpu.reg[rn] + 4 * immed;

        if(addr >= ROM_MIN && addr <= ROM_MAX)
            rom[addr - ROM_MIN] = cpu.reg[rd];
        else if(addr >= RAM_MIN && addr <= RAM_MAX)
            ram[addr - RAM_MIN] = cpu.reg[rd];
        else if(addr >= PER_MIN && addr <= PER_MAX)
            return peripheral_write(addr, cpu.reg[rd]);

        return 0;
    }

    // LDR
    else if (GET_BITS(inst, 15, 5) == 0b01101)
    {
        uint8_t immed = GET_BITS(inst, 10, 5);
        uint8_t rn = GET_BITS(inst, 5, 3);
        uint8_t rd = GET_BITS(inst, 2, 3);

        uint32_t addr = cpu.reg[rn] + 4 * immed;

        if(addr >= ROM_MIN && addr <= ROM_MAX)
            cpu.reg[rd] = rom[addr - ROM_MIN];
        else if(addr >= RAM_MIN && addr <= RAM_MAX)
            cpu.reg[rd] = ram[addr - RAM_MIN];
        else if(addr >= PER_MIN && addr <= PER_MAX)
            return peripheral_read(addr, &cpu.reg[rd]);

        return 0;
    }

    // STR
    else if (GET_BITS(inst, 15, 5) == 0b10010)
    {
        uint8_t rd = GET_BITS(inst, 10, 3);
        uint8_t immed = GET_BITS(inst, 7, 8);

        uint32_t addr = SP + 4 * immed;

        if(addr >= ROM_MIN && addr <= ROM_MAX)
            rom[addr - ROM_MIN] = cpu.reg[rd];
        else if(addr >= RAM_MIN && addr <= RAM_MAX)
            ram[addr - RAM_MIN] = cpu.reg[rd];
        else if(addr >= PER_MIN && addr <= PER_MAX)
            return peripheral_write(addr, cpu.reg[rd]);

        return 0;
    }

    // LDR
    else if (GET_BITS(inst, 15, 5) == 0b10011)
    {
        uint8_t rd = GET_BITS(inst, 10, 3);
        uint8_t immed = GET_BITS(inst, 7, 8);

        uint32_t addr = SP + 4 * immed;

        if(addr >= ROM_MIN && addr <= ROM_MAX)
            cpu.reg[rd] = rom[addr - ROM_MIN];
        else if(addr >= RAM_MIN && addr <= RAM_MAX)
            cpu.reg[rd] = ram[addr - RAM_MIN];
        else if(addr >= PER_MIN && addr <= PER_MAX)
            return peripheral_read(addr, &cpu.reg[rd]);

        return 0;
    }

    // ADD
    else if (GET_BITS(inst, 15, 5) == 0b10100)
    {
        uint8_t rd = GET_BITS(inst, 10, 3);
        uint8_t immed = GET_BITS(inst, 7, 8);

        cpu.reg[rd] = PC + immed * 4;

        update_nz_flags(cpu.reg[rd]);

        return 0;
    }

    // ADD
    else if (GET_BITS(inst, 15, 5) == 0b10101)
    {
        uint8_t rd = GET_BITS(inst, 10, 3);
        uint8_t immed = GET_BITS(inst, 7, 8);

        cpu.reg[rd] = SP + immed * 4;

        update_nz_flags(cpu.reg[rd]);

        return 0;
    }

    // ADD
    else if (GET_BITS(inst, 15, 9) == 0b101100000)
    {
        uint8_t immed = GET_BITS(inst, 6, 7);

        SP = SP + immed * 4;

        update_nz_flags(SP);

        return 0;
    }

    // SUB
    else if (GET_BITS(inst, 15, 9) == 0b101100001)
    {
        uint8_t immed = GET_BITS(inst, 6, 7);

        SP = SP - immed * 4;

        update_nz_flags(SP);

        return 0;
    }

    // PUSH
    else if (GET_BITS(inst, 15, 7) == 0b1011010)
    {
        uint8_t r = GET_BITS(inst, 8, 1);
        uint8_t list = GET_BITS(inst, 7, 8);
        uint32_t addr = SP;

        if(r == 1){
            addr -= 4;
            ram[addr] = LR;
        }
        for(int i = 7; i >= 0; i--){
            if((list & (1 << i)) != 0){
                addr -= 4;
                ram[addr] = cpu.reg[i];
            }
        }

        SP = addr;
    }

    // POP
    else if (GET_BITS(inst, 15, 7) == 0b1011110)
    {
        uint8_t r = GET_BITS(inst, 8, 1);
        uint8_t list = GET_BITS(inst, 7, 8);
        uint32_t addr = SP;

        for(int i = 0; i < 8; i++){
            if((list & (1 << i)) != 0){
                ram[addr] = cpu.reg[i];
                addr += 4;
            }
        }
        if(r == 1){
            ram[addr] = PC;
            addr += 4;
        }

        SP = addr;
    }

    fprintf(stderr, "invalid instruction 0x%08X 0x%04X\n", PC - 4, inst);
    return 1;
}
