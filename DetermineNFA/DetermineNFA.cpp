#pragma onse

#include <set>
#include "DetermineNFA.h"

void DetermineNFA::ReadFromCSVFile(const std::string &fileName) {

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
            m_inSymbols.push_back(inSymbol);
            int index = 0;
            std::string transition;
            while (std::getline(ss, transition, ';')) {
                if (transition != "\"\"" && !transition.empty()) {
                    std::set<int> transitionSet;
                    std::stringstream sst(transition);
                    std::string toState;
                    while (std::getline(sst, toState, ',')) {
                        transitionSet.insert(m_statesMap[toState]);
                    }
                    m_transitions.emplace_back(index, transitionSet, inSymbol);
                    m_states[index].transitions.insert(m_transitions.size() - 1);
                }
                index++;
            }
        }
    }

    file.close();
}

void DetermineNFA::FindChain() {
    for (int i = 0; i < m_states.size(); ++i) {

        std::map<std::string, std::set<int>> chainedStates;
        std::set<int> transitionSet = {i};
        chainedStates[m_states[i].state] = std::set<int>{};

        std::queue<MooreState> stateQueue;
        stateQueue.emplace(m_states[i]);
        while (!stateQueue.empty()) {
            MooreState state = stateQueue.front();
            stateQueue.pop();

            for (const auto &transitionInd: state.transitions) {
                auto &transition = m_transitions[transitionInd];
                if (transition.m_inSymbol == "ε"
                    || transition.m_inSymbol == "E"
                    || transition.m_inSymbol == "Оµ") {
                    auto &toStatesSet = transition.m_to;
                    transitionSet.insert(toStatesSet.begin(), toStatesSet.end());
                    for (const auto &toStateInd: toStatesSet) {
                        auto it = chainedStates.find(m_states[toStateInd].state);
                        if (it == chainedStates.end()) {
                            stateQueue.emplace(m_states[toStateInd]);
                        } else {
                            transitionSet.insert(it->second.begin(), it->second.end());
                        }
                    }
                }
            }
        }
        m_chainedStates[m_states[i].state] = MooreChain(m_states[i].state, i, transitionSet);
    }
}

void DetermineNFA::ConvertToDFA() {
    std::map<std::string, MooreChain> newChainedStateMap;
    std::vector<MooreState> newStates;
    std::vector<MooreTransition> newTransitions;

    newChainedStateMap[m_states[0].state] = m_chainedStates[m_states[0].state];

    std::queue<MooreChain> stateQueue;
    stateQueue.emplace(newChainedStateMap[m_states[0].state]);

    int stateIndex = 1;
    while (!stateQueue.empty()) {
        MooreChain chState = stateQueue.front(); // Связанные состояния(строка)
        stateQueue.pop();

        // Добавляем состояние (без переходов)
        {
            std::string outSymbol;
            for (int stateInd: chState.chainedStates) {
                outSymbol = m_states[stateInd].outSymbol;
                if (!outSymbol.empty()) {
                    break;
                }
            }

            newStates.emplace_back(chState.state, outSymbol);
        }

        // Перебираем inSymbols(столбец)
        for (auto &inSymbol: m_inSymbols) {
            std::set<int> toNewStateByInSymbol; // Все переходы по вх символу
            // Перебираем, связанные с основным, состояния
            for (int chainedStateInd: chState.chainedStates) {
                // Перебираем переходы состояний через set(для отсутствия повторений - q1q2 и q2q1 - одинаковы)
                for (int transitionInd: m_states[chainedStateInd].transitions) {
                    auto &transition = m_transitions[transitionInd];
                    if (transition.m_inSymbol == inSymbol) {
                        toNewStateByInSymbol.insert(transition.m_to.begin(), transition.m_to.end());
                    }
                }
            }
            if (toNewStateByInSymbol.empty()) {
                continue;
            }
            // На этом моменте имеем состояния в которые переходим по символу

            // Имя нового состояния
            std::string newToStateName;
            for (int stateInd: toNewStateByInSymbol) {
                newToStateName += m_states[stateInd].state;
            }

            // Если такое состояние уже попадалось
            auto it = newChainedStateMap.find(newToStateName);
            if (it != newChainedStateMap.end()) {
                newTransitions.emplace_back(chState.stateInd, std::set<int>{it->second.stateInd}, inSymbol);
                newStates[chState.stateInd].transitions.insert(newTransitions.size() - 1);
                continue;
            }

            // Формируем новые связанные состояния для нового состояния
            std::set<int> toStates;
            for (int stateInd: toNewStateByInSymbol) {
                toStates.insert(m_chainedStates[m_states[stateInd].state].chainedStates.begin(),
                                m_chainedStates[m_states[stateInd].state].chainedStates.end());
            }

            newChainedStateMap[newToStateName] = MooreChain(newToStateName, stateIndex, toStates);

            newTransitions.emplace_back(chState.stateInd, std::set<int>{stateIndex}, inSymbol);
            newStates[chState.stateInd].transitions.insert(newTransitions.size() - 1);
            stateQueue.emplace(newChainedStateMap[newToStateName]);
            stateIndex++;
        }
    }

    m_chainedStates = newChainedStateMap;
    std::cout << std::endl;
    m_states = newStates;
    m_statesMap.clear();
    m_transitions = newTransitions;
}

