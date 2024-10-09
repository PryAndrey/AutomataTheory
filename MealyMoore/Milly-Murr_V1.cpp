#pragma once

#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <tuple>
#include <iomanip>
#include <fstream>
#include <map>
#include <set>
#include <queue>
#include <memory>

using namespace std;

using Table = vector<vector<string>>;

Table ReadMealy(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw invalid_argument("Can`t open file" + filename);
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
                throw invalid_argument("Wrong states format");
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

Table ReadMoore(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw invalid_argument("Can`t open file" + filename);
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
                throw invalid_argument("Wrong out symbols format");
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
                throw invalid_argument("Wrong state format");
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

void WriteTableCout(const Table &table, int space = 5) {
    for (auto &row: table) {
        for (int i = 0; i < row.size(); ++i) {
            if (i == 0) {
                cout << row[0];
            } else {
                if (row[i].empty()) {
                    cout << setw(space) << "--";
                } else {
                    cout << setw(space) << row[i];
                }
            }
        }
        cout << endl;
    }
    cout << "----------------------" << endl;
}

void WriteMealy(const std::string &filename, Table &mealyTable) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw invalid_argument("Can`t open file" + filename);
    }

    for (int j = 1; j < mealyTable[0].size(); ++j) {
        file << ";" << mealyTable[0][j];
    }
    file << endl;

    for (int i = 1; i < mealyTable.size(); ++i) {
        file << mealyTable[i][0];
        for (int j = 1; j < mealyTable[i].size(); ++j) {
            file << ";" << mealyTable[i][j];
        }
        file << endl;
    }

    file.close();
}

void WriteMoore(const std::string &filename, Table &mooreTable) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw invalid_argument("Can`t open file" + filename);
    }
    for (int i = 0; i < mooreTable.size(); ++i) {
        if (i < 2) {
            for (int j = 1; j < mooreTable[i].size(); ++j) {
                file << ";" << mooreTable[i][j];
            }
            file << endl;
            continue;
        }
        file << mooreTable[i][0];
        for (int j = 1; j < mooreTable[i].size(); ++j) {
            file << ";" << mooreTable[i][j];
        }
        file << endl;
    }

    file.close();
}

pair<string, string> SplitMealyState(const std::string &input) {
    std::stringstream ss(input);

    std::string item1, item2;
    std::getline(ss, item1, '/');
    std::getline(ss, item2, '/');
//    item1 = item1.empty() ? "-" : item1;
//    item2 = item2.empty() ? "-" : item2;
    return pair<string, string>(item1, item2);
}


Table MooreToMealy(const Table &mooreTable) {
    Table mealyTable(mooreTable.size() - 1, vector<string>(mooreTable[0].size()));
    mealyTable[0][0] = "\\";
    // копируем состояния
    mealyTable[0] = mooreTable[1];
    // копируем входные состояния
    for (int i = 2; i < mooreTable.size(); ++i) {
        mealyTable[i - 1][0] = mooreTable[i][0];
    }
    // Сносим выходные символы в переходы
    for (int i = 2; i < mooreTable.size(); ++i) {
        for (int j = 1; j < mooreTable[i].size(); ++j) {
            if (mooreTable[i][j].empty()) {
                mealyTable[i - 1][j] = "";
            } else {
                mealyTable[i - 1][j] = mooreTable[i][j] + "/" + mooreTable[0][j];
            }
        }
    }
    return mealyTable;
}

