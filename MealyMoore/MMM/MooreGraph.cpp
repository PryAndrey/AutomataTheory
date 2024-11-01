#pragma onse

#include <string>
#include <fstream>
#include <sstream>
#include <queue>
#include <utility>
#include <iostream>
#include "MooreGraph.h"
#include "MealyGraph.h"

void MooreGraph::FillGraphFromCSVFile(const std::string &fileName) {
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
            std::getline(ssOut, outSymbol, ';');
            std::getline(ssStates, state, ';');
            std::getline(ssStates, state, ';');
            m_states.emplace_back(state, outSymbol, std::set<int>());
            while (std::getline(ssOut, outSymbol, ';')) {
                std::getline(ssStates, state, ';');
                if (!state.empty() && state != "\"\"" && !outSymbol.empty() && outSymbol != "\"\"") {
                    m_states.emplace_back(state, outSymbol, std::set<int>());
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
                if (transition == "\"\"" || transition.empty()) {
                    m_transitions.emplace_back(index, -1, inSymbol);
                } else {
                    m_transitions.emplace_back(index, FindStateByString(transition), inSymbol);
                }
                m_states[index].transitions.insert(m_transitions.size() - 1);
                index++;
            }
        }
    }

    file.close();
    TrimStates();
}

int MooreGraph::FindStateByString(const std::string &state) {
    for (int i = 0; i < m_states.size(); ++i) {
        if (m_states[i].state == state) {
            return i;
        }
    }
    throw std::invalid_argument("State dont exist");
}

MealyGraph MooreGraph::ToMealyGraph() {
    MealyGraph newGraph;
    newGraph.m_inSymbols = m_inSymbols;
    for (auto &state: m_states) {
        newGraph.m_states.emplace_back(state.state, state.transitions);
    }
    for (auto &transition: m_transitions) {
        std::string outSymbol = transition.m_to != -1 ? m_states[transition.m_to].outSymbol : "";
        MealyTransition mealyTransition(transition.m_from, transition.m_to, transition.m_inSymbol, outSymbol);
        newGraph.m_transitions.push_back(mealyTransition);
    }

    return newGraph;
}


void MooreGraph::WriteToCSVFile(const std::string &filename) {
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
            for (auto &transition: state.transitions) {
                if (m_transitions[transition].m_inSymbol == inSymbol) {
                    file << ";" << (m_transitions[transition].m_to != -1
                                    ? m_states[m_transitions[transition].m_to].state : "");

                    break;
                }
            }
        }
        file << std::endl;
    }

    file.close();
}

void MooreGraph::TrimStates() {
    struct QueueContainer {

        QueueContainer(std::string state, int index, std::string outSymbol, std::set<int> transitions)
                : state(std::move(state)), index(index), outSymbol(std::move(outSymbol)),
                  transitions(std::move(transitions)) {}

        std::string state;
        std::string outSymbol;
        int index;
        std::set<int> transitions;
    };

    std::queue<QueueContainer> stateQueue; // очередь для обхода графа
    std::map<std::string, int> visitedStates; // Посещенные вершины

    stateQueue.emplace(m_states[0].state, 0, m_states[0].outSymbol, m_states[0].transitions);
    while (!stateQueue.empty()) {
        auto stateInfo = stateQueue.front();
        stateQueue.pop();
        if (visitedStates.find(stateInfo.state) == visitedStates.end()) {
            for (auto &transitionIndex: stateInfo.transitions) {
                auto transition = m_transitions[transitionIndex];
                auto stateTo = m_states[transition.m_to];
                if (stateInfo.state != stateTo.state && visitedStates.find(stateTo.state) == visitedStates.end()) {
                    stateQueue.emplace(stateTo.state, transition.m_to, stateTo.outSymbol, stateTo.transitions);
                }
            }
        }
        visitedStates[stateInfo.state] = stateInfo.index;
    }

    std::pair<std::string, int> startState;
    std::vector<MooreTransition> newTransitions;
    std::vector<MooreState> newStates;
    std::map<std::string, int> newStatesMap;
    newStates.reserve(visitedStates.size());
    for (auto &[stateName, index]: visitedStates) {
        if (index == 0) {
            startState = std::pair(stateName, index);
            continue;
        }
        std::set<int> tempTransition;
        for (auto &transitionIndex: m_states[index].transitions) {
            auto transition = m_transitions[transitionIndex];
            newTransitions.emplace_back(newStates.size() + 1, transition.m_to, transition.m_inSymbol);
            tempTransition.insert(newTransitions.size() - 1);
        }
        newStatesMap[m_states[index].state] = newStates.size() + 1;
        newStates.emplace_back(m_states[index].state, m_states[index].outSymbol, tempTransition);
    }

    // Ставим 1 вершину
    std::set<int> tempTransition;
    for (auto &transitionIndex: m_states[startState.second].transitions) {
        auto transition = m_transitions[transitionIndex];
        newTransitions.emplace_back(0, transition.m_to, transition.m_inSymbol);
        tempTransition.insert(newTransitions.size() - 1);
    }
    newStatesMap[m_states[startState.second].state] = 0;
    newStates.insert(newStates.cbegin(),
                     MooreState(m_states[startState.second].state, m_states[startState.second].outSymbol,
                                tempTransition));

    for (auto &transition: newTransitions) {
        transition.m_to = newStatesMap[m_states[transition.m_to].state];
    }

    m_states = newStates;
    m_transitions = newTransitions;
}

void MooreGraph::Minimize() {
    TrimStates();

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
                            newStateTransition += stateConverterTemp[m_states[m_transitions[transitionInd].m_to].state].second;
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
                std::string toStateString = toNewStatesMap[transition.m_to];
                for (int i = 0; i < newStates.size(); ++i) {
                    if (toStateString == newStates[i].state) {
                        to = i;
                        break;
                    }
                }
                newTransitions.emplace_back(from, to, transition.m_inSymbol);
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

size_t MooreGraph::UniqueNames(std::map<std::string, std::map<std::string, std::set<int>>> &temp) {
    size_t size = 0;
    for (auto &[outCombination, stateMap]: temp) {
        size += stateMap.size();
    }
    return size;
}
