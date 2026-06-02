#include "Chip8.hpp"
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <random>

const std::array<uint8_t, 80> fontset = {
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

Chip8::Chip8() {
    pc = 0x200;
    I = 0;
    delayTimer = 0;
    soundTimer = 0;
    clearDisplay();
    keypad.fill(0);

    for (size_t i = 0; i < fontset.size(); ++i) {
        memory[0x50 + i] = fontset[i];
    }
}

void Chip8::loadRom(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("Ошибка: Не удалось открыть ROM файл по пути " + filePath);
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (size > (4096 - 512)) {
        throw std::runtime_error("Ошибка: ROM файл слишком велик для памяти CHIP-8.");
    }

    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        throw std::runtime_error("Ошибка: Не удалось прочитать данные из файла.");
    }

    for (size_t i = 0; i < buffer.size(); ++i) {
        memory[0x200 + i] = buffer[i];
    }
}

void Chip8::clearDisplay() {
    display.fill(0);
}

void Chip8::emulateCycle() {
    if (pc >= 4094) {
        throw std::runtime_error("Ошибка: Выход за пределы памяти (PC out of bounds).");
    }

    // Fetch (Выборка опкода)
    uint16_t opcode = (memory[pc] << 8) | memory[pc + 1];

    // Разбор компонентов инструкции
    uint16_t x = (opcode & 0x0F00) >> 8;
    uint16_t y = (opcode & 0x00F0) >> 4;
    uint16_t nnn = opcode & 0x0FFF;
    uint8_t kk = opcode & 0x00FF;
    uint8_t nibble = opcode & 0x000F;

    // Настройка генератора случайных чисел
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<uint8_t> dist(0, 255);

    switch (opcode & 0xF000) {
        case 0x0000:
            if (opcode == 0x00E0) {
                clearDisplay();
                pc += 2;
            } else if (opcode == 0x00EE) {
                if (stack.empty()) {
                    throw std::runtime_error("Ошибка: Стек пуст при вызове RET.");
                }
                pc = stack.back();
                stack.pop_back();
                pc += 2;
            } else {
                // Старые системные вызовы на некоторых ROM игнорируем
                pc += 2;
            }
            break;

        case 0x1000: // 1nnn - JP addr
            pc = nnn;
            break;

        case 0x2000: // 2nnn - CALL addr
            stack.push_back(pc);
            pc = nnn;
            break;

        case 0x3000: // 3xkk - SE Vx, byte
            pc += (V[x] == kk) ? 4 : 2;
            break;

        case 0x4000: // 4xkk - SNE Vx, byte
            pc += (V[x] != kk) ? 4 : 2;
            break;

        case 0x5000: // 5xy0 - SE Vx, Vy
            pc += (V[x] == V[y]) ? 4 : 2;
            break;

        case 0x6000: // 6xkk - LD Vx, byte
            V[x] = kk;
            pc += 2;
            break;

        case 0x7000: // 7xkk - ADD Vx, byte
            V[x] += kk;
            pc += 2;
            break;

        case 0x8000:
            switch (nibble) {
                case 0x0: V[x] = V[y]; break;  // 8xy0 - LD Vx, Vy
                case 0x1: V[x] |= V[y]; break; // 8xy1 - OR Vx, Vy
                case 0x2: V[x] &= V[y]; break; // 8xy2 - AND Vx, Vy
                case 0x3: V[x] ^= V[y]; break; // 8xy3 - XOR Vx, Vy
                case 0x4: {                    // 8xy4 - ADD Vx, Vy (Carry flag)
                    uint16_t sum = V[x] + V[y];
                    V[0xF] = (sum > 255) ? 1 : 0;
                    V[x] = sum & 0xFF;
                    break;
                }
                case 0x5:                      // 8xy5 - SUB Vx, Vy
                    V[0xF] = (V[x] > V[y]) ? 1 : 0;
                    V[x] -= V[y];
                    break;
                case 0x6:                      // 8xy6 - SHR Vx
                    V[0xF] = V[x] & 0x1;
                    V[x] >>= 1;
                    break;
                case 0x7:                      // 8xy7 - SUBN Vx, Vy
                    V[0xF] = (V[y] > V[x]) ? 1 : 0;
                    V[x] = V[y] - V[x];
                    break;
                case 0xE:                      // 8xyE - SHL Vx
                    V[0xF] = (V[x] & 0x80) >> 7;
                    V[x] <<= 1;
                    break;
                default:
                    goto unknown_opcode;
            }
            pc += 2;
            break;

        case 0x9000: // 9xy0 - SNE Vx, Vy
            pc += (V[x] != V[y]) ? 4 : 2;
            break;

        case 0xA000: // Annn - LD I, addr
            I = nnn;
            pc += 2;
            break;

        case 0xB000: // Bnnn - JP V0, addr
            pc = nnn + V[0];
            break;

        case 0xC000: // Cxkk - RND Vx, byte
            V[x] = dist(gen) & kk;
            pc += 2;
            break;

        case 0xD000: { // Dxyn - DRW Vx, Vy, nibble
            uint8_t xCoord = V[x] % 64;
            uint8_t yCoord = V[y] % 32;
            V[0xF] = 0;

            for (unsigned int row = 0; row < nibble; row++) {
                uint8_t spriteByte = memory[I + row];
                for (unsigned int col = 0; col < 8; col++) {
                    uint8_t spritePixel = spriteByte & (0x80 >> col);
                    if (spritePixel) {
                        // Клиппинг границ экрана для предотвращения выхода за границы массива
                        if (xCoord + col < 64 && yCoord + row < 32) {
                            uint32_t index = (xCoord + col) + (yCoord + row) * 64;
                            if (display[index] == 1) {
                                V[0xF] = 1; // Коллизия обнаружена
                            }
                            display[index] ^= 1;
                        }
                    }
                }
            }
            pc += 2;
            break;
        }

        case 0xE000:
            if (kk == 0x9E) {      // Ex9E - SKP Vx
                pc += (keypad[V[x]]) ? 4 : 2;
            } else if (kk == 0xA1) { // ExA1 - SKNP Vx
                pc += (!keypad[V[x]]) ? 4 : 2;
            } else {
                goto unknown_opcode;
            }
            break;

        case 0xF000:
            switch (kk) {
                case 0x07: V[x] = delayTimer; break; // Fx07 - LD Vx, DT
                case 0x15: delayTimer = V[x]; break; // Fx15 - LD DT, Vx
                case 0x18: soundTimer = V[x]; break; // Fx18 - LD ST, Vx
                case 0x1E: I += V[x]; break;         // Fx1E - ADD I, Vx
                
                case 0x0A: {                         // Fx0A - LD Vx, K (Ожидание кнопки)
                    bool keyPressed = false;
                    for (int i = 0; i < 16; ++i) {
                        if (keypad[i]) {
                            V[x] = i;
                            keyPressed = true;
                            break;
                        }
                    }
                    if (!keyPressed) return; // Блокируем шаг PC, пока кнопка не нажата
                    break;
                }
                case 0x29: // Fx29 - LD F, Vx (Установка I на адрес спрайта символа)
                    I = 0x50 + (V[x] * 5);
                    break;

                case 0x33: // Fx33 - LD B, Vx (BCD представление числа)
                    memory[I]     = V[x] / 100;
                    memory[I + 1] = (V[x] / 10) % 10;
                    memory[I + 2] = V[x] % 10;
                    break;

                case 0x55: // Fx55 - LD [I], Vx (Сохранение регистров в память)
                    for (uint16_t i = 0; i <= x; ++i) {
                        memory[I + i] = V[i];
                    }
                    break;

                case 0x65: // Fx65 - LD Vx, [I] (Чтение регистров из памяти)
                    for (uint16_t i = 0; i <= x; ++i) {
                        V[i] = memory[I + i];
                    }
                    break;

                default:
                    goto unknown_opcode;
            }
            pc += 2;
            break;

        unknown_opcode:
        default: {
            std::stringstream ss;
            ss << "Неизвестная или нереализованная инструкция: 0x" 
               << std::hex << std::uppercase << opcode;
            throw std::runtime_error(ss.str());
        }
    }

    if (delayTimer > 0) --delayTimer;
    if (soundTimer > 0) --soundTimer;
}
