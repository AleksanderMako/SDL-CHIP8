#include "stdint.h"
#include <string>
#ifndef ch8
#define ch8
using namespace std;
class Chip8
{
private:
    uint8_t memory[4096]; // main program memory
    uint8_t V[16];        // registers
    uint16_t memory_address_register;
    uint8_t sound_delay; // sound delay register
    uint8_t time_delay;  // time delay register
    uint16_t pc;         // program counter
    uint8_t sp;          // stack pointer
    uint16_t stack[16];  // stack
    uint16_t opcode;
public:
    uint8_t gfx_buffer[64 *32]; // grafics buffer 
    uint8_t keypad[16]; // keypad
    bool drawFlag; 
    Chip8() ;
    void Initialize(); 
    bool LoadRom(string path);
    void Emulate();
    

};
#endif 