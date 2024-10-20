#pragma onse

#include <string>
#include <utility>
#include <vector>
#include <unordered_map>
#include <map>
#include <set>

const bool TEST = false;

class MealyGraph;

class MooreTransition {
public:
    int m_from;
    int m_to;
    std::string m_inSymbol;

    MooreTransition(int from, int to, std::string inSymbol)
            : m_from(from), m_to(to), m_inSymbol(std::move(inSymbol)) {}
};

struct MooreState {
    MooreState(std::string nState, std::string nOutSymbol, std::set<int> nTransitions = {}) : state(std::move(nState)),
                                                                                              outSymbol(std::move(
                                                                                                      nOutSymbol)),
                                                                                              transitions(std::move(
                                                                                                      nTransitions)) {}

    MooreState() = default;
    std::string state;
    std::string outSymbol;
    std::set<int> transitions;
};

class MooreGraph {
public:
    std::vector<std::string> m_inSymbols;
    std::vector<MooreState> m_states;
    std::vector<MooreTransition> m_transitions;

    void FillGraphFromCSVFile(const std::string &fileName);
    MealyGraph ToMealyGraph();
    void WriteToCSVFile(const std::string &filename);

private:
    int FindStateByString(const std::string &state);
};
