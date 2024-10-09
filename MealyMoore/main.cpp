#include "Milly-Murr_V2.cpp"
#include <iostream>

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
            Table mealyTable = ReadMealyToTable(inputFile);
            cout << "Read mealy" << endl;
//            WriteTableCout(mealyTable, 6);

            MealyVer* startVer = TableToMealyGraph(mealyTable);

            MooreVer* startMooreVer = MealyToMoore(startVer);

            Table mooreTable = MooreGraphToTable(startMooreVer);

//            WriteTableCout(mooreTable);
            WriteMooreFromTable(outputFile, mooreTable);

        } catch (exception &e) {
            cout << e.what();
        }
    } else if (command == "moore-to-mealy") {
        try {
            cout << "Read moore" << endl;
            Table mooreTable = ReadMooreToTable(inputFile);
            WriteTableCout(mooreTable, 3);

            MooreVer* startVer = TableToMooreGraph(mooreTable);

            MealyVer* startMealyVer = MooreToMealy(startVer);
            Table mealyTable = MealyGraphToTable(startMealyVer);
            WriteTableCout(mealyTable);
            WriteMealyFromTable(outputFile, mealyTable);

        } catch (exception &e) {
            cout << e.what();
        }
    }
    return 0;
}
