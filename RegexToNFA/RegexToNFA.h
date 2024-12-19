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

    void ToNFA();

    void WriteToCSVFile(const std::string &filename);
};