#include <fstream>
#include <raylib.h>
#include <iostream>
#include <bitset>

//**********************CHIP-8 HARDWARE EMULATION **************************** */
uint8_t registers[16]{}; 
uint16_t memory[2048]{};
uint16_t index_register{}; 
uint16_t program_counter{0x200}; 
uint16_t stack[16]{};
uint8_t stack_pointer{};
uint8_t delay_timer{};
uint8_t sound_timer{};
uint32_t screen[64*32]{0};
uint16_t opcode;
uint8_t keypad[16]{
	49,50,51,52,
	81,82,83,84,
	65,55,67,68,
	90,91,92,93
};

/************************LOADS THE ROM INTO MEMORY**************************** */
/*
Testing the IBM logo CHIP-8 program provided by Timendus
Program completes in 20 cycles 
Tests the following OP Codes: 
--00E0 
--6xnn
-Annn
-7xnn
-Dxyn
*/
const unsigned int START_ADDRESS = 0x200; 
void Load_ROM(char const * file_name) {   //verified to print out correct num of OP Codes and load into memory
    std::ifstream file(file_name, std::ios::binary);

    if (file.is_open()) {

        file.seekg(0, file.end);
        int length{file.tellg()};
        file.seekg(0, file.beg);

        char buffer[length]{};

        file.read(buffer, length);
        file.close();

        for(int i = 0; i < length; i+=2) {
            
            std::cout << std::hex << (std::bitset<16>(buffer[i]<<buffer[i+1])).to_ulong() << "\n";
            memory[START_ADDRESS + i] = buffer[i];
            memory[START_ADDRESS + i + 1] = buffer[i + 1];
        }
        std::cout<<"**********************************************\n";
    }
}

/************************LOADS THE FONT INTO MEMORY**************************** */

uint8_t FONTS[80] = 
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

const unsigned int FONT_START = 0x50;
constexpr void load_font() {
  for (int i = 0; i < 80; ++i) {
    memory[FONT_START + i] = FONTS[i];
  }
}

/************************RANDOM NUMBER GENERATOR**************************** */

uint8_t random_number_generator() {
  return rand() % 256;
}

/************************DRAW SCREEN COMMAND**************************** */


void draw_screen() {
  for (int i = 0; i < 32; ++i) {
    for (int r = 0; r < 64; ++r) {
      
      if (screen[i*32 + r] == 1) {
        DrawRectangle(r*20, i*20, 20, 20, WHITE);
      }
      
      //std::cout<<(i*32 + r)<<" : " << screen[i*32 + r] << "\n";
    }
  }
}

/************************OP CODES**************************** */

void OP_00E0() { //clear screen
  memset(screen, 0, 64*32);
}

void OP_6xkk() { //Set regX = kk
  uint8_t reg = (opcode & 0x0F00u) >> 8u;
  uint8_t byte = opcode & 0x00FF;
  registers[reg] = byte;
}

void OP_Annn() { //set index to address
	uint16_t address = opcode & 0x0FFFu;

	index_register = address;
}

void OP_7xkk() { //regX += kk
  uint8_t reg = (opcode & 0x0F00u) >> 8u;
  uint8_t byte = opcode & 0x00FF;
  registers[reg] += byte;
}

void OP_Dxyn() //Draws sprites
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
	uint8_t height = opcode & 0x000Fu;
	uint8_t xPos = registers[Vx] % 64;
	uint8_t yPos = registers[Vy] % 32;
	registers[0xF] = 0;
	for (unsigned int row = 0; row < height; ++row) {
		uint8_t spriteByte = memory[index_register + row];

		for (unsigned int col = 0; col < 8; ++col) {
			uint8_t spritePixel = spriteByte & (0x80u >> col);
			uint32_t* screenPixel = &screen[(yPos + row) * 64 + (xPos + col)];
			if (spritePixel) {
				if (*screenPixel == 0xFFFFFFFF) {
					registers[0xF] = 1;
				}
				*screenPixel ^= 0xFFFFFFFF;
			}
		}
	}
}



int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int SCREEN_WIDTH{640};
    const int SCREEN_HEIGHT{320};
    
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "PIN-8 (A CHIP-8 Interpreter)");
    SetTargetFPS(1000);

    int i{0};
    while (!WindowShouldClose()) {
      BeginDrawing();
      screen[i++] = 1;
      draw_screen();
      EndDrawing();
    }
    return 0;
}