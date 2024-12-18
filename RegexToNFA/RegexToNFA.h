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
    MooreTransition(int from, std::set<int> to, std::string inSymbol)
            : m_from(from), m_to(std::move(to)), m_inSymbol(std::move(inSymbol)) {}

    int m_from;
    std::set<int> m_to;
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
    std::set<int> transitions{};
};

class RegexToNFA {
public:
    std::set<std::string> m_inSymbols;
    std::string m_regularExpression;
    std::vector<MooreState> m_states;
    std::unordered_map<std::string, int> m_statesMap;
    std::vector<MooreTransition> m_transitions;

    explicit RegexToNFA(std::string &regularExpression) {
        m_regularExpression = regularExpression;
        ToNFA();
    }

    void ToNFA() {
        auto ToSet = [&](int ind) {
            return std::set<int>{ind};
        };
        auto AddTransitionToNew = [&](int from, int to, std::string ch = "e") {
            m_states[from].transitions.insert(m_transitions.size());
            m_states.emplace_back("S" + std::to_string(to), "",
                                  ToSet(m_transitions.size()));
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
        int stateIndex; // Актуальное последнеее состояния
        std::stack<std::set<int>> stateIndexToBrackets; // Стэк состояний которые нужно привязать к конечному
        bool closeBracket = false;
        bool openBracket = false;

        preBracketStateIndex.emplace(0);
        stateIndexToBrackets.emplace();
        stateIndex = 0;
        int stateCounter = 0; // содержит индекс последнего состояния

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
                        auto lastStateInd = stateIndex;

                        // Переход из предыдущего в новое состояние
                        // Создание нового состояния и переход в предыдущее по входному символу
                        AddTransitionToNew(lastStateInd, ++stateCounter, "e");
                        const auto transition = m_transitions[*m_states[lastStateInd].transitions.begin()];
                        AddTransitionTo(stateCounter, lastStateInd, transition.m_inSymbol);
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
                        auto lastStateInd = stateIndex;
                        // Переход из предыдущего в новое состояние
                        AddTransitionToNew(lastStateInd, ++stateCounter, "e");
                        // Создание нового состояния и переход в предыдущее по входному символу
                        const auto transition = m_transitions[*m_states[lastStateInd].transitions.begin()];
                        AddTransitionTo(stateCounter, lastStateInd, transition.m_inSymbol);
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

    void WriteToCSVFile(const std::string &filename);

};

// TODO Проверить (...)r*
// TODO () = e