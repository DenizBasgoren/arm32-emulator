
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "emulib.h"



#define SCREEN_WIDTH	800
#define SCREEN_HEIGHT	600
#define N_SLOTS			4
#define OS_NAME			"Puhu OS"

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *texture[4]; // arr of 4 ptr to texture,
						// or ptr to arr of 4 texture
SDL_Event event;

// prototypes
static void refresh();
static void clean();
static void readKeyboard(uint32_t *value);

struct gpu {

	// 00
	const uint16_t screen_w;
	const uint16_t screen_h;
	const uint8_t n_slots;
	uint8_t _[11];

	// 10
	uint8_t red; // clear_call
	uint8_t green;
	uint8_t blue;
	uint8_t alpha;
	uint8_t selected_slot; // draw_call
	uint8_t update_slot; // update slot call
	uint8_t __[10];

	// 20
	uint16_t texture_width; // struct texture
	uint16_t texture_height;
	uint32_t texture_data_addr;
	uint8_t texture_channel; // 0 = rgb, 1 = rgba
	uint8_t ___[7];

	// 30
	uint16_t src_x; // texture pixel position
	uint16_t src_y;
	uint16_t src_w;
	uint16_t src_h;
	uint16_t target_x; // screen pixel position
	uint16_t target_y;
	uint16_t target_w;
	uint16_t target_h;
};

// static void refresh()
// {
// 	// for every texture?
// 	SDL_RenderCopy(renderer, texture, NULL, NULL);
// 	SDL_RenderPresent(renderer);
// }

static void clean()
{
	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
	SDL_RenderClear(renderer);
	// refresh(); // ?
}

static void readKeyboard(uint32_t *value) {
	
	SDL_PumpEvents();

	const uint8_t *state = (uint8_t*) SDL_GetKeyState(NULL);

	*value = \
		state[SDLK_a] << 31 |
		state[SDLK_b] << 30 |
		state[SDLK_c] << 29 |
		state[SDLK_d] << 28 |
		state[SDLK_e] << 27 |
		state[SDLK_f] << 26 |
		state[SDLK_g] << 25 |
		state[SDLK_h] << 24 |
		state[SDLK_i] << 23 |
		state[SDLK_j] << 22 |
		state[SDLK_k] << 21 |
		state[SDLK_l] << 20 |
		state[SDLK_m] << 19 |
		state[SDLK_n] << 18 |
		state[SDLK_o] << 17 |
		state[SDLK_p] << 16 |
		state[SDLK_q] << 15 |
		state[SDLK_r] << 14 |
		state[SDLK_s] << 13 |
		state[SDLK_t] << 12 |
		state[SDLK_u] << 11 |
		state[SDLK_v] << 10 |
		state[SDLK_w] << 9 |
		state[SDLK_x] << 8 |
		state[SDLK_y] << 7 |
		state[SDLK_z] << 6 |
		state[SDLK_UP] << 5 |
		state[SDLK_RIGHT] << 4 |
		state[SDLK_DOWN] << 3 |
		state[SDLK_LEFT] << 2 |
		state[SDLK_SPACE] << 1 |
		state[SDLK_ESCAPE];
}


int32_t system_init()
{

	if (SDL_Init( SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS)) {
		printf("No SDL :c %s", SDL_GetError());
	}

 	window = SDL_CreateWindow(OS_NAME,
                        300, // position
                        100,
                        SCREEN_WIDTH, SCREEN_HEIGHT,
                        SDL_WINDOW_RESIZABLE);
	
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	return 0;
}

void system_deinit()
{
	for (int i = 0; i< N_SLOTS; i++) {
    	SDL_DestroyTexture(texture[i]);
	}
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

	SDL_Quit();
}


int32_t load_program(char *path, uint8_t *rom, uint8_t *ram) {
	FILE *infile;
	uint32_t len;
	int32_t errorCode = 0;
	char cmd[512];

	if (rom == NULL || ram == NULL)
	{
		return -1;
	}

	sprintf(cmd, "arm-none-eabi-as -mcpu=cortex-m0 -mthumb %s -o armapp.o", path);
	if (system(cmd) != 0)
	{
		errorCode = -1;
		goto cleanup1;
	}

	sprintf(cmd, "arm-none-eabi-ld -T linker.ld armapp.o -o armapp.elf");
	if (system(cmd) != 0)
	{
		errorCode = -1;
		goto cleanup1;
	}

	sprintf(cmd, "arm-none-eabi-objcopy -O binary -j .text armapp.elf text.bin");
	if (system(cmd) != 0)
	{
		errorCode = -1;
		goto cleanup1;
	}

	sprintf(cmd, "arm-none-eabi-objcopy -O binary -j .data armapp.elf data.bin");
	if (system(cmd) != 0)
	{
		errorCode = -1;
		goto cleanup1;
	}

	// load text to rom
	infile = fopen("text.bin", "rb");
	fseek(infile, 0, SEEK_END);
	len = ftell(infile);
	fseek(infile, 0, SEEK_SET);

	if (fread(rom, 1, len, infile) != len)
	{
		errorCode = -1;
		goto cleanup2;
	}
	fclose(infile);


	// load data to ram
	infile = fopen("data.bin", "rb");
	fseek(infile, 0, SEEK_END);
	len = ftell(infile);
	fseek(infile, 0, SEEK_SET);

	if (fread(ram, 1, len, infile) != len)
	{
		errorCode = -1;
		goto cleanup2;
	}

	printf("Assembled successfully!\n");

	cleanup1:
	fclose(infile);

	cleanup2:
	#if defined(__unix__)
		system("rm armapp.o text.bin data.bin");
	#elif defined(_WIN32) || defined(_WIN64)
		system("del armapp.o text.bin data.bin");
	#endif

	return errorCode;
}

int32_t peripheral_write(uint32_t addr, uint32_t value)
{
	switch(addr)
	{
		// TODO
	}
	return -1;
}

int32_t peripheral_read(uint32_t addr, uint32_t *value)
{
	if (value == NULL)
		return -1;
	switch(addr)
	{
		case 0x40010020:
			readKeyboard(value);
			return 0;
	}
	return -1;
}
