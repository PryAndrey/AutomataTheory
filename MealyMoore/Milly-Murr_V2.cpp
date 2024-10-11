#pragma once
#include "Milly-Murr_V2.h"

std::pair<std::string, std::string> SplitMealyState(const std::string &input) {
    std::stringstream ss(input);

    std::string item1, item2;
    std::getline(ss, item1, '/');
    std::getline(ss, item2, '/');
    return {item1, item2};
}

Table ReadMealyToTable(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::invalid_argument("Can`t open file" + filename);
    }
    Table mealyTable;
    std::string line;

    mealyTable.push_back({"\\ "});

    // Чтение первой строки (состояния)
    if (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string state;
        std::getline(ss, state, ';');
        while (std::getline(ss, state, ';')) {
            if (!state.empty() && state != "\"\"") {
                mealyTable[0].push_back(state);
            } else {
                throw std::invalid_argument("Wrong states format");
            }
        }
    }

    while (std::getline(file, line)) {
        mealyTable.emplace_back();
        auto lastRow = --mealyTable.end();
        std::stringstream ss(line);
        std::string input;
        if (std::getline(ss, input, ';')) {
            lastRow->push_back(input);
            std::string transition;
            while (std::getline(ss, transition, ';')) {
                if (transition == "\"\"" || transition.empty()) {
                    lastRow->push_back("");
                } else {
                    lastRow->push_back(transition);
                }
            }
        }
    }

    file.close();
    return mealyTable;
}

void WriteTableCout(const Table &table, int space) {
    for (auto &row: table) {
        for (int i = 0; i < row.size(); ++i) {
            if (i == 0) {
                std::cout << row[0];
            } else {
                if (row[i].empty()) {
                    std::cout << std::setw(space) << "--";
                } else {
                    std::cout << std::setw(space) << row[i];
                }
            }
        }
        std::cout << std::endl;
    }
    std::cout << "----------------------" << std::endl;
}

void WriteMealyFromTable(const std::string &filename, Table &mealyTable) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::invalid_argument("Can`t open file" + filename);
    }

    for (int j = 1; j < mealyTable[0].size(); ++j) {
        file << ";" << mealyTable[0][j];
    }
    file << std::endl;

    for (int i = 1; i < mealyTable.size(); ++i) {
        file << mealyTable[i][0];
        for (int j = 1; j < mealyTable[i].size(); ++j) {
            file << ";" << mealyTable[i][j];
        }
        file << std::endl;
    }

    file.close();
}

Table MealyGraphToTable(MealyVer *startVer) {
    std::queue<MealyVer *> stateQueue;
    stateQueue.push(startVer);

    // Получаем map состояний к вершинам
    std::map<std::string, MealyVer *> stateMap;
    std::set<std::string> inSet;
    while (!stateQueue.empty()) {
        auto ver = stateQueue.front();
        stateQueue.pop();
        if (ver->GetState().empty()) {
            continue;
        }
        stateMap[ver->GetState()] = ver;
        for (auto &[inSymbol, nextOutSymbolVer]: ver->GetTransitions()) {
            inSet.insert(inSymbol);
            if (stateMap.find(nextOutSymbolVer.second->GetState()) == stateMap.end()) {
                stateQueue.push(nextOutSymbolVer.second);
            }
        }
    }
    //Заполняем вх символы
    Table mealyTable;
    mealyTable.push_back({"\\ "});
    for (const auto &in: inSet) {
        mealyTable.push_back({in});
    }

    for (auto &[state, ver]: stateMap) {
        mealyTable[0].push_back(state);               // текущее состояние
        for (auto &[inSymbol, nextOutSymbolVer]: ver->GetTransitions()) {
            //todo проверять на пустые
            for (int i = 1; i < mealyTable.size(); ++i) {
                if (mealyTable[i][0] == inSymbol) {
                    if (nextOutSymbolVer.second->GetState().empty()) {
                        mealyTable[i].emplace_back("");
                    } else {
                        mealyTable[i].push_back(nextOutSymbolVer.second->GetState() + "/" + nextOutSymbolVer.first);
                    }
                    break;
                }
            }

        }
    }
    return mealyTable;
}

