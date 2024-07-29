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
      if (screen[i*r] != 0) {
        DrawRectangle(i*20, r*20, 20, 20, WHITE);
      }
    }
  }
}
/*
void draw_screen() {
  DrawRectangle(0,0, 20, 20, WHITE);
}
*/
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int SCREEN_WIDTH{640};
    const int SCREEN_HEIGHT{320};

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "PIN-8");

    SetTargetFPS(1);
    int i{0};
    while (!WindowShouldClose())
    {
        if (i < 180) { ++i; }
        screen[i] = 1;
        BeginDrawing();
          ClearBackground(BLACK);
          draw_screen();
        EndDrawing();
    }
    CloseWindow();
    return 0;
}