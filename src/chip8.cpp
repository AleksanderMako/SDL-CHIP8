#include <iostream>
#include "chip8.h"
#include <fstream>
#include <random>
unsigned char chip8_fontset[80] =
    {
        0xF0, 0x90, 0x90, 0x90, 0xF0, //0
        0x20, 0x60, 0x20, 0x20, 0x70, //1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
        0x90, 0x90, 0xF0, 0x10, 0x10, //4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
        0xF0, 0x10, 0x20, 0x40, 0x40, //7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
        0xF0, 0x90, 0xF0, 0x90, 0x90, //A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
        0xF0, 0x80, 0x80, 0x80, 0xF0, //C
        0xE0, 0x90, 0x90, 0x90, 0xE0, //D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
        0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};
Chip8 ::Chip8() {}

void Chip8 ::Initialize()
{
    sp = 0;
    pc = 0x200;
    memory_address_register = 0;
    opcode = 0;
    // clear memory

    for (int i = 0; i < 4096; ++i)
    {
        memory[i] = 0;
    }

    // clear stack , registers and keypad

    for (int i = 0; i < 16; ++i)
    {
        stack[i] = 0;
        V[i] = 0;
        keypad[i] = 0;
    }

    // clear gx buffer

    for (int i = 0; i < 2048; ++i)
    {
        gfx_buffer[i] = 0;
    }
    // load font set
    for (int i = 0; i < 80; ++i)
    {
        memory[i] = chip8_fontset[i];
    }

    // clear timers

    sound_delay = 0;
    time_delay = 0;

    srand(time(NULL));
}
// load rom in memory

bool Chip8 ::LoadRom(string path)
{
    Initialize();
    cout << "LOdinf rom" << endl;
    ifstream file;
    file.open(path, ios::binary);

    if (!file.is_open())
    {
        cerr << "failed to open rom File ";
    }
    file.seekg(0, ios::end);
    const int rom_size = file.tellg();
    file.seekg(0, ios::beg);

    // if rom fits in memory
    if ((4096 - 512) > rom_size)
    {
        //allocate buffer to store rom

        char *buff = new char[rom_size];
        // read to a buffer
        file.read(buff, rom_size);

        // copy buffer to memory

        for (int i = 0; i < rom_size; ++i)
        {
            memory[i + 512] = (uint8_t)buff[i];
        }

        free(buff);
    }
    else
    {
        cerr << "rom is to big to read in memory " << endl;
        return false;
    }

    file.close();
    return true;
}