MealyVer *TableToMealyGraph(const Table &table) {
    if (table.size() < 2 || table[0].size() < 2) {
        return new MealyVer();
    }
//  Создаем состояния
//  map[колонка в таблице] = вершина
    std::map<int, MealyVer *> mealyIndexMap;
    std::map<std::string, MealyVer *> mealyStateMap;
    auto startVer = new MealyVer(table[0][1]);
    mealyIndexMap[1] = startVer;
    mealyStateMap[startVer->GetState()] = startVer;

    for (int i = 2; i < table[0].size(); ++i) {
        auto ver = new MealyVer(table[0][i]);
        mealyIndexMap[i] = ver;
        mealyStateMap[ver->GetState()] = ver;
    }

    for (auto &[index, ver]: mealyIndexMap) {
        for (int i = 1; i < table.size(); ++i) {
            auto [state, outSymbol] = SplitMealyState(table[i][index]);
            auto it = mealyStateMap.find(state);
            if (it != mealyStateMap.end()) {
                ver->AddTransition(table[i][0], outSymbol, it->second);
            } else {
                ver->AddTransition(table[i][0], "", new MealyVer);
            }
        }
    }

    return startVer;
}

void RenameMooreGraphStates(MooreVer *startVer, const std::string &symbol) {
    std::queue<MooreVer *> stateQueue; // очередь для обхода графа
    std::map<std::string, MooreVer *> visitedMooreVers; // Посещенные вершины
    std::map<std::string, std::string> convertStates;

    stateQueue.push(startVer);
    int index = 0;
    while (!stateQueue.empty()) {
        auto ver = stateQueue.front();
        stateQueue.pop();
        if (ver->GetState().empty())
            continue;
        if (visitedMooreVers.find(ver->GetState()) == visitedMooreVers.end()) {
            convertStates[ver->GetState()] = symbol + std::to_string(index++);
            for (auto &[inSymbol, nextVer]: ver->GetTransitions()) {
                stateQueue.push(nextVer);
            }
        }
        visitedMooreVers[ver->GetState()] = ver;
    }
    for (auto &[state, ver]: visitedMooreVers) {
        ver->SetState(convertStates[state]);
    }
}


void WriteMooreFromTable(const std::string &filename, Table &mooreTable) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::invalid_argument("Can`t open file" + filename);
    }
    for (int i = 0; i < mooreTable.size(); ++i) {
        if (i < 2) {
            for (int j = 1; j < mooreTable[i].size(); ++j) {
                file << ";" << mooreTable[i][j];
            }
            file << std::endl;
            continue;
        }
        file << mooreTable[i][0];
        for (int j = 1; j < mooreTable[i].size(); ++j) {
            file << ";" << mooreTable[i][j];
        }
        file << std::endl;
    }

    file.close();
}

MooreVer *TableToMooreGraph(const Table &table) {
    if (table.size() < 3 || table[0].size() < 2) {
        return new MooreVer();
    }
//  Создаем состояния
//  map[колонка в таблице] = вершина
    std::map<int, MooreVer *> tempMap;
    auto startVer = new MooreVer(table[1][1], table[0][1]);
    tempMap[1] = startVer;
    for (int i = 2; i < table[0].size(); ++i) {
        auto ver = new MooreVer(table[1][i], table[0][i]);
        tempMap[i] = ver;
    }

    for (auto &[index, ver]: tempMap) {
        for (int i = 2; i < table.size(); ++i) {
            bool emptyTransition = true;
            for (auto &[index1, ver1]: tempMap) {
                if (ver1->GetState() == table[i][index]) {
                    ver->AddTransition(table[i][0], ver1);
                    emptyTransition = false;
                    break;
                }
            }
            if (emptyTransition) {
                ver->AddTransition(table[i][0], new MooreVer);
            }
        }
    }

    return startVer;
}

