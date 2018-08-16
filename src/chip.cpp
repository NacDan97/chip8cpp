#include <iostream>
#include <cstdlib>
#include <cstdio>
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
		}
		break;


		case 0x1000: //jump to location NNN
			pc = opcode & NNN;
			std::printf("Jumping to %p", pc);
			break;

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
					if(V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4]){ //if VX > VY, then VF to 1, otherwise 0
						V[0xF] = 1; //if there is no borrow, set VF = 1
					}

					else{
						V[0xF] = 0; //otherwise set VF to 0
					}

					V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] - V[(opcode & 0x00F0) >> 4]; //VY is subtracted from VX, and the results are stored in VX
					pc += 2;
					break;

				case 0x0006: //set VX = VX SHR (shift right) 1; VF set to least significant bit of VX (8XY6)
					V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1; // When you & VX with the 1 bit (0x1), you get the least significant bit.	
					V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] >> 1;
					pc += 2;
					break; 

				case 0x0007: //set VX = VY - VX, set VF = NOT borrow (8XY7)
					if(V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8]){ //if VY > VX, then set VF to 1, otherwise 0
						V[0xF] = 1; //if there is no borrow set VF to 1
					}

					else{
						V[0xF] = 0; //otherwise set VF to 0
					}

					V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8]; //VX = VY - VF
					pc += 2;
					break;

				case 0x000E: //set VX = VX SHL (shift left) 1 (8XYE)
					V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7; //store most significant bit of VX in VF
					V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] << 1; //shift VX left one bit 
					pc += 2;
					break;
			}
			break;

		case 0x9000: //skip next instruction if VX != VY (9XY0)
			if(V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4]){
				pc += 4;
			}

			else{
				pc += 2;
			}
			break;

		case 0xA000: //set I = NNN (ANNN)
			I = NNN;
			pc += 2;
			break;

		case 0xB000: //pc = NNN + V0 (BNNN)
			pc = NNN + V[0];
			break;

		case 0xC000: //VX = random byte (between 0 to 255) AND KK
			V[(opcode & 0x0F00) >> 8] = (std::rand() % (0xFF + 1)) & KK; //range from  0 to 255 (0xFF)
			pc += 2;
			break;

		case 0xD000: //draw sprite at (VX, VY) that has width of 8 pixels and height of N pixels. (DXYN)
		{
			//TODO: give short but detailed explanation what's going on just so that I will remember
			unsigned short x = V[(opcode & 0x0F00) >> 8];
			unsigned short y = V[(opcode & 0x00F0) >> 4];
			unsigned short height = opcode & 0x000F;
			unsigned short pixel;

			V[0xF] = 0;
			for(int yline = 0; yline < height; yline++){
				pixel = memory[I + yline];
				for(int xline = 0; xline < 8; xline++){
					if(pixel & (0x80 >> xline) != 0){
						if(gfx[(x + xline) + (y + yline) * 64] == 1){
							V[0xF] = 1;
							gfx[(x + xline) + (y + yline) * 64] ^= 1;
						}
					}
				}
			}
			drawFlag = true;
			pc += 2;
		}
			break;

		case 0xE000:
			switch(opcode & 0x00FF){

				case 0x009E: //skips next instruction if key stored in VX is pressed (0xEX9E)
					if(key[V[(opcode & 0x0F00) >> 8]] == 1){
						pc += 4;
					}

					else{
						pc += 2;
					}
					break;

				case 0x00A1: //skips next instruction if the key stored in VX isn't pressed (0xEXA1)
					if(key[V[(opcode & 0x0F00) >> 8]] == 0){
						pc += 4;
					}

					else{
						pc += 2;
					}
					break;
			}
			break;

		case 0xF000:
			switch(opcode & 0x00FF){

				case 0x0007: //sets VX to the value of the delay timer
					V[(opcode & 0x0F00) >> 8] = delay_timer;
					pc += 2;
					break;

				case 0x000A: //a key press is awaited, and then stored in VX

					for(int i = 0; i < key.size(); i++){
						if(key[i] == 1){
							V[(opcode & 0x00F0) >> 8] = i;
							pc += 2;
							break;
						}
					}
					// TODO : define macro for VX and others
					std::printf("awaiting key press to be stored in V[%x]", (opcode & 0x0F00) >> 8 );
					break;


				case 0x0015: //set delay timer to VX
					delay_timer = V[(opcode & 0x0F00) >> 8];
					pc += 2;
					break;

				case 0x0018: //set the sound timer to VX
					sound_timer = V[(opcode & 0x0F00) >> 8];
					pc += 2;
					break;

				case 0x001E: //set I = I + VX
					I = I + V[(opcode & 0x0F00) >> 8];
					pc += 2;
					break;

				case 0x0029: 							//sets I to the location of the sprite for character VX
					I = V[(opcode & 0x0F00) >> 8] * 5;	//characters set from hex 0 - 5 are represented in 4x5 font
					pc += 2;
					break;

				case 0x0033:
					pc += 2;
					break;

				case 0x0055:
					pc += 2;
					break;

				case 0x0065:
					pc += 2;
					break;
			}
			break;

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
