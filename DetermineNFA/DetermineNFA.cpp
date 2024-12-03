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
                if (transition.m_inSymbol == "ε" || transition.m_inSymbol == "Оµ") {
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
        m_chainedStates.emplace_back(m_states[i].state, i, transitionSet);
    }
}


void DetermineNFA::ConvertToDFA() {
    std::map<std::string, MooreChain> newChainedStateMap;
    std::vector<MooreState> newStates;
    std::vector<MooreTransition> newTransitions;

    {
        auto chStStart = m_chainedStates[0];
        auto stStart = m_states[chStStart.stateInd];

        // Новые цепочки для очереди
        newChainedStateMap[chStStart.state] = MooreChain(chStStart.state, 0, chStStart.chainedStates);

        // Новые состояния и ориентирование по состояниям
        newStates.emplace_back(stStart);
        newStates[0].transitions.clear();
    }

    std::queue<MooreChain> stateQueue;
    stateQueue.emplace(newChainedStateMap[m_chainedStates[0].state]);

    while (!stateQueue.empty()) {
        MooreChain chState = stateQueue.front(); // Связанные состояния(строка)
        stateQueue.pop();

        // Перебираем inSymbols(столбец)
        for (auto &inSymbol: m_inSymbols) {
            std::set<int> toNewStateByInSymbol; // Все переходы по вх символу
            std::string outSymbol;
            // Перебираем, связанные с основным, состояния
            for (int chainedStateInd: chState.chainedStates) {
                // Перебираем переходы состояний через set(для отсутствия повторений - q1q2 и q2q1 - одинаковы)
                for (int transitionInd: m_states[chainedStateInd].transitions) {
                    auto &transition = m_transitions[transitionInd];
                    if (transition.m_inSymbol == inSymbol) {
                        toNewStateByInSymbol.insert(transition.m_to.begin(), transition.m_to.end());
                    }
                }
                if (outSymbol.empty()) {
                    outSymbol = m_states[chainedStateInd].outSymbol;
                }
            }
            if (toNewStateByInSymbol.empty()) {
                continue;
            }

            // Имя состояния
            std::string newToStateName;
            for (int stateInd: toNewStateByInSymbol) {
                newToStateName += m_states[stateInd].state;
            }

            auto it = newChainedStateMap.find(newToStateName);
            if (it != newChainedStateMap.end()) {
                newTransitions.emplace_back(chState.stateInd, std::set<int>{it->second.stateInd}, inSymbol);
                newStates[chState.stateInd].transitions.insert(newTransitions.size() - 1);
                continue;
            }

            // Переходы из нового состояния по вх символу
            std::set<int> toStates;
            for (int stateInd: toNewStateByInSymbol) {
                for (int transitionInd: m_states[stateInd].transitions) {
                    auto &transition = m_transitions[transitionInd];
                    toStates.insert(transition.m_to.begin(), transition.m_to.end());
                }
            }

            newStates[chState.stateInd].transitions.insert(newTransitions.size());

            newStates.emplace_back(newToStateName, outSymbol);
            newTransitions.emplace_back(chState.stateInd, std::set<int>{(int) newStates.size() - 1}, inSymbol);
            newChainedStateMap[newToStateName] = MooreChain(newToStateName, newStates.size() - 1, toStates);
            stateQueue.emplace(newChainedStateMap[newToStateName]);
        }
    }

    for (auto &[stateName, chainedState]: newChainedStateMap) {
        m_chainedStates.emplace_back(chainedState);
    }
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
            for (auto &transition: state.transitions) {
                if (m_transitions[transition].m_inSymbol == inSymbol) {
                    file << ";" << (*m_transitions[transition].m_to.begin() != -1
                                    ? m_states[*m_transitions[transition].m_to.begin()].state : "");

                    break;
                }
            }
        }
        file << std::endl;
    }

    file.close();
}
