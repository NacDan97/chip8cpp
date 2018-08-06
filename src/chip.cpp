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

	drawFlag = true;
}

void chip8::emulateCycle(){

	//fetch opcode

	// std::cout << memory[pc] << std::endl;
	// std::cout << memory[pc+1] << std::endl;

	opcode = memory[pc] << 8 | memory[pc + 1];

	//decode opcode

	switch(opcode & 0xF000){



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
