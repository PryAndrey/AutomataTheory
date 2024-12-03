#include <iostream>
#include "DetermineNFA.h"

int main(int argc, char *argv[]) {
    if (argc != 3 && argc != 4) {
        std::cerr << "Must be program.exe <input_file> <output_file>"
                  << std::endl;
        return 1;
    }

    std::string inputFile;
    std::string outputFile;

    if (argc == 4) {
        inputFile = argv[2];
        outputFile = argv[3];
    }

    if (argc == 3) {
        inputFile = argv[1];
        outputFile = argv[2];
    }
#ifdef _WIN32
    system("chcp 65001");
#endif
    try {
        DetermineNFA dnfa;
        dnfa.ReadFromCSVFile(inputFile);
        dnfa.ToDFA();
        dnfa.WriteToCSVFile(outputFile);
    } catch (std::exception &e) {
        std::cout << e.what();
    }

    return 0;
}
