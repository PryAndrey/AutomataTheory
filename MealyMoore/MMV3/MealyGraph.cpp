#pragma onse

#include <string>
#include <utility>
#include <set>
#include <fstream>
#include <sstream>
#include <queue>
#include <iostream>
#include "MealyGraph.h"
#include "MooreGraph.h"

void MealyGraph::FillGraphFromCSVFile(const std::string &fileName) {
    std::ifstream file(fileName);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + fileName);
    }

    std::string line;
    // Чтение первой строки (состояния)
    if (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string state;
        std::getline(ss, state, ';');
        std::getline(ss, state, ';');
        m_states.emplace_back(state, std::set<int>());
        while (std::getline(ss, state, ';')) {
            if (!state.empty() && state != "\"\"") {
                m_states.emplace_back(state, std::set<int>());
            } else {
                throw std::invalid_argument("Wrong states format");
            }
        }
    }

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string inSymbol;
        if (std::getline(ss, inSymbol, ';')) {
            m_inSymbols.push_back(inSymbol);
            int index = 0;
            std::string transitionDataString;
            while (std::getline(ss, transitionDataString, ';')) {
                if (transitionDataString == "\"\"" || transitionDataString.empty()) {
                    m_transitions.emplace_back(index, -1, inSymbol, "");
                } else {
                    auto transitionData = SplitMealyState(transitionDataString);
                    m_transitions.emplace_back(index, FindStateByString(transitionData.first), inSymbol,
                                               transitionData.second);
                }
                m_states[index].second.insert(m_transitions.size() - 1);
                index++;
            }
        }
    }

    file.close();
    TrimStates();
}

std::pair<std::string, std::string> MealyGraph::SplitMealyState(const std::string &input) {
    std::stringstream ss(input);

    std::string item1, item2;
    std::getline(ss, item1, '/');
    std::getline(ss, item2, '/');
    return {item1, item2};
}

int MealyGraph::FindStateByString(const std::string &state) {
    for (int i = 0; i < m_states.size(); ++i) {
        if (m_states[i].first == state) {
            return i;
        }
    }
    throw std::invalid_argument("State dont exist");
}

MooreGraph MealyGraph::ToMooreGraph() {
    MooreGraph newGraph;
    newGraph.m_inSymbols = m_inSymbols;

    std::map<std::string, int> statesMap; // state/oytS -> index
    std::map<std::string, std::set<int>> fromMap; // state -> [indexes]
    std::map<std::string, std::string> newNameConvert;
    std::vector<MooreState> startStates;
    {
        std::set<std::string> statesSet;
        for (auto &state: m_states) {
            for (auto &transitionIndex: state.second) {
                auto transition = m_transitions[transitionIndex];
                if (transition.m_to == -1) {
                    continue;
                }
                std::string mooreStateName = m_states[transition.m_to].first + "/" + transition.m_outSymbol;
                auto it = statesSet.find(mooreStateName);
                if (it == statesSet.end()) {
                    MooreState newState(mooreStateName, transition.m_outSymbol);
                    statesSet.insert(mooreStateName);
                    if (transition.m_to == 0) {
                        startStates.push_back(newState);
                    } else {
                        newGraph.m_states.push_back(newState);
                    }
                }
            }
        }

        // Заполняем 1-ыми вершинами
        if (startStates.empty()) {
            MooreState newState(m_states[0].first, "");
            newGraph.m_states.insert(newGraph.m_states.cbegin(), newState);
        } else {
            newGraph.m_states.insert(newGraph.m_states.begin(), startStates.begin(), startStates.end());
        }

        int i = 0;
        char newSymbol = 'q';
        for (auto &state: newGraph.m_states) {
            statesMap[state.state] = i;
            newNameConvert[state.state] = newSymbol + std::to_string(i++);
        }

        for (auto &state: newGraph.m_states) {
            fromMap[SplitMealyState(state.state).first].insert(statesMap[state.state]);
        }
    }

    // Добавляем переходы
    for (auto &transition: m_transitions) {
        if (transition.m_to == -1) {
            for (auto &newStateInd: fromMap[SplitMealyState(m_states[transition.m_from].first).first]) {
                MooreTransition mooreTransition(newStateInd, -1, transition.m_inSymbol);
                newGraph.m_transitions.push_back(mooreTransition);
                newGraph.m_states[newStateInd].transitions.insert(newGraph.m_transitions.size() - 1);
            }
            continue;
        }
        std::string mooreStateName = m_states[transition.m_to].first + "/" + transition.m_outSymbol;
        for (auto &newStateInd: fromMap[SplitMealyState(m_states[transition.m_from].first).first]) {
            MooreTransition mooreTransition(newStateInd, statesMap[mooreStateName], transition.m_inSymbol);
            newGraph.m_transitions.push_back(mooreTransition);
            newGraph.m_states[newStateInd].transitions.insert(newGraph.m_transitions.size() - 1);
        }

    }
    if (!TEST) {
        for (auto &state: newGraph.m_states) {
            state.state = newNameConvert[state.state];
        }
    }

    return newGraph;
}

