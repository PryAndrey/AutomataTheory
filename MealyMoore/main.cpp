#include "MMV3/MealyGraph.h"
#include "MMV3/MooreGraph.h"
#include <iostream>
#include <chrono>

int main(int argc, char *argv[]) {
    if (argc != 5 && argc != 4) {
        std::cerr << "Must be program.exe <mealy-to-moore|moore-to-mealy> "
                     "<input_file> <output_file>"
                  << std::endl;
        return 1;
    }

    std::string command;
    std::string inputFile;
    std::string outputFile;

    if (argc == 4) {
        command = argv[1];
        inputFile = argv[2];
        outputFile = argv[3];
    }

    if (argc == 5) {
        command = argv[2];
        inputFile = argv[3];
        outputFile = argv[4];
    }

    if (command == "mealy-to-moore") {
        try {
            MealyGraph mealyGraph;
            auto TP1 = std::chrono::high_resolution_clock::now();

            mealyGraph.FillGraphFromCSVFile(inputFile);
            auto TP2 = std::chrono::high_resolution_clock::now();
            std::cout << "Read: " << std::chrono::duration<double>(TP2 - TP1).count() << std::endl;

            MooreGraph mooreGraph = mealyGraph.ToMooreGraph();
            auto TP3 = std::chrono::high_resolution_clock::now();
            std::cout << "Convert: " << std::chrono::duration<double>(TP3 - TP2).count() << std::endl;

            mealyGraph.WriteToCSVFile(outputFile);
            auto TP4 = std::chrono::high_resolution_clock::now();
            std::cout << "Write: " << std::chrono::duration<double>(TP4 - TP3).count() << std::endl;
        } catch (std::exception &e) {
            std::cout << e.what();
        }
    } else if (command == "moore-to-mealy") {
        try {
            MooreGraph mooreGraph;
            auto TP1 = std::chrono::high_resolution_clock::now();

            mooreGraph.FillGraphFromCSVFile(inputFile);
            auto TP2 = std::chrono::high_resolution_clock::now();
            std::cout << "Read: " << std::chrono::duration<double>(TP2 - TP1).count() << std::endl;

            MealyGraph mealyGraph = mooreGraph.ToMealyGraph();
            auto TP3 = std::chrono::high_resolution_clock::now();
            std::cout << "Convert: " << std::chrono::duration<double>(TP3 - TP2).count() << std::endl;

            mealyGraph.WriteToCSVFile(outputFile);
            auto TP4 = std::chrono::high_resolution_clock::now();
            std::cout << "Write: " << std::chrono::duration<double>(TP4 - TP3).count() << std::endl;
        } catch (std::exception &e) {
            std::cout << e.what();
        }
    }
    return 0;
}
