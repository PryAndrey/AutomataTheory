#pragma onse

#include <utility>
#include <vector>
#include <unordered_map>
#include <map>
#include <set>
#include <fstream>
#include <regex>
#include <iostream>
#include <unordered_set>
#include <queue>

class MooreTransition {
public:
    MooreTransition(int from, int to, std::string inSymbol)
            : m_from(from), m_to(to), m_inSymbol(std::move(inSymbol)) {}

    int m_from;
    int m_to;
    std::string m_inSymbol;
};

struct MooreState {
    MooreState(std::string nState, std::string nOutSymbol, std::set<int> nTransitions = {})
            : state(std::move(nState)),
              outSymbol(std::move(
                      nOutSymbol)),
              transitions(std::move(
                      nTransitions)) {}

    MooreState() = default;

    std::string state;
    std::string outSymbol;
    std::set<int> transitions;
};

class Token {
public:
    Token(std::string type = "", int line = -1, int column = -1, std::string value = "")
            : m_type(std::move(type)), m_line(line), m_column(column), m_value(std::move(value)) {}

    int m_line;
    int m_column;
    std::string m_type;
    std::string m_value;
};

class Scanner {
public:
    std::vector<std::string> m_inSymbols;
    std::vector<MooreState> m_states;
    std::map<std::string, std::pair<std::string, int>> m_statistic;
    std::unordered_map<std::string, int> m_statesMap;
    std::map<std::string, std::string> m_typeMap = {
            {"*",         "MULTIPLICATION"},
            {"+",         "PLUS"},
            {"-",         "MINUS"},
            {"/",         "DIVIDE"},
            {";",         "SEMICOLON"},
            {",",         "COMMA"},
            {"(",         "LEFT_PAREN"},
            {")",         "RIGHT_PAREN"},
            {"[",         "LEFT_BRACKET"},
            {"]",         "RIGHT_BRACKET"},
            {"=",         "EQ"},
            {">",         "GREATER"},
            {"<",         "LESS"},
            {"<=",        "LESS_EQ"},
            {">=",        "GREATER_EQ"},
            {"<>",        "NOT_EQ"},
            {":",         "COLON"},
            {":=",        "ASSIGN"},
            {".",         "DOT"},
            {"ARRAY",     "ARRAY"},
            {"BEGIN",     "BEGIN"},
            {"ELSE",      "ELSE"},
            {"END",       "END"},
            {"IF",        "IF"},
            {"OF",        "OF"},
            {"OR",        "OR"},
            {"PROGRAM",   "PROGRAM"},
            {"PROCEDURE", "PROCEDURE"},
            {"THEN",      "THEN"},
            {"TYPE",      "TYPE"},
            {"VAR",       "VAR"},
    };
    std::vector<MooreTransition> m_transitions;
    int m_currLineCount = 1;
    int m_currColumnCount = 1;

    void FillRulesFromCSVFile(const std::string &fileName);

    void ScanFile(const std::string &fileName, const std::string &outFilename);

    void AddToStatistic(const Token &token);

    Token FindToken(std::ifstream &file);

    static void WriteTokenToFile(std::ofstream &file, const Token &token);
};
