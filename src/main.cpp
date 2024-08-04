#include <fstream>
#include <fstream>
#include <raylib.h>
#include <iostream>
#include <cstring>
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
uint8_t screen[64*32]{0};
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
            
            memory[START_ADDRESS + i] = buffer[i];
            memory[START_ADDRESS + i + 1] = buffer[i + 1];
			//std::cout<<std::hex<<std::bitset<16>(buffer[i] << buffer[i+1]).to_ulong() << "\n";
        }
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
	for (int y = 0; y < 32; ++y) {
		for (int x = 0; x < 64; ++x) {
		
		if (screen[(64*y) + x] == 1) {
			DrawRectangle(x*20, y*20, 20, 20, WHITE);
			
		}
		
		//std::cout<<(i*32 + r)<<" : " << screen[i*32 + r] << "\n";
		}
	}
}

/************************OP CODES**************************** */

void OP_Dxyn() //Draws sprites   ***************************** formula for (x,y) : 64*y + x
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
	uint8_t height = opcode & 0x000Fu;

	uint8_t xPos{registers[Vx]};
	if (registers[Vx] > 64) {xPos = registers[Vx] % 64;}
	uint8_t yPos{registers[Vy]};
	if (registers[Vy] > 32) {yPos = registers[Vx] % 32;}

	registers[0xF] = 0;

	for (int r = 0; r < height; r++) {
		uint8_t spriteByte = memory[index_register + r];

		std::cout << "Printed : " << r << " times\n";
		std::cout << std::bitset<8>(spriteByte) << "\n";
		
		for (int i = 0; i < 8; ++i) {

			uint8_t maskedBit = (spriteByte & (static_cast<uint8_t>(1) << (7-i))) >>(7-i);
			std::cout << std::bitset<1>(maskedBit) << "\n";

			screen[(64*(yPos+r) + xPos + i)] = maskedBit;


			/*
			if ((screen[(64*(yPos+r)) + xPos + i]) ^ ((spriteByte & static_cast<uint8_t>(1) <<(7-i)) >> (7-i))) {
				registers[0xF] = 1;
			}
			*/
			//screen[(64*(yPos+r)) + xPos + i] = (screen[(64*(yPos+r)) + xPos + i]) ^ ((spriteByte & static_cast<uint8_t>(1) <<(7-i)) >> (7-i));
		}
	}
}

void drawSmile(uint8_t x, uint8_t y) {
	screen[64*(y+0) + (x + 1)] = 1;
	screen[64*(y+0) + (x + 3)] = 1;
	screen[64*(y+1) + (x + 1)] = 1;
	screen[64*(y+1) + (x + 3)] = 1;
	screen[64*(y+3) + (x + 0)] = 1;
	screen[64*(y+3) + (x + 4)] = 1;
	screen[64*(y+4) + (x + 1)] = 1;
	screen[64*(y+4) + (x + 2)] = 1;
	screen[64*(y+4) + (x + 3)] = 1;
}

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int SCREEN_WIDTH{1280};
    const int SCREEN_HEIGHT{640};
	SetTargetFPS(10);
	Load_ROM("/home/doppler/C++ Projects/PIN-8/external/programs/2-ibm-logo.ch8");
	load_font();


	
	memory[0] = 0b11111111;
	memory[1] = 0b00000000;
	memory[2] = 0b11111111;
	memory[3] = 0b00000000;
	memory[4] = 0b00111100;
	memory[5] = 0b00000000;
	memory[6] = 0b00111100;
	memory[7] = 0b00000000;
	memory[8] = 0b00111100;
	memory[9] = 0b00000000;
	memory[10] = 0b00111100;
	memory[11] = 0b00000000;
	memory[12] = 0b11111111;
	memory[13] = 0b00000000;
	memory[14] = 0b11111111;
	
	index_register = 0;
	registers[0] = 0;
	registers[1] = 0;
	opcode = 0xD01F;
	
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "PIN-8 (A CHIP-8 Interpreter)");

	OP_Dxyn();
	//drawSmile(5,5);
	
	while(!WindowShouldClose()) {
		//Cycle();
		BeginDrawing();
			ClearBackground(BLACK);
			draw_screen();
		EndDrawing();
	}
	return 0;
	

}