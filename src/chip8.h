#define NNN (opcode & 0x0FFF)	//address
#define KK (opcode & 0x00FF)	//8 bit constant
#define N (opcode & 0x000F)		//4 bit constant

// #define DX (px * SCALE)
// #define DY (py * SCALE)


class chip8{
	public:
		chip8();
		~chip8();

		bool drawFlag;

		void emulateCycle();
		void debugRender();
		bool loadApplication(const char * filename);

		unsigned char gfx[ 64 * 32];
		unsigned char key[16];

	private:
		unsigned short pc;
		unsigned short opcode;
		unsigned short I;
		unsigned short sp;

		unsigned char V[16];
		unsigned short stack[16];
		unsigned char memory[4096];

		unsigned char delay_timer;
		unsigned char sound_timer;

		void init();
};