Table MealyToMoore(const Table &mealyTable) {
    // Собираем все уникальные переходы(новые состояния F1/x1 -> F1 x1 columnIndexes)
    map<string, std::tuple<string, string, set<int>>> mealyStatesSet;
    for (int i = 1; i < mealyTable.size(); ++i) {
        for (int j = 1; j < mealyTable[i].size(); ++j) {
            if (!mealyTable[i][j].empty()) {
                std::stringstream ss1(mealyTable[i][j]);
                pair<string, string> stateOut = SplitMealyState(mealyTable[i][j]);
                if (mealyStatesSet.find(mealyTable[i][j]) == mealyStatesSet.end()) {
                    mealyStatesSet[mealyTable[i][j]] = {stateOut.first, stateOut.second, {j}};
                } else {
                    set<int> temp = get<2>(mealyStatesSet[mealyTable[i][j]]);
                    temp.insert(j);
                    mealyStatesSet[mealyTable[i][j]] = {stateOut.first, stateOut.second, {temp}};
                }
            }
        }
    }

    Table mooreTable(mealyTable.size() + 1, vector<string>(mealyStatesSet.size() + 1));
    mooreTable[0][0] = "\\ ";
    mooreTable[1][0] = " \\";

    // копируем входные состояния
    for (int i = 1; i < mealyTable.size(); ++i) {
        mooreTable[i + 1][0] = mealyTable[i][0];
    }

    map<string, string> oldStatesToNewStates;
    int j = 0;
    //Заполняем таблицу новыми состояниями и соответствующими вых символами
    for (auto &stateOut: mealyStatesSet) {
        if (!stateOut.first.empty()) {
            string newStateName = "q" + to_string(j);
            oldStatesToNewStates[stateOut.first] = newStateName;
            mooreTable[0][j + 1] = get<1>(stateOut.second);
            mooreTable[1][j + 1] = newStateName;
            j++;
        }
    }

    j = 0;
    for (auto &stateOut: mealyStatesSet) {
        for (int k = 2; k < mooreTable.size(); ++k) {
            for (auto &i: get<2>(stateOut.second)) {
                if (mealyTable[k - 1][i].empty()) {
                    mooreTable[k][j + 1] = "";
                } else {
                    mooreTable[k][j + 1] = oldStatesToNewStates[mealyTable[k - 1][i]];
                }
            }
        }
        j++;
    }

    return mooreTable;
}

Table TrimmingMooreStates(Table &mooreTable) {
    set<string> stateVisited;
    queue<pair<string, int>> stateQueue;

    stateQueue.emplace(mooreTable[1][1], 1);
    //Получаем достижимые вершины
    while (!stateQueue.empty()) {
        auto [currentState, mooreIndex] = stateQueue.front();
        stateQueue.pop();

        if (stateVisited.find(currentState) != stateVisited.end()) {
            continue;
        }
        stateVisited.insert(currentState);
        for (int j = 2; j < mooreTable.size(); j++) {
            string nextState = mooreTable[j][mooreIndex];
            if (stateVisited.find(nextState) == stateVisited.end() && !nextState.empty()) {
                for (int i = 1; i < mooreTable[1].size(); ++i) {
                    if (mooreTable[1][i] == nextState) {
                        stateQueue.emplace(nextState, i);
                        break;
                    }
                }
            }
        }
    }

    // Удаляем колонку которую не нашли
    for (int i = 1; i < mooreTable[1].size(); ++i) {
        if (stateVisited.find(mooreTable[1][i]) == stateVisited.end()) {
            for (auto &mooreRow: mooreTable) {
                mooreRow.erase(mooreRow.begin() + i);
            }
            i--;
        }
    }

    // Удаляем вх символ по которому никто не переходит
    for (int i = 0; i < mooreTable.size(); ++i) {
        bool clear = true;
        for (int j = 1; j < mooreTable[i].size(); ++j) {
            if (!mooreTable[i][j].empty()) {
                clear = false;
                break;
            }
        }
        if (clear) {
            mooreTable.erase(mooreTable.begin() + i);
        }
    }

    return mooreTable;
}