void Chip8 ::Emulate()
{

    // each opcode is two bytes => fetch pc and pc+1 and merge them
    // bitwise or to merge two bit sequences
    opcode = memory[pc] << 8 | memory[pc + 1];
    cout << "opcode is : " << opcode << endl;
    // bitwise & to select x  bits
    switch (opcode & 0xF000) //select first 4 bits to determine opcode type:  0,1,2,3,4 etc
    {
    // opcode series 0
    case 0x0000:
        // select last 4 bits
        switch (opcode & 0x000F)
        {
        case 0x0000:
            for (int i = 0; i < 2048; ++i)
            {
                gfx_buffer[i] = 0;
            }
            drawFlag = true;
            pc += 2;
            break;
        case 0x000E:
            --sp;
            pc = stack[sp];
            pc += 2;
            break;
        default:
            cout << "unknown opcode in 0 series  " << endl;
            break;
        }

        break;
        // 0 series end

    case 0x1000:
        // jump to location nnn , select first 12 bits
        pc = 0x0FFF & opcode;
        break;

    case 0x2000:
        // call subroutine at nnn , select first 12 bits
        stack[sp] = pc;
        ++sp;
        pc = opcode & 0x0FFF;

        break;
    case 0x3000:
        // skip next instruction if VX= kk
        // x is equal to the 4 lower bits of the high byte
        // to use it as index extract bits shift the result 8 to the left to fit within range
        if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
        {
            pc += 4;
        }
        else
            pc += 2;
        break;
    case 0x4000:
        // skip next instruction if Vx != nn
        if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
        {
            pc += 4;
        }
        else
            pc += 2;
        break;

    case 0x5000:
        // skip next instruction if Vx = Vy
        if ((V[(opcode & 0x0F00) >> 8]) == (V[(opcode & 0x00F0 )>> 4]))
        {
            pc += 4;
        }
        else
            pc += 2;
        break;

    case 0x6000:
        // set vx = kk

        V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
        pc += 2;
        break;

    case 0x7000:
        // set vx = vx +kk
        V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
        pc += 2;
        break;

    case 0x8000:
        // start instruction series 8, by extracting the last 4 bits
        switch (opcode & 0x000F)
        {
        case 0x0000:
            // set vx = vy

            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;
        case 0x0001:
            // vx = vx bitwise or vy
            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] | V[(opcode & 0x00F0 )>> 4];
            pc += 2;
            break;
        case 0x0002:
            // vx = vx bitwise and vy
            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] & V[(opcode & 0x00F0 )>> 4];
            pc += 2;

            break;
        case 0x0003:
            // vx = vx bitwise xor vy
            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] ^ V[(opcode & 0x00F0) >> 4];
            pc += 2;

            break;
        case 0x0004:
            // set vx = vx + vy and vf carry if sum is > 255
            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] + V[(opcode & 0x00F0) >> 4];
            if ((V[(opcode & 0x00F0) >> 4]) > (0xFF - (V[(opcode & 0x0F00) >> 8])))
            {
                V[0xF] = 1;
            }
            else
                V[0xF] = 0;
            pc += 2;
            break;
        case 0x0005:
            // set vx = vx -vy
            if ((V[(opcode & 0x00F0 )>> 4]) > (V[(opcode & 0x0F00) >> 8]))
            {
                V[0xF] = 0;
            }
            else
                V[0xF] = 1;

            V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
            pc += 2;

            break;

        case 0x0006:
            // store least significant bit and then divide by 2
            V[0xF] = (V[(opcode & 0x0F00) >> 8] & 0x1);
            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] >> 1;
            pc += 2;
            break;

        case 0x0007:
            if ((V[(opcode & 0x00F0) >> 4]) < (V[(opcode & 0x0F00) >> 8]))
            {
                V[0xF] = 0;
            }
            else
                V[0xF] = 1;

            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0 )>> 4] - V[(opcode & 0x0F00) >> 8];
            pc += 2;
            break;

        case 0x000E:
            V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] << 1;
            pc += 2;
            break;
        default:
            cout << "Unknown opcode in 8 series :  " << opcode << endl;
            break;
        }

        break;

    case 0x9000:
        // skip next instruction if vx != vy
        if ((V[(opcode & 0x0F00) >> 8]) != (V[(opcode & 0x00F0 )>> 4]))
        {
            pc += 4;
        }
        else
            pc += 2;

        break;

    case 0xA000:
        memory_address_register = opcode & 0x0FFF;
        pc += 2;
        break;

    case 0xB000:
        pc = (opcode & 0x0FFF) + V[0];
        break;
    case 0xC000:
        V[(opcode & 0x0F00) >> 8] = (rand() % (0xFF + 1)) & (opcode & 0x00FF);
        pc += 2;
        break;

    case 0xD000:
    {
        // enclose variable declaration in brackets in switch case to prevent usage in other cases
        unsigned short x = V[(opcode & 0x0F00) >> 8];
        unsigned short y = V[(opcode & 0x00F0 )>> 4];
        unsigned short h = opcode & 0x000F;
        unsigned short pixel;
        V[0xF] = 0;
        for (int Yi = 0; Yi < h; Yi++)
        {
            pixel = memory[memory_address_register + Yi];

            for (int Xi = 0; Xi < 8; Xi++)
            {
                // scan pixel l-> r to see where to draw
                if ((pixel & (0x80 >> Xi)) != 0)
                {
                    // Draw here
                    if (gfx_buffer[x + Xi + ((y + Yi) * 64)] == 1)
                    {
                        V[0xF] = 1; // collision
                    }
                    gfx_buffer[x + Xi + ((y + Yi) * 64)] ^= 1; // handle collision or not
                }
            }
        }

        drawFlag = true;
        pc += 2;
    }
    break;
    case 0xE000:

        switch (opcode & 0x00FF)
        {
            // if key at vx is pressed skip next opcode
        case 0x009E:
            if (keypad[V[(opcode & 0x0F00) >> 8]] != 0)
            {
                pc += 4;
            }
            else
                pc += 2;
            break;

        case 0x00A1:
            // if key at vx is not pressed skip next opcode
            if (keypad[V[(opcode & 0x0F00) >> 8]] == 0)
            {
                pc += 4;
            }
            else
                pc += 2;
            break;
        }
        break;
    case 0xF000:
        switch (opcode & 0x00FF)
        {
        case 0X0007:
            // set vx to delay timer

            V[(opcode & 0x0F00) >> 8] = time_delay;
            pc += 2;

            break;
        case 0x000A:
        {
            // await key press and store in vx
            bool keyflag = false;
            for (int i = 0; i < 16; ++i)
            {
                if (keypad[i] != 0)
                {
                    V[(opcode & 0x0F00) >> 8] = i;
                    keyflag = true;
                }
            }

            if (!keyflag)
                return; // exit and try again

            pc += 2; // nxt instruction
        }
        break;

        case 0x0015:
            // time delay = vx
            time_delay = V[(opcode & 0x0F00) >> 8];
            pc += 2;
            break;
        case 0x0018:
            sound_delay = V[(opcode & 0x0F00) >> 8];
            pc += 2;

            break;
        case 0x001E:

            if (memory_address_register + V[(opcode & 0x0F00) >> 8] > 0xFFF)
                V[0xF] = 1;
            else
                V[0xF] = 0;

            memory_address_register += V[(opcode & 0x0F00) >> 8];
            pc += 2;

            break;
        case 0x0029:
            memory_address_register = V[(opcode & 0x0F00) >> 8] * 0x5;

            pc += 2;
            break;
        case 0x0033:
            memory[memory_address_register] = V[(opcode & 0x0F00) >> 8] / 100;
            memory[memory_address_register + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
            memory[memory_address_register + 2] = (V[(opcode & 0x0F00) >> 8]) % 10;
            pc += 2;

            break;
        case 0x055:
            for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
            {
                memory[memory_address_register + i] = V[i];
            }

            // memory_address_register += ((opcode & 0x0F00) >> 8) + 1;
            pc += 2;
            break;
        case 0x0065:

            for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
            {
                V[i] = memory[memory_address_register + i];
            }
            // memory_address_register += ((opcode & 0x0F00) >> 8) + 1;
            pc += 2;

            break;
        default:
            cout << " Unknown opcode general: " << opcode << endl;
            break;
        }
        break;
    }

    // update timers
    if (time_delay > 0)
        --time_delay;

    if (sound_delay > 0)
        --sound_delay;
}