#include "Milly-Murr_V2.h"
#include <iostream>
#include <chrono>

int main(int argc, char *argv[]) {
    if (argc != 5) {
        std::cerr << "Must be program.exe <mealy-to-moore|moore-to-mealy> "
                     "<input_file> <output_file>"
                  << std::endl;
        return 1;
    }

    std::string command = argv[2];
    std::string inputFile = argv[3];
    std::string outputFile = argv[4];
    if (command == "mealy-to-moore") {
        try {
            auto TC1 = std::chrono::high_resolution_clock::now();
            Table mealyTable = ReadMealyToTable(inputFile);
            auto TC2 = std::chrono::high_resolution_clock::now();
            std::cout << "Read " << std::chrono::duration<double>(TC2 - TC1).count() << std::endl;
            MealyVer *startVer = TableToMealyGraph(mealyTable);
            auto TC3 = std::chrono::high_resolution_clock::now();
            std::cout << "To graph " << std::chrono::duration<double>(TC3 - TC2).count() << std::endl;
            auto startMooreVer = MealyToMoore(startVer);
            auto TC4 = std::chrono::high_resolution_clock::now();
            std::cout << "Mealy to Moore " << std::chrono::duration<double>(TC4 - TC3).count() << std::endl;
            Table mooreTable = MooreGraphToTable(startMooreVer);
            auto TC5 = std::chrono::high_resolution_clock::now();
            std::cout << "To table " << std::chrono::duration<double>(TC5 - TC4).count() << std::endl;
            WriteMooreFromTable(outputFile, mooreTable);
            auto TC6 = std::chrono::high_resolution_clock::now();
            std::cout << "Write " << std::chrono::duration<double>(TC6 - TC5).count() << std::endl;
        } catch (std::exception &e) {
            std::cout << e.what();
        }
    } else if (command == "moore-to-mealy") {
        try {
            std::cout << "Read moore" << std::endl;
            Table mooreTable = ReadMooreToTable(inputFile);
            MooreVer *startVer = TableToMooreGraph(mooreTable);
            MealyVer *startMealyVer = MooreToMealy(startVer);
            Table mealyTable = MealyGraphToTable(startMealyVer);
            WriteMealyFromTable(outputFile, mealyTable);
        } catch (std::exception &e) {
            std::cout << e.what();
        }
    }
    return 0;
}
