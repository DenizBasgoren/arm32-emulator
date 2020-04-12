#include "emulib.h"

typedef struct {
    int32_t reg[16];
    int32_t cpsr;
}tCPU;

uint8_t rom[0x200000];
uint8_t ram[0x100000];
tCPU cpu;


int32_t execute_next(void);


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



//Fetches an instruction from ROM, decodes and executes it
int32_t execute_next(void)
{
#if 0
    uint32_t pc;
    uint32_t sp;
	uint32_t inst;
	uint32_t ra, rb, rc;
	uint32_t rm, rd, rn, rs;
	uint32_t op;
	uint16_t X;

	pc = cpu.reg[15];

	X = pc - 2;
	inst = rom[X] | rom[X+1] << 8;
	pc += 2;
	cpu.reg[15] = pc;
	
	//ORR
	if ((inst & 0xFFC0) == 0x4300)
	{
		rd = (inst >> 0) & 0x7;
		rm = (inst >> 3) & 0x7;
		fprintf(stderr, "orrs r%u,r%u\n", rd, rm);
		ra = cpu.reg[rd];
		rb = cpu.reg[rm];
		rc = ra | rb;
		cpu.reg[rd] = rc;
		//Update the flags here if necessary (to be filled)
		return(0);
	}
	//Rest of the instructions to be implemented here
	fprintf(stderr, "invalid instruction 0x%08X 0x%04X\n", pc - 4, inst);
#endif

#define GET_BITS(bits, start, offset) (((bits) >> ((start) - (offset) + 1)) & ((1 << (offset)) - 1))

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
#define PSR (cpu.cpsr)		// This register is called the Program Status Register, page 19

	uint16_t inst = rom[PC - 2] | rom[PC - 1] << 8;
	PC += 2;

	// AND
	if (GET_BITS(inst, 15, 10) == 0b0100'0000'00)
	{
		uint8_t rd = GET_BITS(inst, 2, 3);
		uint8_t rm = GET_BITS(inst, 5, 3);
		uint8_t ra = cpu.reg[rd];
		uint8_t rb = cpu.reg[rd];
		uint8_t rc = ra & rb;
		cpu.reg[rd] = rc;
		// TODO: Set the zero flag to 1 if rc is 0
		return 0;
	}
	// EOR
	else if (GET_BITS(inst, 15, 10) == 0b0100'0000'01)
	{
		uint8_t rd = GET_BITS(inst, 2, 3);
		uint8_t rm = GET_BITS(inst, 5, 3);
		uint8_t ra = cpu.reg[rd];
		uint8_t rb = cpu.reg[rd];
		uint8_t rc = ra ^ rb;
		cpu.reg[rd] = rc;
		// TODO: Set the zero flag to 1 if rc is 0
		return 0;
	}
	// ORR
	else if (GET_BITS(inst, 15, 10) == 0b0100'0011'00)
	{
		uint8_t rd = GET_BITS(inst, 2, 3);
		uint8_t rm = GET_BITS(inst, 5, 3);
		uint8_t ra = cpu.reg[rd];
		uint8_t rb = cpu.reg[rd];
		uint8_t rc = ra | rb;
		cpu.reg[rd] = rc;
		// TODO: Set the zero flag to 1 if rc is 0
		return 0;
	}

	return(1);
}


