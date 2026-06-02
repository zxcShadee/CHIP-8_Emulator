#include "Chip8.hpp"
#include <iostream>
#include <exception>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Использование: " << argv[0] << " <путь/к/rom/файлу>\n";
        return 1;
    }

    try {
        Chip8 emulator;
        std::string romPath = argv[1];
        
        // Заменяем обратные слеши на прямые, если пользователь ввел их по ошибке
        for (char& c : romPath) {
            if (c == '\\') c = '/';
        }

        std::cout << "Попытка загрузки ROM: " << romPath << "\n";
        emulator.loadRom(romPath);
        std::cout << "ROM успешно загружен. Запуск эмуляции...\n";

        // Простой цикл для демонстрации. В реальном приложении здесь будет SDL2/Qt цикл
        for (int i = 0; i < 10; ++i) {
            emulator.emulateCycle();
        }
        
        std::cout << "Эмуляция завершена успешно.\n";

    } catch (const std::exception& e) {
        std::cerr << "Критическая ошибка: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
