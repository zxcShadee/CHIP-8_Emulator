#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

/// @class Chip8
/// @brief Класс, эмулирующий работу виртуальной машины CHIP-8.
///
/// Управляет памятью, регистрами, стеком и графическим экраном эмулятора.
/// Все ошибки инициализации или выполнения генерируют исключения std::runtime_error.
class Chip8 {
public:
    /// @brief Конструктор по умолчанию. Инициализирует начальное состояние.
    ///
    /// Очищает память, загружает стандартные шрифты CHIP-8 в начало памяти
    /// и устанавливает программный счетчик на адрес 0x200.
    Chip8();

    /// @brief Загружает ROM-файл в память эмулятора.
    /// @param filePath Путь к файлу (рекомендуется использовать разделитель /).
    /// @throws std::runtime_error В случае, если файл не найден или недоступен для чтения.
    void loadRom(const std::string& filePath);

    /// @brief Выполняет один такт процессора (fetch, decode, execute).
    /// @throws std::runtime_error В случае обнаружения неизвестной или неподдерживаемой инструкции.
    void emulateCycle();

    /// @brief Очищает графический экран.
    void clearDisplay();

    /// @brief Массив памяти CHIP-8 (4096 байт).
    std::array<uint8_t, 4096> memory{};

    /// @brief Регистры общего назначения (V0 - VF).
    std::array<uint8_t, 16> V{};

    /// @brief Индексный регистр (I).
    uint16_t I{};

    /// @brief Программный счетчик (Program Counter).
    uint16_t pc{};

    /// @brief Стек вызовов для возврата из подпрограмм.
    std::vector<uint16_t> stack{};

    /// @brief Таймер задержки.
    uint8_t delayTimer{};

    /// @brief Звуковой таймер.
    uint8_t soundTimer{};

    /// @brief Состояние экрана (64x32 пикселей).
    std::array<uint8_t, 64 * 32> display{};
};