Table TrimmingMealyStates(Table &mealyTable) {
    set<string> stateVisited;
    queue<pair<string, int>> stateQueue;

    stateQueue.emplace(mealyTable[0][1], 1);

    //Получаем достижимые вершины
    while (!stateQueue.empty()) {
        auto [currentState, mooreIndex] = stateQueue.front();
        stateQueue.pop();

        if (stateVisited.find(currentState) != stateVisited.end()) {
            continue;
        }
        stateVisited.insert(currentState);
        for (int j = 1; j < mealyTable.size(); j++) {
            string nextStateOut = mealyTable[j][mooreIndex];
            pair<string, string> nextStateOutPair = SplitMealyState(nextStateOut);
            if (stateVisited.find(nextStateOutPair.first) == stateVisited.end() && !nextStateOutPair.first.empty()) {
                for (int i = 1; i < mealyTable[0].size(); ++i) {
                    if (mealyTable[0][i] == nextStateOutPair.first) {
                        stateQueue.emplace(nextStateOutPair.first, i);
                        break;
                    }
                }
            }
        }
    }

    // Удаляем колонку которую не нашли
    for (int i = 1; i < mealyTable[0].size(); ++i) {
        if (stateVisited.find(mealyTable[0][i]) == stateVisited.end()) {
            for (auto &mealyRow: mealyTable) {
                mealyRow.erase(mealyRow.begin() + i);
            }
        }
    }

    // Удаляем вх символ по которому никто не переходит
    for (int i = 0; i < mealyTable.size(); ++i) {
        bool clear = true;
        for (int j = 1; j < mealyTable[i].size(); ++j) {
            if (!mealyTable[i][j].empty()) {
                clear = false;
                break;
            }
        }
        if (clear) {
            mealyTable.erase(mealyTable.begin() + i);
        }
    }
    return mealyTable;
}

size_t UniqueNames(vector<string> &row) {
    set<string> setNames;
    for (int i = 1; i < row.size(); ++i) {
        if (setNames.find(row[i]) == setNames.end()) {
            setNames.insert(row[i]);
        }
    }
    return setNames.size();
}

Table InitMooreMinimize(Table prevMoore, char nextSymbol = 'A') {
    Table currMoore = prevMoore;

    //Делим по вых. символам
    //outSymbol -> [statesIndex...]
    map<string, set<int>> outToStatesSet;
    for (int i = 1; i < currMoore[1].size(); ++i) {
        outToStatesSet[currMoore[1][i]].insert(i);
    }

    //Перемещаем все по позициям
    map<string, string> convertedStatesToNewNames;
    int stateIndInit = 1, indInit = 1;
    for (auto &[out, statesIndex]: outToStatesSet) {
        for (auto &stateIndex: statesIndex) {
            convertedStatesToNewNames[prevMoore[2][stateIndex]] = nextSymbol + to_string(stateIndInit);
            currMoore[2][indInit] = nextSymbol + to_string(stateIndInit);
            for (int i = 0; i < currMoore.size(); ++i) {
                currMoore[i][indInit] = prevMoore[i][stateIndex];
            }
            indInit++;
        }
        stateIndInit++;
    }
    //Переименовываем переходы
    for (int i = 2; i < currMoore.size(); ++i) {
        for (int j = 1; j < currMoore[i].size(); ++j) {
            currMoore[i][j] = convertedStatesToNewNames[currMoore[i][j]];
        }
    }
    return currMoore;
}

