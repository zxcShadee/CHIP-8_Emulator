#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "Chip8.hpp"
#include <fstream>

TEST_CASE("Chip8 Initialization") {
    Chip8 chip;
    CHECK(chip.pc == 0x200);
    CHECK(chip.I == 0);
    CHECK(chip.delayTimer == 0);
    
    // Проверка загрузки шрифта (первый байт нуля)
    CHECK(chip.memory[0x50] == 0xF0);
}

TEST_CASE("Chip8 loadRom Function") {
    Chip8 chip;
    
    SUBCASE("Negative: File does not exist") {
        CHECK_THROWS_AS(chip.loadRom("dummy/non_existent_file.ch8"), std::runtime_error);
    }

    SUBCASE("Positive: Loading valid dummy ROM") {
        // Создаем временный файл
        const std::string testPath = "test_rom.ch8";
        std::ofstream testFile(testPath, std::ios::binary);
        uint8_t dummyData[] = {0xA2, 0xF0, 0x12, 0x34}; // LD I, 0x2F0; JP 0x234
        testFile.write(reinterpret_cast<char*>(dummyData), 4);
        testFile.close();

        REQUIRE_NOTHROW(chip.loadRom(testPath));
        CHECK(chip.memory[0x200] == 0xA2);
        CHECK(chip.memory[0x201] == 0xF0);
        CHECK(chip.memory[0x202] == 0x12);
        CHECK(chip.memory[0x203] == 0x34);

        std::remove(testPath.c_str());
    }
}

TEST_CASE("Chip8 emulateCycle Function") {
    Chip8 chip;

    SUBCASE("Positive: Instruction 0x00E0 (Clear Screen)") {
        chip.display.fill(0xFF); // Заполняем экран мусором
        chip.memory[0x200] = 0x00;
        chip.memory[0x201] = 0xE0;
        
        REQUIRE_NOTHROW(chip.emulateCycle());
        
        // Экран должен быть очищен
        bool allClear = true;
        for (auto pixel : chip.display) {
            if (pixel != 0) allClear = false;
        }
        CHECK(allClear == true);
        CHECK(chip.pc == 0x202); // PC увеличился на 2
    }

    SUBCASE("Negative: Unknown Instruction") {
        chip.memory[0x200] = 0xFF; // Несуществующий опкод
        chip.memory[0x201] = 0xFF;
        
        CHECK_THROWS_AS(chip.emulateCycle(), std::runtime_error);
    }
}
