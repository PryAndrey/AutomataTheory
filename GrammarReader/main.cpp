#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include "GrammarReader.h"

int main(int argc, char *argv[]) {
    if (argc != 3 && argc != 4) {
        std::cerr << "Must be program.exe <input_file> <output_file>"
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

    if (argc == 3) {
        command = argv[0];
        inputFile = argv[1];
        outputFile = argv[2];
    }
#ifdef _WIN32
    system("chcp 65001");
#endif
    try {
        GrammarReader gr;
        gr.ReadFile(inputFile);
        gr.WriteMooreToFile(outputFile);
    } catch (std::exception &e) {
        std::cout << e.what();
    }

    return 0;
}