Table MooreGraphToTable(MooreVer *startVer) {
    std::queue<MooreVer *> stateQueue;
    stateQueue.push(startVer);

    // Получаем map состояний к вершинам
    std::map<std::string, MooreVer *> stateMap;
    std::set<std::string> inSet;
    std::set<std::string> visitedVersSet;
    visitedVersSet.insert(startVer->GetState());
    while (!stateQueue.empty()) {
        auto ver = stateQueue.front();
        stateQueue.pop();
        if (ver->GetState().empty()) {
            continue;
        }
        stateMap[ver->GetState()] = ver;
        for (auto &[inSymbol, nextVer]: ver->GetTransitions()) {
            inSet.insert(inSymbol);
            if (visitedVersSet.find(nextVer->GetState()) == visitedVersSet.end()) {
                visitedVersSet.insert(nextVer->GetState());
                stateQueue.push(nextVer);
            }
        }
    }
    //Заполняем вх символы
    Table mooreTable;
    mooreTable.reserve(inSet.size() + 2);
    mooreTable.push_back({"\\ "});
    mooreTable.push_back({" \\"});
    for (const auto &in: inSet) {
        mooreTable.push_back({in});
    }

    for (auto &[state, ver]: stateMap) {
        mooreTable[0].push_back(ver->GetOutSymbol()); // вх символ
        mooreTable[1].push_back(state);               // текущее состояние
        for (auto &[inSymbol, nextVer]: ver->GetTransitions()) {
            //todo проверять на пустые
            for (int i = 2; i < mooreTable.size(); ++i) {
                if (mooreTable[i][0] == inSymbol) {
                    mooreTable[i].push_back(nextVer->GetState());
                    break;
                }
            }

        }
    }
    return mooreTable;
}

Table ReadMooreToTable(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::invalid_argument("Can`t open file" + filename);
    }
    Table mooreTable;
    std::string line;

    mooreTable.push_back({"\\ "});
    mooreTable.push_back({" \\"});

    // Чтение первой строки (выходные символы)
    if (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string outSignals;
        std::getline(ss, outSignals, ';');
        while (std::getline(ss, outSignals, ';')) {
            if (!outSignals.empty() && outSignals != "\"\"") {
                mooreTable[0].push_back(outSignals);
            } else {
                throw std::invalid_argument("Wrong out symbols format");
            }
        }
    }

    // Чтение второй строки (состояния)
    if (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string state;
        std::getline(ss, state, ';');
        while (std::getline(ss, state, ';')) {
            if (!state.empty() && state != "\"\"") {
                mooreTable[1].push_back(state);
            } else {
                throw std::invalid_argument("Wrong state format");
            }
        }
    }

    while (std::getline(file, line)) {
        mooreTable.emplace_back();
        auto lastRow = --mooreTable.end();
        std::stringstream ss(line);
        std::string input;
        if (std::getline(ss, input, ';')) {
            lastRow->push_back(input);
            std::string transition;
            while (std::getline(ss, transition, ';')) {
                if (transition == "\"\"" || transition.empty()) {
                    lastRow->push_back("");
                } else {
                    lastRow->push_back(transition);
                }
            }
        }
    }

    file.close();
    return mooreTable;
}