void MealyGraph::WriteToCSVFile(const std::string &filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::invalid_argument("Can`t open file" + filename);
    }

    for (auto &state: m_states) {
        file << ";" << state.first;
    }
    file << std::endl;
    for (const auto &inSymbol: m_inSymbols) {
        file << inSymbol;
        for (auto &[state, transitionsSet]: m_states) {
            for (auto &transition: transitionsSet) {
                if (m_transitions[transition].m_inSymbol == inSymbol) {
                    std::string toState =
                            m_transitions[transition].m_to != -1
                            ? m_states[m_transitions[transition].m_to].first + "/" +
                              m_transitions[transition].m_outSymbol
                            : "";
                    file << ";" << toState;
                    break;
                }
            }
        }
        file << std::endl;
    }

    file.close();
}

void MealyGraph::TrimStates() {
    struct QueueContainer {

        QueueContainer(std::string state, int index, std::set<int> transitions)
                : state(std::move(state)), index(index), transitions(std::move(transitions)) {}

        std::string state;
        int index;
        std::set<int> transitions;
    };

    std::queue<QueueContainer> stateQueue; // очередь для обхода графа
    std::map<std::string, int> visitedStates; // Посещенные вершины

    stateQueue.emplace(m_states[0].first, 0, m_states[0].second);
    int i = 0;
    while (!stateQueue.empty()) {
        auto stateInfo = stateQueue.front();
        stateQueue.pop();
        if (visitedStates.find(stateInfo.state) == visitedStates.end()) {
            for (auto &transitionIndex: stateInfo.transitions) {
                auto transition = m_transitions[transitionIndex];
                auto stateTo = m_states[transition.m_to];
                if (stateInfo.state != stateTo.first && visitedStates.find(stateTo.first) == visitedStates.end()) {
                    stateQueue.emplace(stateTo.first, transition.m_to, stateTo.second);
                }
            }
        }
        visitedStates[stateInfo.state] = stateInfo.index;
    }

    std::vector<MealyTransition> newTransitions;
    std::vector<std::pair<std::string, std::set<int>>> newStates;
    std::map<std::string, int> newStatesMap;
    newStates.reserve(visitedStates.size());
    for (auto &[stateName, index]: visitedStates) {
        std::set<int> tempTransition;
        for (auto &transitionIndex: m_states[index].second) {
            auto transition = m_transitions[transitionIndex];
            newTransitions.emplace_back(newStates.size(), transition.m_to, transition.m_inSymbol,
                                        transition.m_outSymbol);
            tempTransition.insert(newTransitions.size() - 1);
        }
        newStatesMap[m_states[index].first] = newStates.size();
        newStates.emplace_back(m_states[index].first, tempTransition);
    }

    for (auto &transition: newTransitions) {
        transition.m_to = newStatesMap[m_states[transition.m_to].first];
    }

    m_states = newStates;
    m_transitions = newTransitions;
}
