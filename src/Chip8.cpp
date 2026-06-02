#include "Chip8.hpp"
#include <fstream>
#include <stdexcept>
#include <iostream>

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
    pc = 0x200; // Программы начинаются с адреса 0x200
    I = 0;
    delayTimer = 0;
    soundTimer = 0;

    // Загрузка шрифтов в память (адреса 0x050 - 0x09F)
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

    // Выборка инструкции (Fetch)
    uint16_t opcode = (memory[pc] << 8) | memory[pc + 1];

    // Декодирование и Выполнение (Decode and Execute)
    uint16_t x = (opcode & 0x0F00) >> 8;
    uint16_t y = (opcode & 0x00F0) >> 4;
    uint16_t nnn = opcode & 0x0FFF;
    uint8_t kk = opcode & 0x00FF;

    switch (opcode & 0xF000) {
        case 0x0000:
            if (opcode == 0x00E0) {
                clearDisplay();
                pc += 2;
            } else if (opcode == 0x00EE) {
                if (stack.empty()) {
                    throw std::runtime_error("Ошибка: Попытка возврата из пустой подпрограммы.");
                }
                pc = stack.back();
                stack.pop_back();
                pc += 2;
            } else {
                throw std::runtime_error("Неизвестный опкод: " + std::to_string(opcode));
            }
            break;
        case 0x1000: // 1nnn: JP addr
            pc = nnn;
            break;
        case 0x2000: // 2nnn: CALL addr
            stack.push_back(pc);
            pc = nnn;
            break;
        case 0x3000: // 3xkk: SE Vx, byte
            pc += (V[x] == kk) ? 4 : 2;
            break;
        case 0x4000: // 4xkk: SNE Vx, byte
            pc += (V[x] != kk) ? 4 : 2;
            break;
        case 0x5000: // 5xy0: SE Vx, Vy
            pc += (V[x] == V[y]) ? 4 : 2;
            break;
        case 0x6000: // 6xkk: LD Vx, byte
            V[x] = kk;
            pc += 2;
            break;
        case 0x7000: // 7xkk: ADD Vx, byte
            V[x] += kk;
            pc += 2;
            break;
        case 0xA000: // Annn: LD I, addr
            I = nnn;
            pc += 2;
            break;
        default:
            throw std::runtime_error("Неизвестная или нереализованная инструкция: 0x" + std::to_string(opcode));
    }

    // Обновление таймеров
    if (delayTimer > 0) --delayTimer;
    if (soundTimer > 0) --soundTimer;
}
