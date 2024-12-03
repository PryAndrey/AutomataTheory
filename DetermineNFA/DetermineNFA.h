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
    std::set<int> transitions;
};

struct [[maybe_unused]] MooreChain {

    MooreChain() = default;

    MooreChain(std::string nState, int nStateInd, std::set<int> nChainedStates = {})
            : state(std::move(nState)),
              stateInd(nStateInd),
              chainedStates(std::move(nChainedStates)) {}

    std::string state;
    int stateInd{};
    std::set<int> chainedStates;
};

class DetermineNFA {
public:
    std::vector<std::string> m_inSymbols;
    std::vector<MooreState> m_states;
    std::unordered_map<std::string, int> m_statesMap;
    std::vector<MooreChain> m_chainedStates;
    std::vector<MooreTransition> m_transitions;

    void ReadFromCSVFile(const std::string &fileName);

    void FindChain();

    void ConvertToDFA();

    void ToDFA() {
        FindChain();// Находим последовательности через ε
        m_inSymbols.erase(std::find(m_inSymbols.begin(), m_inSymbols.end(), "ε"));
        ConvertToDFA();
    }

    void WriteToCSVFile(const std::string &filename);
};