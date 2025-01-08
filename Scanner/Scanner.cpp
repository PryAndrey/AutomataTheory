#pragma onse

#include <set>
#include "Scanner.h"

bool isSign(char ch) {
    return (ch == '!' || ch == '#' || ch == '$' || ch == '%' ||
            ch == '&' || ch == '\'' || ch == '(' || ch == ')' ||
            ch == '*' || ch == '+' || ch == ',' || ch == '-' ||
            ch == '.' || ch == '/' || ch == ':' || ch == '<' ||
            ch == '=' || ch == '>' || ch == '?' || ch == '@' ||
            ch == '[' || ch == ']' || ch == '^' || ch == '{' ||
            ch == '}' || ch == '\n' || ch == '\t' || ch == ' ');
}

std::string toUpperCase(const std::string &str) {
    std::string result = str;
    for (char &ch: result) {
        ch = std::toupper(static_cast<unsigned char>(ch));
    }
    return result;
}

void Scanner::FillRulesFromCSVFile(const std::string &fileName) {
    std::ifstream file(fileName);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + fileName);
    }

    std::string line;
    // Чтение 2 первых строк (состояния)
    if (std::getline(file, line)) {
        std::string statesString, outSymbolsString = line;
        if (std::getline(file, line)) {
            statesString = line;
            std::stringstream ssOut(outSymbolsString), ssStates(statesString);
            std::string outSymbol, state;
            std::getline(ssOut, outSymbol, ';');
            std::getline(ssStates, state, ';');
            while (std::getline(ssStates, state, ';')) {
                std::getline(ssOut, outSymbol, ';');
                if (!state.empty() && state != "\"\"" && outSymbol != "\"\"") {
                    m_states.emplace_back(state, outSymbol, std::set<int>());
                    m_statesMap[state] = m_states.size() - 1;
                } else {
                    throw std::invalid_argument("Wrong states format");
                }
            }
        }
    }

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string inSymbol;
        if (std::getline(ss, inSymbol, ';')) {
            std::string transition;
            if (ss.peek() == '\'') {
                m_inSymbols.push_back(";");
                inSymbol = ";";
                std::getline(ss, transition, ';');
            } else {
                m_inSymbols.push_back(inSymbol);
            }
            int index = 0;
            while (std::getline(ss, transition, ';')) {
                if (transition != "\"\"" && !transition.empty()) {
                    m_transitions.emplace_back(index, m_statesMap[transition], inSymbol);
                    m_states[index].transitions.insert(m_transitions.size() - 1);
                }
                index++;
            }
        }
    }

    file.close();
}

