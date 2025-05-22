#include <iostream>
#include "RegexToNFA.h"
#include <codecvt>

int main(int argc, char *argv[]) {
    if (argc != 3 && argc != 4) {
        std::cerr << "Must be program.exe <output_file> regular_expression"
                  << std::endl;
        return 1;
    }
    std::setlocale(LC_ALL, "Russian");
    std::string outputFile;
    std::string regularExpression;

    if (argc == 4) {
        outputFile = argv[2];
        regularExpression = argv[3];
    }

    if (argc == 3) {
        outputFile = argv[1];
        regularExpression = argv[2];
    }

    regularExpression = "(ab*a|b)+|abb(ab)*";
    regularExpression = "((a*c+b*)|(b*a+c*)|(c*b+a*))+";
    regularExpression = "ab*((a|b*)df(b|a*))+";
//    regularExpression = "ab*((a|b*)df(b|a*))+";
//    regularExpression = "(ARRAY|BEGIN|ELSE|END|IF|OF|OR|PROGRAM|PROCEDURE|THEN|TYPE|VAR)";
    regularExpression = R"(ARRAY|REPEAT|UNTIL|INTEGER|BEGIN|ELSE|END|IF|OF|OR|PROGRAM|PROCEDURE|THEN|TYPE|VAR|\*|\+|-|/|;|,|\(|\)|[|]|=|>|<|<=|>=|<>|:|:=|.|//|(0|1|2|3|4|5|6|7|8|9)+|(0|1|2|3|4|5|6|7|8|9)+.(0|1|2|3|4|5|6|7|8|9)+|(0|1|2|3|4|5|6|7|8|9)+E\+(0|1|2|3|4|5|6|7|8|9)+|(0|1|2|3|4|5|6|7|8|9)+E-(0|1|2|3|4|5|6|7|8|9)+|(0|1|2|3|4|5|6|7|8|9)+E(0|1|2|3|4|5|6|7|8|9)+|(0|1|2|3|4|5|6|7|8|9)+.(0|1|2|3|4|5|6|7|8|9)+E\+(0|1|2|3|4|5|6|7|8|9)+|(0|1|2|3|4|5|6|7|8|9)+.(0|1|2|3|4|5|6|7|8|9)+E-(0|1|2|3|4|5|6|7|8|9)+|(0|1|2|3|4|5|6|7|8|9)+.(0|1|2|3|4|5|6|7|8|9)+E(0|1|2|3|4|5|6|7|8|9)+)";
//

#ifdef _WIN32
    system("chcp 65001");
#endif
    try {
        RegexToNFA rtNFA(regularExpression);
        rtNFA.WriteToCSVFile(outputFile);
    } catch (std::exception &e) {
        std::cout << e.what();
    }

    return 0;
}
