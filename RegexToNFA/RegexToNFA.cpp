#pragma onse

#include <set>
#include "RegexToNFA.h"


void RegexToNFA::WriteToCSVFile(const std::string &filename) {
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
        for (int i = 0; i < m_states.size(); ++i) {
            auto &state = m_states[i];
            std::set<std::string> emptyTransitionsSet;
            for (auto &transition: state.transitions) {

                if (m_transitions[transition].m_from == i && m_transitions[transition].m_inSymbol == inSymbol) {
                    for (auto toInd: m_transitions[transition].m_to) {
                        if (toInd == i && inSymbol == "e") {
                            continue;
                        }
                        emptyTransitionsSet.insert(m_states[toInd].state);
                    }
                }
            }
            std::string emptyTransitions;
            int size = emptyTransitionsSet.size();
            for (const auto &stateName: emptyTransitionsSet) {
                emptyTransitions += stateName + (size-- == 1 ? "" : ",");
            }

            file << ";" << emptyTransitions;
        }
        file << std::endl;
    }

    file.close();
}

void RegexToNFA::ToNFA() {
    auto ToSet = [&](int ind) {
        return std::set<int>{ind};
    };
    auto AddTransitionToNew = [&](int from, int to, std::string ch = "e") {
        m_states[from].transitions.insert(m_transitions.size());
        m_states.emplace_back("S" + std::to_string(to), "", ToSet(m_transitions.size()));
        m_transitions.emplace_back(from, ToSet(to), ch);
    };
    auto AddTransitionTo = [&](int from, int to, std::string ch = "e") {
        m_states[from].transitions.insert(m_transitions.size());
        m_transitions.emplace_back(from, ToSet(to), ch);
    };
    m_inSymbols.insert("e");
    m_states.emplace_back("S0", "");
    m_transitions.emplace_back(0, ToSet(0), "e");
    std::stack<int> preBracketStateIndex; // Стэк состояний которые предшествуют скобкам
    std::stack<std::set<int>> stateIndexToBrackets; // Стэк состояний которые нужно привязать к конечному
    int stateIndex = 0; // Актуальное последнеее состояния
    int stateCounter = 0; // Содержит индекс последнего состояния
    bool closeBracket = false;
    bool openBracket = false;

    preBracketStateIndex.emplace(0);
    stateIndexToBrackets.emplace();

    for (char c: m_regularExpression) {
        switch (c) {
            case '(': {
                m_states.emplace_back("S" + std::to_string(++stateCounter), "");
                AddTransitionTo(stateCounter, stateCounter, "e");
                AddTransitionTo(stateIndex, stateCounter, "e");

                stateIndex = stateCounter;
                stateIndexToBrackets.emplace();
                preBracketStateIndex.emplace(stateCounter);
                openBracket = true;
                closeBracket = false;
                break;
            }
            case ')': {
                if (openBracket) {
                    // () = e
                    AddTransitionToNew(stateIndex, ++stateCounter, "e");
                    stateIndex = stateCounter;
                    stateIndexToBrackets.pop();
                    preBracketStateIndex.pop();
                } else {
                    if (closeBracket) {
                        preBracketStateIndex.pop();
                    }
                    m_states.emplace_back("S" + std::to_string(++stateCounter), "", ToSet(m_transitions.size()));
                    AddTransitionTo(stateIndex, stateCounter, "e");
                    if (!stateIndexToBrackets.top().empty()) {
                        for (auto stateInd: stateIndexToBrackets.top()) {
                            AddTransitionTo(stateInd, stateCounter, "e");
                        }
                    }
                    stateIndexToBrackets.pop();
                    stateIndex = stateCounter;
                    closeBracket = true;
                }
                openBracket = false;
                break;
            }
            case '+': {
                if (closeBracket) {
                    AddTransitionToNew(stateIndex, ++stateCounter, "e");
                    const auto transition = m_transitions[*m_states[preBracketStateIndex.top()].transitions.begin()];
                    AddTransitionTo(stateCounter, preBracketStateIndex.top(), transition.m_inSymbol);
                    preBracketStateIndex.pop();
                } else {
                    // Переход из предыдущего в новое состояние
                    // Создание нового состояния и переход в предыдущее по входному символу
                    AddTransitionToNew(stateIndex, ++stateCounter, "e");
                    const auto transition = m_transitions[*m_states[stateIndex].transitions.begin()];
                    AddTransitionTo(stateCounter, stateIndex, transition.m_inSymbol);
                }
                stateIndex = stateCounter;
                openBracket = false;
                closeBracket = false;
                break;
            }
            case '*': {
                if (closeBracket) {
                    AddTransitionToNew(stateIndex, ++stateCounter, "e");
                    const auto transition = m_transitions[*m_states[preBracketStateIndex.top()].transitions.begin()];
                    AddTransitionTo(stateIndex, preBracketStateIndex.top(), transition.m_inSymbol);
                    AddTransitionTo(transition.m_from, stateCounter, "e"); // Переход из пред-предыдущего в новое состояние по e
                    preBracketStateIndex.pop();
                } else {
                    // Переход из предыдущего в новое состояние
                    AddTransitionToNew(stateIndex, ++stateCounter, "e");
                    // Создание нового состояния и переход в предыдущее по входному символу
                    const auto transition = m_transitions[*m_states[stateIndex].transitions.begin()];
                    AddTransitionTo(stateCounter, stateIndex, transition.m_inSymbol);
                    // Переход из пред-предыдущего в новое состояние по e
                    AddTransitionTo(transition.m_from, stateCounter, "e");
                }
                stateIndex = stateCounter;
                openBracket = false;
                closeBracket = false;
                break;
            }
            case '|': {
                if (closeBracket) {
                    preBracketStateIndex.pop();
                }
                stateIndexToBrackets.top().insert(stateIndex);
                stateIndex = preBracketStateIndex.top();
                openBracket = false;
                closeBracket = false;
                break;
            }
            default: {
                if (closeBracket) {
                    preBracketStateIndex.pop();
                }
                m_inSymbols.insert(std::string(1, c));
                AddTransitionToNew(stateIndex, ++stateCounter, std::string(1, c));
                stateIndex = stateCounter;
                openBracket = false;
                closeBracket = false;
                break;
            }
        }
    }
    m_states.emplace_back("S" + std::to_string(++stateCounter), "F");
    if (!stateIndexToBrackets.empty()) {
        stateIndexToBrackets.top().insert(stateIndex);
    }

    if (!stateIndexToBrackets.empty() && !stateIndexToBrackets.top().empty()) {
        for (auto stateInd: stateIndexToBrackets.top()) {
            AddTransitionTo(stateInd, stateCounter, "e");
        }
    }
}
