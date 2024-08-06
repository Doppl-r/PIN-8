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

void OP_00E0() { //clear screen
  std::memset(screen, 0, 64*32);
  std::cout<<"clear screen\n";
}

void OP_00EE() { //return from a subroutine
  --stack_pointer;
  program_counter = stack[stack_pointer];
  std::cout<<"return from subroutine\n";
}

void OP_1nnn() { //Jump to nnn
  program_counter = opcode & 0x0FFFu;
  std::cout<<"jump to nnn\n";
}

void OP_2nnn() { //call subroutine
  stack[stack_pointer] = program_counter;
  ++stack_pointer;
  program_counter = opcode & 0x0FFFu;
  std::cout<<"call subroutine\n";
}

void OP_3xkk() { //skip next instruction if regX == kk
  uint8_t reg = (opcode & 0x0F00u) >> 8u;
  uint8_t byte = (opcode & 0x00FFu);
  if (registers[reg] == byte) {
    program_counter+=2;
  }
  std::cout<<"skip next instruction if regX == kk\n";
}

void OP_4xkk() { //Skip next instructoin if regX != kk
  uint8_t reg = (opcode & 0x0F00u) >> 8u;
  uint8_t byte = (opcode & 0x00FFu);
  if (registers[reg] != byte) {
    program_counter+=2;
  }
  std::cout<<"Skip next instructoin if regX != kk\n";
}

void OP_5xy0() { //skip next command if reg x is equal to reg y
  uint8_t reg = (opcode & 0x0F00u) >> 8u;
  uint8_t reg2 = (opcode & 0x00F0u) >> 4u;
  if (registers[reg] == registers[reg2]) {
    program_counter+=2;
  }
  std::cout<<"Skip next instructoin if regX == regY\n";
}

void OP_6xkk() { //Set regX = kk
  uint8_t reg = std::bitset<8>((opcode & 0x0F00u) >> 8u).to_ulong();
  uint8_t byte = std::bitset<8>((opcode & 0x00FFu)).to_ulong();

  registers[reg] = byte;
  std::cout<<"Set regX = kk\n";
}

void OP_7xkk() { //regX += kk
  uint8_t reg = std::bitset<8>((opcode & 0x0F00u) >> 8u).to_ulong();
  uint8_t byte = std::bitset<8>((opcode & 0x00FFu)).to_ulong();
  registers[reg] += byte;
  std::cout<<"Set regX += kk\n";
}

void OP_8xy0() { //regX = regY
  uint8_t reg = (opcode & 0x0F00u) >> 8u;
  uint8_t reg2 = (opcode & 0x00F0u) >> 4u;
  registers[reg] = registers[reg2];
  std::cout<<"Set regX = regY\n";
}

void OP_8xy1() { //regX = regX OR regY
  uint8_t reg = (opcode & 0x0F00u) >> 8u;
  uint8_t reg2 = (opcode & 0x00F0u) >> 4u;
  registers[reg] |= registers[reg2];
  std::cout<<"Set regX = regX OR regY\n";
}

void OP_8xy2() { //Set regX = regX AND regY
  uint8_t reg = (opcode & 0x0F00u) >> 8u;
  uint8_t reg2 = (opcode & 0x00F0u) >> 4u;
  registers[reg] &= registers[reg2];
  std::cout<<"Set regX = regX AND regY\n";
}

void OP_8xy3() { // Set regX = regX XOR regY
  uint8_t reg = (opcode & 0x0F00u) >> 8u;
  uint8_t reg2 = (opcode & 0x00F0u) >> 4u;
  registers[reg] ^= registers[reg2];
  std::cout<<"Set regX = regX XOR regY\n";
}

void OP_8xy4() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	uint16_t sum = registers[Vx] + registers[Vy];

	registers[Vx] = sum & 0x00FFu;
	if (sum > 255)
	{
		registers[0xF] = 1;
	}
	else
	{
		registers[0xF] = 0;
	}

	
	std::cout<<"Set regX = regX + regY, regF set to carry\n";
}

void OP_8xy5() { // set regX-=regY  set regF  = NOT borrow
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	uint8_t valX = registers[Vx];
	uint8_t valY = registers[Vy];

	registers[Vx] -= registers[Vy];

	if (valX >= valY) {
		registers[0xF] = 1;
	}
	else {
		registers[0xF] = 0;
	}
	std::cout<<"Set regX -= regY, set regF = NOT borrow\n";
}

void OP_8xy6() { // set regX = regX SHR 1
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t carry = registers[Vx] & 0b1u;
	
	registers[Vx] >>= 1;

	registers[0xF] = carry;
}