Table MinimizeMoore(Table mooreTable) {
    Table prevMoore = mooreTable;
    prevMoore.insert(prevMoore.begin(), mooreTable[1]);
    Table currMoore = InitMooreMinimize(prevMoore);

    WriteTableCout(currMoore, 3);

    char nextSymbol = 'B';

    while (UniqueNames(prevMoore[2]) != UniqueNames(currMoore[2])) {
        prevMoore = currMoore;

        Table tempMoore(currMoore.size());
        tempMoore[0].emplace_back("\\ ");
        tempMoore[1] = currMoore[1];
        tempMoore[2].emplace_back(" \\");
        // копируем входные состояния
        for (int i = 3; i < tempMoore.size(); ++i) {
            tempMoore[i].emplace_back(currMoore[i][0]);
        }

        int newStateIndex = 0;
        map<string, string> stateToState;
        for (int i = 1; i < currMoore[1].size(); ++i) {
            // <statesSum> -> [<OriginalState, newStateName(везде 1)>, columnIndex]
            map<string, set<std::tuple<string, string, int>>> convStates;
            for (int j = i; currMoore[2][i] == currMoore[2][j]; ++j) {
                // Собираем всю колонку переходов
                string stateSum;
                for (int k = 3; k < currMoore.size(); ++k) {
                    stateSum += currMoore[k][j];
                }
                // map уникальных комбинаций
                if (convStates.find(stateSum) == convStates.end()) {
                    convStates[stateSum].insert({currMoore[0][j], nextSymbol + to_string(newStateIndex++), j});
                } else {
                    convStates[stateSum].insert({currMoore[0][j], std::get<1>(*convStates[stateSum].begin()), j});
                }
                i = j;
            }

            for (auto &[sumStates, OriginalToNewStatesWithIndex]: convStates) {
                for (auto &[original, newState, index]: OriginalToNewStatesWithIndex) {
                    tempMoore[2].push_back(newState);
                    stateToState[original] = newState;
                    // Ищем столбец оригинальной таблицы
                    for (int k = 1; k < mooreTable[1].size(); ++k) {
                        if (original == mooreTable[1][k]) {
                            tempMoore[0].push_back(mooreTable[1][k]);
                            for (int l = 2; l < mooreTable.size(); ++l) {
                                tempMoore[l + 1].push_back(mooreTable[l][k]);
                            }
                            break;
                        }
                    }
                }
            }
        }

        //Переименовываем состояния
        for (int i = 3; i < tempMoore.size(); ++i) {
            for (int j = 1; j < tempMoore[i].size(); ++j) {
                tempMoore[i][j] = stateToState[tempMoore[i][j]];
            }
        }

        WriteTableCout(tempMoore, 3);
        currMoore = tempMoore;
        nextSymbol++;
    }

    for (int i = 2; i < currMoore[2].size(); ++i) {
        if (currMoore[2][i] == currMoore[2][i - 1]) {
            for (auto &row: currMoore) {
                row.erase(row.begin() + i);
            }
            i--;
        }
    }
    return currMoore;
}

Table InitMealyMinimize(Table prevMealy, char nextSymbol = 'A') {
    //Делим по комбинациям вых. символов
    //outSymbol -> [statesIndex...]
    map<string, set<int>> outToStatesSet;
    for (int i = 1; i < prevMealy[1].size(); ++i) {
        string summOut;
        for (int j = 2; j < prevMealy.size(); ++j) {
            string tempOutSymbol = SplitMealyState(prevMealy[j][i]).second;
            summOut += !tempOutSymbol.empty() ? tempOutSymbol : "--";
        }
        outToStatesSet[summOut].insert(i);
    }

    Table currMealy(prevMealy.size());
    currMealy[0].emplace_back("\\ ");
    currMealy[1].emplace_back(" \\");
    // копируем входные состояния
    for (int i = 2; i < prevMealy.size(); ++i) {
        currMealy[i].emplace_back(prevMealy[i][0]);
    }

    //Перемещаем все по позициям
    map<string, string> convertedStatesToNewNames;
    int stateIndInit = 1;
    for (auto &[outSumm, statesIndexes]: outToStatesSet) {
        for (auto &stateIndex: statesIndexes) {
            convertedStatesToNewNames[prevMealy[1][stateIndex]] = nextSymbol + to_string(stateIndInit);
            currMealy[1].push_back(nextSymbol + to_string(stateIndInit));
            currMealy[0].push_back(prevMealy[0][stateIndex]);
            for (int i = 2; i < prevMealy.size(); ++i) {
                currMealy[i].push_back(prevMealy[i][stateIndex]);
            }
        }
        stateIndInit++;
    }
    //Переименовываем переходы
    for (int i = 2; i < currMealy.size(); ++i) {
        for (int j = 1; j < currMealy[i].size(); ++j) {
            currMealy[i][j] = convertedStatesToNewNames[SplitMealyState(currMealy[i][j]).first];
        }
    }
    return currMealy;
}

