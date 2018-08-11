#include <iostream>
#include "chip8.h"

unsigned char fontset[80]{
	0xF0, 0x90, 0x90, 0x90, 0xF0,		// 0
	0x20, 0x60, 0x20, 0x20, 0x70,		// 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0,		// 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0,		// 3
	0x90, 0x90, 0xF0, 0x10, 0x10,		// 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0,		// 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0,		// 6
	0xF0, 0x10, 0x20, 0x40, 0x40,		// 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0,		// 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0,		// 9
	0xF0, 0x90, 0xF0, 0x90, 0x90,		// A
	0xE0, 0x90, 0xE0, 0x90, 0xE0,		// B
	0xF0, 0x80, 0x80, 0x80, 0xF0,		// C
	0xE0, 0x90, 0x90, 0x90, 0xE0,		// D
	0xF0, 0x80, 0xF0, 0x80, 0xF0,		// E
	0xF0, 0x80, 0xF0, 0x80, 0x80		// F
};

//constructor
chip8::chip8(){

}

//deconstructor
chip8::~chip8(){

}

void chip8::init(){
	pc = 0x200;
	opcode = 0;
	sp = 0;
	I = 0;

	//clear display
	for(int i = 0; i < (sizeof(gfx)/sizeof(*gfx)); i++){
		gfx[i] = 0;
	}

	//clear stack
	for(int i = 0; i < (sizeof(stack)/sizeof(*stack)); i++){
		stack[i] = 0;
	}

	//clear memory
	for(int i = 0; i < (sizeof(memory)/sizeof(*memory)); i++){
		memory[i] = 0;
	}

	//load fontset
	for(int i = 0; i < (sizeof(fontset)/sizeof(*fontset)); i++){
		memory[i] = fontset[i];
	}

	//reset timers
	delay_timer = 0;
	sound_timer = 0;

	drawFlag = false; //by default no drawing has been done
}

void chip8::emulateCycle(){

	//fetch opcode
	// std::cout << memory[pc] << std::endl;
	// std::cout << memory[pc+1] << std::endl;

	opcode = memory[pc] << 8 | memory[pc + 1];

	//decode opcode
	switch(opcode & 0xF000){

		switch(opcode & 0x00F){	//compare the 4 bits

			case 0x0000: //00E0 clear screen
				for(int i = 0; i < (sizeof(gfx)/sizeof(*gfx)); i++){
					gfx[i] = 0;
				}

				drawFlag = true;
				pc += 2;
				break;

			case 0x000E: //return from subroutine
				--sp;
				pc = stack[sp];
				pc += 2;
				break;

			default:
				std::cout << "Unsupported Opcode!" << std::endl;
				exit(EXIT_FAILURE);


		case 0x1000: //jump to location NNN
			pc = opcode & NNN;
			printf("Jumping to %p", pc);
			break;
		}

		case 0x2000: //call subroutine at NNN
			pc = stack[sp];
			++sp;
			pc = opcode & NNN;
			break;

		case 0x3000: //skip next instruction if VX = kk (3XKK)

			if(V[(opcode & 0x0F00) >> 8] == KK){ pc += 4; }
			//2 nybbles is one byte... so shifting to the right 2 nybles (8 bytes) will allow us to compare since V is an unsigned char
			else{ pc +=2; }
			break;

		case 0x4000: //skip next instruction if VX != KK (4XKK)
			if(V[(opcode & 0x0F00) >> 8] != KK){
				pc += 4;
			}

			else{
				pc += 2;
			}
			break;

		case 0x5000: //skip next instruction if VX = VY (5XY0)
			if(V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4]){ //shift 4 bytes since Y is in the 2nd nybble
				pc += 4;
			}

			else{
				pc += 2;
			}
			break;

		case 0x6000: //set VX = KK
			V[(opcode & 0x0F00) >> 8] = KK;
			pc += 2;
			break;

		case 0x7000: //set VX = VX + KK (7XKK)
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] + KK;
			pc += 2;
			break;

		case 0x8000:
			switch(opcode & 0x000F){

				case 0x0000: //set VX = VY (8XY0)
					V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
					pc += 2;
					break;

				case 0x0001: //set VX = VX OR VY (8XY1)
					V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] | V[(opcode & 0x00F0) >> 4];
					pc += 2;
					break;

				case 0x0002: //set VX = VX AND VY
					V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] & V[(opcode & 0x00F0) >> 4];
					pc += 2;
					break;

				case 0x0003:
					V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] ^ V[(opcode & 0x00F0) >> 4];
					pc += 2;
					break;

				case 0x0004: //set VX = VX + VY, set VF = carry (8XY4)

					if(V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8])){ //subtracting VY > 8 bits - VX would be the same as comparing if VX + VY > 8 bits
						V[0xF] = 1;
					}

					else{
						V[0xF] = 0;
					}

					V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] + V[(opcode & 0x00F0) >> 4]; //only the lowest 8 bits of the result are kept, and stored in VX
					pc += 2;
					break;

				case 0x0005: //set VX = VX - VY, set VF = NOT borrow(8XY5)
					if(V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4]){ //if VVX > VY, then VF is set to 1, otherwise 0
						V[0xF] = 1;
					}

					else{
						V[0xF] = 0;
					}

					V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] - V[(opcode & 0x00F0) >> 4]; //VY is subtracted from VX, and the results are stored in VX
					pc += 2;
					break;

			}
}

chip8 myChip8;

int main(){

	myChip8.emulateCycle();

	// int arr[5]{1,2,3,4,5};

	// //there are 4 bytes in an interger... since there are 5 elements in the array, 5 * 4 = 20
	// std::cout << sizeof(arr) << std::endl;
	// //*arr always returns the first element in the array 
	// std::cout << sizeof(*arr) << std::endl;
	// std::cout << *arr << std::endl;

	// //dividing the two will show the amount of items in the array. 20 bytes / 4 bytes for each element
	// std::cout << (sizeof(arr)/sizeof(*arr)) << std::endl;

	// return 0;

}