void OP_8xy7() { //set regX = regY - regX set regF = NOT borrow
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	uint8_t valX = registers[Vx];
	uint8_t valY = registers[Vy];

	registers[Vx] = valY - valX;

	if (valY >= valX) {
		registers[0xF] = 1;
	}
	else {
		registers[0xF] = 0;
	}
	
}

void OP_8xyE() { //set regX  = regX SHL 1
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	uint8_t carry = (registers[Vx] & 0b10000000u) >> 7u;

	registers[Vx] <<= 1;

	registers[0xF] = carry;
}

void OP_9xy0() { //skip next instruction if regX != regY
	uint8_t reg1 = (opcode & 0x0F00u) >> 8u;
	uint8_t reg2 = (opcode & 0x00F0u) >> 4u;

	if (registers[reg1] != registers[reg2]) {
		program_counter += 2;
	}
}

void OP_Annn() { //set index to address
	uint16_t address = opcode & 0x0FFFu;

	index_register = address;
	std::cout<<"set index to register\n";
}

void OP_Bnnn() { //jump to location
	uint16_t address = opcode & 0x0FFFu;
	program_counter = registers[0] + address;
}

void OP_Cxkk() { //set regX to random value and kk
	uint8_t reg1 = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	registers[reg1] = random_number_generator() & byte;
}

void OP_Dxyn() //Draws sprites   ***************************** formula for (x,y) : 64*y + x
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
	uint8_t height = opcode & 0x000Fu;
	std::cout<< "Val in Vx: " << std::bitset<8>(registers[Vx]) << "\n";
	std::cout<< "Val in Vy: " << std::bitset<8>(registers[Vy]) << "\n";
	uint8_t xPos{registers[Vx]};
	if (registers[Vx] > 64) {xPos = registers[Vx] % 64;}
	uint8_t yPos{registers[Vy]};
	if (registers[Vy] > 32) {yPos = registers[Vx] % 32;}

	std::cout << "xPos: " << std::bitset<8>(xPos) << "\n";
	std::cout << "ypos: " << std::bitset<8>(yPos) << "\n";

	registers[0xF] = 0;

	for (int r = 0; r < height; r++) {
		uint8_t spriteByte = memory[index_register + r];

		for (int i = 0; i < 8; ++i) {
			uint8_t maskedBit = (spriteByte & (static_cast<uint8_t>(1) << (7-i))) >>(7-i);

			screen[(64*(yPos+r) + xPos + i)] = screen[(64*(yPos+r) + xPos + i)] ^ maskedBit;

			if (screen[(64*(yPos+r) + xPos + i)] ^ maskedBit) {
				registers[0xF] = 1;
			}
		}
	}
}

void OP_Ex9E() { //skip next instruction if the value of a key = regX
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t key = registers[Vx];
	if (keypad[key]) {
		program_counter += 2;
	}
}

void OP_ExA1() { //skip next instruction if vascreelue of key is not equal to regX
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t key = registers[Vx];
	if (!keypad[key]) {
		program_counter += 2;
	}
}

void OP_Fx07() { //set regX to delay timer
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	registers[Vx] = delay_timer;
}

void OP_Fx0A() { //wait for a key press, store value in regX ------------------------------------------------------------------------------------------
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	int key = 0;

	while(key == 0) {
		key = GetKeyPressed();

	}

	for (int i = 0; i < 16; ++i) {
		if (keypad[i] == key) {
			registers[Vx] = key;
		}
	}
	std::cout<< "looking for key press\n";

}

void OP_Fx15() { //set delay timer = regX
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	delay_timer = registers[Vx];
}

void OP_Fx18() { //set sound_timer = regX
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	sound_timer = registers[Vx];
}

void OP_Fx1E() { //set index = index + regX
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	index_register += registers[Vx];
}

void OP_Fx29() { //set index_register equal to sprite location
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t digit = registers[Vx];
	index_register = FONT_START + (5 * digit);
}

void OP_Fx33() { //stores mem location 
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t value = registers[Vx];
	// Ones-place
	memory[index_register + 2] = value % 10;
	value /= 10;
	// Tens-place
	memory[index_register + 1] = value % 10;
	value /= 10;
	// Hundreds-place
	memory[index_register] = value % 10;
}

void OP_Fx55() { //store registers reg0 through regX in memory starting at index_register
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	for (uint8_t i = 0; i <= Vx; ++i)
	{
		memory[index_register + i] = registers[i];
	}
}