Token Scanner::FindToken(std::ifstream &file) {
    char ch;
    std::string line;
    int lineCount = m_currLineCount;
    int columnCount = m_currColumnCount;
    bool bad = false;

// todo Подумать над аргументом

    bool comment = false;       // Начинается с //
    bool blockComment = false;  // Начинается с {
    bool string = false;        // Начинается с '
    bool canBeIdentifier = false;    // Начинается с букв или _
    bool identifier = false;    // Начинается с букв или _
    MooreState currentState = m_states[0];

    while (file.get(ch)) {
        if (bad) {
            for (auto transitionInd: currentState.transitions) {
                if (ch == '\n' ||
                    toUpperCase(m_transitions[transitionInd].m_inSymbol) == toUpperCase(std::string(1, ch))) {
                    file.unget();
                    return {"BAD", lineCount, columnCount, line};
                }
            }
            m_currColumnCount++;
            line += ch;

            continue;
        }
        if (string) {
            if (ch != '\'') {
                line += ch;
                m_currColumnCount++;
            } else {
                line += ch;
                return {"STRING", lineCount, columnCount, line};
            }
            continue;
        }
        if (blockComment) {
            if (ch != '}') {
                line += ch;
                if (ch == '\n') {
                    m_currLineCount++;
                    m_currColumnCount = 1;
                } else {
                    m_currColumnCount++;
                }
            } else {
                return {"BLOCK_COMMENT", lineCount, columnCount, line};
            }
            continue;
        }
        if (comment) {
            if (ch != '\n') {
                line += ch;
                m_currColumnCount++;
            } else {
                file.unget();
                return {"LINE_COMMENT", lineCount, columnCount, line};
            }
            continue;
        }
        if (identifier) {
            if (std::isalpha(ch) || ch == '_' || isdigit(ch)) {
                line += ch;
                m_currColumnCount++;
            } else {
                file.unget();
                return {"IDENTIFIER", lineCount, columnCount, line};
            }
            continue;
        }
        if (line.empty()) {
            if (ch == ' ' || ch == '\t') {
                m_currColumnCount++;
                continue;
            }
            if (std::isalpha(ch) || ch == '_') {
                canBeIdentifier = true;
            }
            columnCount = m_currColumnCount;
            lineCount = m_currLineCount;
        }
        bool find = false;
        for (auto transitionInd: currentState.transitions) {
            if (toUpperCase(m_transitions[transitionInd].m_inSymbol) == toUpperCase(std::string(1, ch))) {
                if (!line.empty() && std::isdigit(line[line.size() - 1]) && ch == '.' && !std::isdigit(file.peek())) {
                    // На случай фигни после . в цифрах
                    break;
                }
                find = true;
                line += ch;
                currentState = m_states[m_transitions[transitionInd].m_to];
                break;
            }
        }
        if (isSign(ch)) {
            canBeIdentifier = false;
        }
        // Перешли в следующее состояние
        if (find) {
            m_currColumnCount++;
            if (ch == '\n') {
                m_currLineCount++;
                m_currColumnCount = 1;
            }
            continue;
        }

        // Не нашли переход

        // Это конечное состояние и дальше нет элементов идентификатора
        if (currentState.outSymbol == "F" && !(canBeIdentifier && (std::isalpha(ch) || std::isdigit(ch)))) {
            if (line == "//") {
                comment = true;
                line += ch;
                continue;
            }
            auto it = m_typeMap.find(toUpperCase(line));
            file.unget();
            if (it != m_typeMap.end()) {
                // Зарезервированное слово
                return {it->second, lineCount, columnCount, line};
            } else {
                // Число
                return {
                        line.find('.') != std::string::npos ? "REAL" : "INTEGER",
                        lineCount,
                        columnCount,
                        line};
            }
        } else {
            if (ch == '\n') {
                m_currLineCount++;
                m_currColumnCount = 1;
                continue;
            }
            if (ch == '{') {
                blockComment = true;
                continue;
            }
            if (ch == '\'') {
                string = true;
                line += ch;
                continue;
            }
            if (canBeIdentifier) {
                // Идентификатор
                identifier = true;
                canBeIdentifier = false;
                line += ch;
                m_currColumnCount++;
                if (ch == '\n') {
                    m_currLineCount++;
                    m_currColumnCount = 1;
                }
            } else {
                // ошибка
                line += ch;
                currentState = m_states[0];
                identifier = false;
                m_currColumnCount++;
                bad = true;
            }
        }
    }

    // Обработка последнего токена
    if (currentState.outSymbol == "F" && !comment) {
        if (line == "//") {
            return {"LINE_COMMENT", lineCount, columnCount, line};
        }
        auto it = m_typeMap.find(toUpperCase(line));
        if (it != m_typeMap.end()) {
            // Зарезервированное слово
            return {it->second, lineCount, columnCount, line};
        } else {
            // Число
            return {
                    line.find('.') != std::string::npos ? "REAL" : "INTEGER",
                    lineCount,
                    columnCount,
                    line};
        }
    } else {
        if (!line.empty()) {
            if (comment) {
                return {"LINE_COMMENT", lineCount, columnCount, line};
            }
            if (string || blockComment || bad) {
                return {"BAD", lineCount, columnCount, line};
            }
            if (canBeIdentifier || identifier) {
                return {"IDENTIFIER", lineCount, columnCount, line};
            }
        }
    }
    return {"EOF", lineCount, columnCount, ""};
}

void Scanner::ScanFile(const std::string &fileName, const std::string &outFilename) {
    std::ifstream file(fileName);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + fileName);
    }

    std::ofstream outFile(outFilename);
    if (!outFile.is_open()) {
        throw std::runtime_error("Could not open file: " + outFilename);
    }

    while (true) {
        const auto token = FindToken(file);
        if (token.m_type == "EOF") {
            break;
        }
        WriteTokenToFile(outFile, token);
    }
    file.close();
}

void Scanner::WriteTokenToFile(std::ofstream &file, const Token &token) {
    file << token.m_type << " (" << token.m_line << ", " << token.m_column << ") \"" << token.m_value << "\""
         << std::endl;
}

