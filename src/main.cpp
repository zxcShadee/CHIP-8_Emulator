#include "Chip8.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <exception>
#include <cmath>
#include <vector>

/// @brief Считывает текущее состояние клавиатуры ПК и обновляет состояние keypad в эмуляторе.
/// @param emulator Ссылка на объект эмулятора CHIP-8.
void updateKeypad(Chip8& emulator) {
    emulator.keypad[0x1] = sf::Keyboard::isKeyPressed(sf::Keyboard::Num1);
    emulator.keypad[0x2] = sf::Keyboard::isKeyPressed(sf::Keyboard::Num2);
    emulator.keypad[0x3] = sf::Keyboard::isKeyPressed(sf::Keyboard::Num3);
    emulator.keypad[0xC] = sf::Keyboard::isKeyPressed(sf::Keyboard::Num4);

    emulator.keypad[0x4] = sf::Keyboard::isKeyPressed(sf::Keyboard::Q);
    emulator.keypad[0x5] = sf::Keyboard::isKeyPressed(sf::Keyboard::W);
    emulator.keypad[0x6] = sf::Keyboard::isKeyPressed(sf::Keyboard::E);
    emulator.keypad[0xD] = sf::Keyboard::isKeyPressed(sf::Keyboard::R);

    emulator.keypad[0x7] = sf::Keyboard::isKeyPressed(sf::Keyboard::A);
    emulator.keypad[0x8] = sf::Keyboard::isKeyPressed(sf::Keyboard::S);
    emulator.keypad[0x9] = sf::Keyboard::isKeyPressed(sf::Keyboard::D);
    emulator.keypad[0xE] = sf::Keyboard::isKeyPressed(sf::Keyboard::F);

    emulator.keypad[0xA] = sf::Keyboard::isKeyPressed(sf::Keyboard::Z);
    emulator.keypad[0x0] = sf::Keyboard::isKeyPressed(sf::Keyboard::X);
    emulator.keypad[0xB] = sf::Keyboard::isKeyPressed(sf::Keyboard::C);
    emulator.keypad[0xF] = sf::Keyboard::isKeyPressed(sf::Keyboard::V);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Использование: " << argv[0] << " <путь/к/rom/файлу>\n";
        return 1;
    }

    try {
        Chip8 emulator;
        std::string romPath = argv[1];
        
        for (char& c : romPath) {
            if (c == '\\') c = '/';
        }

        emulator.loadRom(romPath);

        sf::RenderWindow window(sf::VideoMode(640, 320), "CHIP-8 Emulator");
        window.setFramerateLimit(60);

        const unsigned sampleRate = 44100;
        std::vector<sf::Int16> samples(sampleRate);
        for (size_t i = 0; i < samples.size(); ++i) {
            double time = static_cast<double>(i) / sampleRate;
            samples[i] = static_cast<sf::Int16>(20000 * std::sin(2 * 3.1415926535 * 440.0 * time));
        }

        sf::SoundBuffer soundBuffer;
        if (!soundBuffer.loadFromSamples(samples.data(), samples.size(), 1, sampleRate)) {
            throw std::runtime_error("Ошибка инициализации звукового движка SFML.");
        }

        sf::Sound beep;
        beep.setBuffer(soundBuffer);
        beep.setLoop(true);

        sf::RectangleShape pixel(sf::Vector2f(10.0f, 10.0f));
        pixel.setFillColor(sf::Color::White);

        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    window.close();
                }
            }

            updateKeypad(emulator);

            for (int i = 0; i < 10; ++i) {
                emulator.emulateCycle();
            }

            if (emulator.soundTimer > 0) {
                if (beep.getStatus() != sf::Sound::Playing) {
                    beep.play();
                }
            } else {
                beep.stop();
            }

            window.clear(sf::Color::Black);

            for (int y = 0; y < 32; ++y) {
                for (int x = 0; x < 64; ++x) {
                    if (emulator.display[x + y * 64]) {
                        pixel.setPosition(x * 10.0f, y * 10.0f);
                        window.draw(pixel);
                    }
                }
            }

            window.display();
        }

    } catch (const std::exception& e) {
        std::cerr << "Критическая ошибка: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