Table MinimizeMealy(Table &mealyTable) {
    Table prevMealy = mealyTable;
    prevMealy.insert(prevMealy.begin(), mealyTable[0]);
    prevMealy[0][0] = "\\ ";

    Table currMealy = InitMealyMinimize(prevMealy);

    WriteTableCout(currMealy, 3);

    char nextSymbol = 'B';

    while (UniqueNames(prevMealy[1]) != UniqueNames(currMealy[1])) {
        prevMealy = currMealy;
        int newStateIndex = 1;

        map<string, string> stateToState;
        for (int i = 1; i < currMealy[1].size(); ++i) {
            // <statesSum> -> [<OriginalState, newState>]
            map<string, set<pair<string, string>>> convStates;
            for (int j = i; currMealy[1][i] == currMealy[1][j]; ++j) {
                // Собираем всю колонку
                string stateSum;
                for (int k = 2; k < currMealy.size(); ++k) {
                    stateSum += currMealy[k][j];
                }
                // map уникальных комбинаций
                if (convStates.find(stateSum) == convStates.end()) {
                    convStates[stateSum].insert({currMealy[0][j], nextSymbol + to_string(newStateIndex++)});
                } else {
                    convStates[stateSum].insert({currMealy[0][j], convStates[stateSum].begin()->second});
                }
                i = j;
            }

            for (auto &[sumStates, OriginalToNewStatesSet]: convStates) {
                for (auto &[original, newState]: OriginalToNewStatesSet) {
                    for (int j = 1; j < currMealy[0].size(); ++j) {
                        if (original == currMealy[0][j]) {
                            currMealy[1][j] = newState;
                            stateToState[original] = newState;
                            for (int k = 1; k < mealyTable[0].size(); ++k) {
                                if (original == mealyTable[0][k]) {
                                    for (int l = 2; l < currMealy.size(); ++l) {
                                        currMealy[l][j] = mealyTable[l - 1][k];
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        //Переименовываем состояния
        for (int i = 2; i < currMealy.size(); ++i) {
            for (int j = 1; j < currMealy[i].size(); ++j) {
                currMealy[i][j] = stateToState[SplitMealyState(currMealy[i][j]).first];
            }
        }

        nextSymbol++;
    }
    Table resultTable(currMealy.size() - 1);
    //Заполняем 1 столбец
    for (int j = 0; j < resultTable.size(); ++j) {
        resultTable[j].push_back(currMealy[j + 1][0]);
    }
    //Заполняем таблицу
    vector<string> temp;
    map<string, string> stateToState;
    set<string> states;
    for (int i = 1; i < currMealy[0].size(); ++i) {
        states.insert(currMealy[0][i]);
    }
    for (int i = 1; i < currMealy[0].size(); ++i) {
        auto it = states.find(currMealy[0][i]);
        if (it != states.end()) {
            for (int j = 0; j < resultTable.size(); ++j) {
                resultTable[j].push_back(currMealy[j + 1][i]);
//                if (j != 0)
//                    stateToState[currMealy[j + 1][i]] = currMealy[1][i];
            }
            temp.push_back(currMealy[2][i]);
            states.erase(it);
        }
    }
    //Переименовываем состояния
    for (int i = 1; i < resultTable.size(); ++i) {
        for (int j = 1; j < resultTable[i].size(); ++j) {
//            resultTable[i][j] = stateToState[resultTable[i][j]];
        }
    }

    return mealyTable;
}

