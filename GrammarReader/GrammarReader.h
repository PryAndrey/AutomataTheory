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


const std::string ALPHABET = "абвгдеёжзийклмнопрстуфхцчшщъыьэюяАБВГДЕЁЖЗИКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ";
const std::string N_TERM = R"(<[\w)" + ALPHABET + R"(]+>)";
const std::string TERM = R"((?:[\w)" + ALPHABET + R"(]|ε))";
const std::string TO = R"(\s*->\s*)";
const std::string OR = R"(\s*\|\s*)";
const auto REG_SETTINGS = std::regex_constants::ECMAScript;

const std::string GRAMMAR_TRANSITION_L = R"(\s*(<[\w)" + ALPHABET + R"(]+>\s+(?:[\w)" + ALPHABET + R"(]|ε))"
                                         + R"(|)" + R"(ε)" + R"(|)"
                                         + R"([\w)" + ALPHABET + R"(])\s*\|?)";
const std::string GRAMMAR_RULES_L = R"(^\s*()" + N_TERM + R"())"
                                    + TO + R"(((?:)" + N_TERM + R"(\s+)?)" + TERM + R"((?:)"
                                    + OR + R"((?:)" + N_TERM + R"(\s+)?)" + TERM + R"()*)\s*$)";

const std::string GRAMMAR_RULES = R"(^\s*()" + N_TERM + R"())"
                                  + TO + R"(()" + TERM + R"((?:)"
                                  + OR + TERM + R"()*)\s*$)";

const std::string GRAMMAR_TRANSITION_R = R"(\s*((?:[\w)" + ALPHABET + R"(]|ε)\s+<[\w)" + ALPHABET + R"(]+>)"
                                         + R"(|)" + R"(ε)" + R"(|)"
                                         + R"([\w)" + ALPHABET + R"(])\s*\|?)";
const std::string GRAMMAR_RULES_R = R"(^\s*()" + N_TERM + R"())"
                                    + TO + R"(()" + TERM + R"((?:\s+)" + N_TERM + R"()?(?:)"
                                    + OR + TERM + R"((?:\s+)" + N_TERM + R"()?)*)\s*$)";

//      ^\s*(<[\w]+>)\s*->\s*((?:[\w]|ε)(?:\s+<[\w]+>)?(?:\s*\|\s*(?:[\w]|ε)(?:\s+<[\w]+>)?)*)\s*$
//      ^\s*<([\w]+)>\s*->\s*((?:<[\w]+>\s+)?(?:[\w]|ε)(?:\s*\|\s*(?:<[\w]+>\s+)?(?:[\w]|ε))*)\s*$

enum class GrammarType {
    LG, RG, N
};

class GrammarReader {
public:
    using Pairs = std::vector<std::pair<std::string, std::string>>;

    struct MooreState {
        MooreState(std::string n_name, std::set<int> n_Transitions = {}) : name(std::move(n_name)),
                                                                           transitions(std::move(
                                                                                   n_Transitions)) {}

        MooreState() = default;

        std::string name;
        std::set<int> transitions;
    };

    struct Transition {
        Transition(int from, int to, std::string in, bool out = false)
                : from(from), to(to), in(std::move(in)) {}

        int from;
        int to;
        std::string in;
    };

    GrammarType m_type = GrammarType::N;
    std::unordered_map<std::string, std::string> m_stateConvert;
    std::unordered_map<std::string, std::string> m_stateConvertR;
    std::set<std::string> m_inSymbols;
    std::vector<MooreState> m_states;
    MooreState m_finishState = {"qf", {}};
    std::vector<Transition> m_transitions;

    Pairs ParseGrammarTransition(const std::string &grammarTransition) const;

    void ReadGrammarRules(Pairs &grammarVector);

    void RegexRead(Pairs &grammarVector, const std::string &regularExpression);

    void ReadFile(const std::string &fileName);

    void WriteMooreToFile(const std::string &fileName);
};