MooreVer *MealyToMoore(MealyVer *startMealyVer) {
    std::queue<MealyVer *> stateQueue; // очередь для обхода графа
    std::map<std::string, MealyVer *> visitedMealyVers; // Посещенные вершины
    std::map<std::string, MooreVer *> mooreStateMap; // map новых Moore вершин
    stateQueue.push(startMealyVer);
    while (!stateQueue.empty()) {
        auto ver = stateQueue.front();
        stateQueue.pop();
        if (ver->GetState().empty()) {
            continue;
        }

        for (auto &[inSymbol, nextOutSymbolVer]: ver->GetTransitions()) {
            if (nextOutSymbolVer.second->GetState().empty()) {
                continue;
            }
            const std::string nextState = nextOutSymbolVer.second->GetState() + "/" + nextOutSymbolVer.first;
            if (visitedMealyVers.find(nextState) != visitedMealyVers.end()) {
                continue;
            }
            if (mooreStateMap.find(nextState) == mooreStateMap.end()) {
                auto mooreVer = new MooreVer(nextState, nextOutSymbolVer.first);
                mooreStateMap[nextState] = mooreVer;
                visitedMealyVers[nextState] = nextOutSymbolVer.second; // Помечаем что уже были в этой вершине
                stateQueue.push(nextOutSymbolVer.second);
            }
        }
    }

    auto startMooreVer = mooreStateMap.begin()->second;
    // Добавляем переходы
    for (auto &[MealyTransitionName, mooreVer]: mooreStateMap) {
        auto it = visitedMealyVers.find(MealyTransitionName);
        if (it != visitedMealyVers.end()) { // Нашли mooreVer <=> mealyVer
            for (auto &[inSymbol, outSymbolAndMealyVer]: it->second->GetTransitions()) {
                if (outSymbolAndMealyVer.second->GetState().empty()) {
                    mooreVer->AddTransition(inSymbol, new MooreVer());
                    continue;
                }
                const std::string nextState =
                        outSymbolAndMealyVer.second->GetState() + "/" + outSymbolAndMealyVer.first;
                mooreVer->AddTransition(inSymbol, mooreStateMap.find(nextState)->second);
            }
        }
    }
    RenameMooreGraphStates(startMooreVer, "q");
    return startMooreVer;
}

MealyVer *MooreToMealy(MooreVer *startMooreVer) {

    std::queue<MooreVer *> stateQueue; // очередь для обхода графа
    std::map<std::string, MooreVer *> visitedMooreVers; // Посещенные вершины
    std::map<std::string, MealyVer *> mealyStateMap; // map новых Mealy вершин
    stateQueue.push(startMooreVer);

    // Собираем 2 map состояний
    while (!stateQueue.empty()) {
        auto ver = stateQueue.front();
        stateQueue.pop();
        if (ver->GetState().empty()) {
            continue;
        }

        for (auto &[inSymbol, nextVer]: ver->GetTransitions()) {
            if (nextVer->GetState().empty()) {
                continue;
            }
            if (visitedMooreVers.find(nextVer->GetState()) != visitedMooreVers.end()) {
                continue;
            }
            const std::string nextState = nextVer->GetState() + "/" + nextVer->GetOutSymbol();
            if (mealyStateMap.find(nextState) == mealyStateMap.end()) {
                auto mealyVer = new MealyVer(nextVer->GetState());
                mealyStateMap[nextState] = mealyVer;
                visitedMooreVers[nextState] = nextVer; // Помечаем что уже были в этой вершине
                stateQueue.push(nextVer);
            }
        }
    }

    auto startMealyVer = mealyStateMap.begin()->second;

    // Добавляем переходы
    for (auto &[MealyTransitionName, mealyVer]: mealyStateMap) {
        auto it = visitedMooreVers.find(MealyTransitionName);
        if (it != visitedMooreVers.end()) { // Нашли mealyVer <=> mealyVer
            for (auto &[inSymbol, mooreVer]: it->second->GetTransitions()) {
                if (mooreVer->GetState().empty()) {
                    mealyVer->AddTransition(inSymbol, "", new MealyVer());
                    continue;
                }
                const std::string nextState = mooreVer->GetState() + "/" + mooreVer->GetOutSymbol();
                auto it1 = mealyStateMap.find(nextState);
                if (it1 != mealyStateMap.end()) {
                    mealyVer->AddTransition(inSymbol, mooreVer->GetOutSymbol(), it1->second);
                }
            }
        }
    }
    return startMealyVer;
}
