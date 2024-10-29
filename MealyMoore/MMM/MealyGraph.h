#pragma onse

#include <string>
#include <utility>
#include <vector>
#include <unordered_map>
#include <map>
#include <set>

class MooreGraph;

class MealyTransition {
public:
    int m_from;
    int m_to;
    std::string m_inSymbol;
    std::string m_outSymbol;

    MealyTransition(int from, int to, std::string inSymbol, std::string outSymbol)
            : m_from(from), m_to(to), m_inSymbol(std::move(inSymbol)),
              m_outSymbol(std::move(outSymbol)) {}
};

class MealyGraph {
public:
    std::vector<std::string> m_inSymbols;
    std::vector<std::pair<std::string, std::set<int>>> m_states;
    std::vector<MealyTransition> m_transitions;

    void FillGraphFromCSVFile(const std::string &fileName);

    MooreGraph ToMooreGraph();

    void WriteToCSVFile(const std::string &filename);

    void Minimize();

private:
    size_t UniqueNames(std::map<std::string, std::map<std::string, std::set<int>>> &temp);

    void TrimStates();

    static std::pair<std::string, std::string> SplitMealyState(const std::string &input);

    int FindStateByString(const std::string &state);
};