void OP_Fx65() { //read registers reg0 through regX from memory starting at location I 
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	for (uint8_t i = 0; i <= Vx; ++i)
	{
		registers[i] = memory[index_register + i];
	}
}

/************************FUNCTION POINTER SETUP**************************** */

void OP_NULL() { std::cout<<"CALLED NULL FUNCTION"<<"\n";}; //null function

decltype(&OP_NULL) fpt[0xF+1]{&OP_NULL}; //for codes numbering 0-F

decltype(&OP_NULL) fpt8[0xE + 1]{&OP_NULL}; //for codes beginning with 8
 
decltype(&OP_NULL) fpt00[0xE + 1]{&OP_NULL}; //for codes that begin with 00

decltype(&OP_NULL) fptE[0xE + 1]{&OP_NULL}; //for codes that begin with E

decltype(&OP_NULL) fptF[0x65 + 1]{&OP_NULL}; //for codes that begin with F

void Table0()
{
	(*fpt00[opcode & 0x000Fu])();
}

void Table8()
{
	(*fpt8[opcode & 0x000Fu])();
}

void TableE()
{
	(*fptE[opcode & 0x000Fu])();
}

void TableF()
{
	(*fptF[opcode & 0x00FFu])();
}

void initialize_table() {
	fpt[0x1] = &OP_1nnn; //Initializing 0-F Codes
	fpt[0x2] = &OP_2nnn;
	fpt[0x3] = &OP_3xkk;
	fpt[0x4] = &OP_4xkk;
	fpt[0x5] = &OP_5xy0;
	fpt[0x6] = &OP_6xkk;
	fpt[0x7] = &OP_7xkk;
	fpt[0x9] = &OP_9xy0;
	fpt[0xA] = &OP_Annn;
	fpt[0xB] = &OP_Bnnn;
	fpt[0xC] = &OP_Cxkk;
	fpt[0xD] = &OP_Dxyn;

	fpt8[0x0] = &OP_8xy0; //Initializing 8 : 0-E Codes
	fpt8[0x1] = &OP_8xy1;
	fpt8[0x2] = &OP_8xy2;
	fpt8[0x3] = &OP_8xy3;
	fpt8[0x4] = &OP_8xy4;
	fpt8[0x5] = &OP_8xy5;
	fpt8[0x6] = &OP_8xy6;
	fpt8[0x7] = &OP_8xy7;
	fpt8[0xE] = &OP_8xyE;

	fpt00[0x0] = &OP_00E0; //Initializing 00 OP codes
	fpt00[0xE] = &OP_00EE;

	fptE[0x1] = &OP_ExA1; //initializing E OP codes
	fptE[0xE] = &OP_Ex9E;

	fptF[0x07] = &OP_Fx07; //Initializing F OP codes
	fptF[0x0A] = &OP_Fx0A;
	fptF[0x15] = &OP_Fx15;
	fptF[0x18] = &OP_Fx18;
	fptF[0x1E] = &OP_Fx1E;
	fptF[0x29] = &OP_Fx29;
	fptF[0x33] = &OP_Fx33;
	fptF[0x55] = &OP_Fx55;
	fptF[0x65] = &OP_Fx65;



	fpt[0x0] = &Table0;
	fpt[0x8] = &Table8;
	fpt[0xE] = &TableE;
	fpt[0xF] = &TableF;
}

/************************CYCLE FUNCTION**************************** */
void Cycle()
{
	// Fetch
	uint8_t firstHalf{static_cast<uint8_t>(memory[program_counter])};
	uint8_t secondHalf{static_cast<uint8_t>(memory[program_counter+1])};

	opcode = (static_cast<uint16_t>(firstHalf)<<8u) | secondHalf;

	// Increment the PC before we execute anything
	program_counter += 2;
	
	// Decode and Execute
	std::cout<< "-------------------\n"<<"Reading opcode : " << std::hex << opcode << "\n";
	(*fpt[(opcode & 0xF000u) >> 12u])();
	

	if (delay_timer > 0) {
		--delay_timer; 
	}

	if (sound_timer > 0) {
		--sound_timer;
	}
}

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int SCREEN_WIDTH{1280};
    const int SCREEN_HEIGHT{640};
	SetTargetFPS(20);
	initialize_table();
	Load_ROM("/home/doppler/C++ Projects/PIN-8/external/programs/6-keypad.ch8");
	load_font();
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "PIN-8 (A CHIP-8 Interpreter)");

	
	while(!WindowShouldClose()) {
		Cycle();
		BeginDrawing();
			ClearBackground(BLACK);
			draw_screen();
		EndDrawing();
	}
	return 0;
	

}