void DetermineNFA::WriteToCSVFile(const std::string &filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::invalid_argument("Can`t open file" + filename);
    }

    for (auto &state: m_states) {
        file << ";" << state.outSymbol;
    }
    file << std::endl;
    for (auto &state: m_states) {
        file << ";" << state.state;
    }
    file << std::endl;
    for (const auto &inSymbol: m_inSymbols) {
        file << inSymbol;
        for (auto &state: m_states) {
            bool find = false;
            for (auto &transition: state.transitions) {
                if (m_transitions[transition].m_inSymbol == inSymbol) {
                    file << ";" << (*m_transitions[transition].m_to.begin() != -1
                                    ? m_states[*m_transitions[transition].m_to.begin()].state : "");
                    find = true;
                    break;
                }
            }

            if (!find) {
                file << ";";
            }
        }
        file << std::endl;
    }

    file.close();
}

size_t DetermineNFA::UniqueNames(std::map<std::string, std::map<std::string, std::set<int>>> &temp) {
    size_t size = 0;
    for (auto &[outCombination, stateMap]: temp) {
        size += stateMap.size();
    }
    return size;
}

void DetermineNFA::Minimize() {
    //Разбиение
    std::map<std::string, std::map<std::string, std::set<int>>> stateMap;
    {
        //Заполняет stateMap 1 раз
        for (int i = 0; i < m_states.size(); ++i) {
            auto &state = m_states[i];
            stateMap[state.outSymbol][state.state].insert(i);
        }

        // Заполняем stateConverter (состояние -> {индекс, новое имя})
        std::map<std::string, std::pair<int, std::string>> stateConverter;
        char newState = 'A';
        int newStateIndex = 0;
        for (auto &[outCombination, stateGroups]: stateMap) {
            for (auto &[state, stateIndexes]: stateGroups) {
                for (auto &stateInd: stateIndexes) {
                    stateConverter[state] = {stateInd, newState + std::to_string(newStateIndex)};
                }
            }
            newStateIndex++;
        }
        //Разбиваем на группы по переходам
        std::map<std::string, std::map<std::string, std::set<int>>> stateMapTemp;
        std::map<std::string, std::pair<int, std::string>> stateConverterTemp;
        newState++;
        while (UniqueNames(stateMap) != UniqueNames(stateMapTemp)) {
            stateConverterTemp = stateConverter;
            stateConverter.clear();
            stateMapTemp = stateMap;
            stateMap.clear();
            newStateIndex = 0;

            for (auto &[outCombination, stateGroups]: stateMapTemp) {
                std::map<std::string, std::string> helpMap;
                for (auto &[state, stateIndexes]: stateGroups) {
                    for (auto &stateIndex: stateIndexes) {
                        std::string newStateTransition;
                        for (auto &transitionInd: m_states[stateIndex].transitions) {
                            newStateTransition += stateConverterTemp[m_states[*m_transitions[transitionInd].m_to.begin()].state].second;
                        }
                        auto it = helpMap.find(newStateTransition);
                        if (it == helpMap.end()) {
                            std::string newName = newState + std::to_string(newStateIndex++);
                            helpMap[newStateTransition] = newName;
                            stateConverter[m_states[stateIndex].state] = {stateIndex, newName};
                            stateMap[outCombination][newName].insert(stateIndex);
                        } else {
                            stateConverter[m_states[stateIndex].state] = {stateIndex, helpMap[it->first]};
                            stateMap[outCombination][helpMap[it->first]].insert(stateIndex);
                        }
                    }
                }
            }
            newState++;
        }
    }

    // Склеивание
    {
        // Создаем map (состояние -> имя группы)
        std::map<int, std::string> toNewStatesMap;
        for (auto &[outCombination, stateGroups]: stateMap) {
            for (auto &[groupName, statesSet]: stateGroups) {
                for (auto &stateIndex: statesSet) {
                    toNewStatesMap[stateIndex] = groupName;
                }
            }
        }

        // Заполняем новый массив состояний новыми вершинами
        std::vector<MooreState> newStates;
        newStates.emplace_back(toNewStatesMap[0], m_states[0].outSymbol, m_states[0].transitions);
        std::vector<MooreTransition> newTransitions;
        std::set<std::string> replacedNames;
        for (auto &[outSymbolComb, newNamesMap]: stateMap) {
            for (auto &[groupName, statesSet]: newNamesMap) {
                if (statesSet.find(0) != statesSet.end()) {
                    continue;
                }
                if (replacedNames.find(groupName) != replacedNames.end()) {
                    continue;
                } else {
                    newStates.emplace_back(groupName, m_states[*(statesSet.begin())].outSymbol,
                                           m_states[*(statesSet.begin())].transitions);
                    replacedNames.insert(groupName);
                }
            }
        }

        // К новым состояниям заполняем новые переходы
        int from = 0;
        for (auto &[groupName, outSymbol, transitionsSet]: newStates) {
            std::set<int> newTransitionsSet;
            for (auto &transitionInd: transitionsSet) {
                auto transition = m_transitions[transitionInd];
                int to = -1;
                std::string toStateString = toNewStatesMap[*transition.m_to.begin()];
                for (int i = 0; i < newStates.size(); ++i) {
                    if (toStateString == newStates[i].state) {
                        to = i;
                        break;
                    }
                }
                newTransitions.emplace_back(from, std::set<int>{to}, transition.m_inSymbol);
                newTransitionsSet.insert(newTransitions.size() - 1);
            }
            transitionsSet = newTransitionsSet;
            from++;
        }
        for (auto &ind: toNewStatesMap) {
            std::cout << m_states[ind.first].state << "->" << ind.second << std::endl;
        }
        m_states = newStates;
        m_transitions = newTransitions;
    }
}