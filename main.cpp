#include "game.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "Welcome to Space Invaders!\n";
    
    std::cout << "\nSelect difficulty:\n";
    std::cout << "1. Easy (5 Lives, Slower)\n";
    std::cout << "2. Normal (3 Lives, Regular Speed)\n";
    std::cout << "3. Hard (2 Lives, Faster)\n";
    std::cout << "Enter number (1-3): ";
    
    int choice;
    while (!(std::cin >> choice) || choice < 1 || choice > 3) {
        std::cout << "Invalid input. Enter number (1-3): ";
        std::cin.clear();
        std::cin.ignore(10000, '\n');
    }
    
    Difficulty selectedDifficulty;
    if (choice == 1) {
        selectedDifficulty = Difficulty::EASY;
    } else if (choice == 3) {
        selectedDifficulty = Difficulty::HARD;
    } else {
        selectedDifficulty = Difficulty::NORMAL;
    }
    
    std::cout << "Starting game. Switching to ncurses mode...\n";
    std::this_thread::sleep_for(std::chrono::seconds(1)); 

    Game game(selectedDifficulty); 
    game.run();

    return 0;
}