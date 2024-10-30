#include "MealyGraph.h"
#include "MooreGraph.h"
#include <iostream>

int main(int argc, char *argv[]) {
    if (argc != 5 && argc != 4) {
        std::cerr << "Must be program.exe <mealy|moore> "
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

    std::cout << command << inputFile << outputFile << std::endl;

    try {
        if (command == "mealy") {
            MealyGraph mealyGraph;
            mealyGraph.FillGraphFromCSVFile(inputFile);
            mealyGraph.Minimize();
            mealyGraph.WriteToCSVFile(outputFile);
        } else if (command == "moore") {
            MooreGraph mooreGraph;
            mooreGraph.FillGraphFromCSVFile(inputFile);
            mooreGraph.Minimize();
            mooreGraph.WriteToCSVFile(outputFile);
        }
    } catch (std::exception &e) {
        std::cout << e.what();
    }
    return 0;
}
