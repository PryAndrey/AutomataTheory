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
#include "GrammarReader.h"

GrammarReader::Pairs GrammarReader::ParseGrammarTransition(const std::string &grammarTransition) const {
    const std::regex pattern(m_type == GrammarType::LG ? GRAMMAR_TRANSITION_L : GRAMMAR_TRANSITION_R);
    std::sregex_iterator begin(grammarTransition.begin(), grammarTransition.end(), pattern);
    std::sregex_iterator end;

    Pairs parsedPairs;
    const char divider = m_type == GrammarType::LG ? '>' : '<';
    for (std::sregex_iterator it = begin; it != end; ++it) {
        std::string match = it->str();
        match.erase(std::remove(match.begin(), match.end(), ' '), match.end());
        match.erase(std::remove(match.begin(), match.end(), '|'), match.end());

        size_t pos = match.find(divider);
        if (pos != std::string::npos) {
            if (m_type == GrammarType::LG) {
                std::string variable = match.substr(0, pos + 1);
                std::string value = match.substr(pos + 1);
                parsedPairs.emplace_back(variable, value);
            }
            if (m_type == GrammarType::RG) {
                std::string value = match.substr(0, pos);
                std::string variable = match.substr(pos);
                parsedPairs.emplace_back(variable, value);
            }
        } else {
            parsedPairs.emplace_back("", match);
        }
    }
    return parsedPairs;
}

void GrammarReader::ReadGrammarRules(Pairs &grammarVector) {
    int stateIndex = 0;
    std::unordered_map<std::string, int> stateMap;
    for (const auto &[grammarKey, grammarTransition]: grammarVector) {
        if (m_stateConvert.find(grammarKey) == m_stateConvert.end()) {
            m_stateConvert[grammarKey] = "q" + std::to_string(stateIndex);
            m_stateConvertR["q" + std::to_string(stateIndex++)] = grammarKey;
            stateMap[grammarKey] = stateIndex - 1;
            m_states.emplace_back(grammarKey);
        }
        Pairs parsedPairs = ParseGrammarTransition(grammarTransition);
        for (const auto &[to, in]: parsedPairs) {
            m_inSymbols.insert(in);
            if (!to.empty() && m_stateConvert.find(to) == m_stateConvert.end()) {
                m_stateConvert[to] = "q" + std::to_string(stateIndex);
                m_stateConvertR["q" + std::to_string(stateIndex++)] = to;
                stateMap[to] = stateIndex - 1;
                m_states.emplace_back(to);
            }
            int toInd = !to.empty() ? stateMap[to] : -1;
            if (m_type == GrammarType::RG || m_type == GrammarType::N) {
                m_transitions.emplace_back(stateMap[grammarKey], toInd, in, toInd == -1);
                m_states[stateMap[grammarKey]].transitions.insert(m_transitions.size() - 1);
            } else if (m_type == GrammarType::LG) {
                m_transitions.emplace_back(toInd, stateMap[grammarKey], in, toInd == -1);
                if (toInd == -1)
                    m_finishState.transitions.insert(m_transitions.size() - 1);
                else
                    m_states[toInd].transitions.insert(m_transitions.size() - 1);
            }
        }
    }
    if (m_states.size() != m_stateConvert.size()) {
        throw std::runtime_error("Lost Non terminate state");
    }
}

void GrammarReader::RegexRead(Pairs &grammarVector, const std::string &regularExpression) {
    std::smatch matches;
    if (std::regex_match(regularExpression, matches, std::regex(GRAMMAR_RULES, REG_SETTINGS))) {
        grammarVector.emplace_back(matches[1], matches[2]);
        return;
    }
    if (std::regex_match(regularExpression, matches, std::regex(GRAMMAR_RULES_L, REG_SETTINGS))) {
        grammarVector.emplace_back(matches[1], matches[2]);
        if (m_type == GrammarType::N) {
            m_type = GrammarType::LG;
        }
        if (m_type != GrammarType::LG) {
            throw std::logic_error("Wrong input format, must be left grammar");
        }
        return;
    }
    if (std::regex_match(regularExpression, matches, std::regex(GRAMMAR_RULES_R, REG_SETTINGS))) {
        grammarVector.emplace_back(matches[1], matches[2]);
        if (m_type == GrammarType::N) {
            m_type = GrammarType::RG;
        }
        if (m_type != GrammarType::RG) {
            throw std::logic_error("Wrong input format, must be right grammar");
        }
        return;
    }

    throw std::logic_error("Wrong input format");

}

void GrammarReader::ReadFile(const std::string &fileName) {
    std::ifstream inFile(fileName);
    if (!inFile.is_open()) {
        throw std::runtime_error("Could not open file: " + fileName);
    }

    Pairs grammarVector;
    std::string regularExpression;
    std::string line;
    while (std::getline(inFile, line)) {
        if (line.find("->") == std::string::npos) {
            regularExpression += line;
            continue;
        }
        if (regularExpression.empty()) {
            regularExpression = line;
            continue;
        }
        regularExpression = std::regex_replace(regularExpression, std::regex("\\s+"), " ");
        RegexRead(grammarVector, regularExpression);
        regularExpression = line;
    }
    regularExpression = std::regex_replace(regularExpression, std::regex("\\s+"), " ");
    RegexRead(grammarVector, regularExpression);
    ReadGrammarRules(grammarVector);
};

void GrammarReader::WriteMooreToFile(const std::string &fileName) {
    std::ofstream file(fileName);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + fileName);
    }

    for (auto &transition: m_transitions) {
        std::cout << (transition.from != -1 ? m_states[transition.from].name : "{ }")
                  << " -" << transition.in << "-> "
                  << (transition.to != -1 ? m_states[transition.to].name : "{ }")
                  << std::endl;
    }

    /// Out symbols
    for (auto &state: m_states) {
        file << ";";
    }
    file << ";" << "F" << std::endl;

    /// States names
    if (m_type == GrammarType::RG || m_type == GrammarType::N) {
        for (auto &state: m_states) {
            file << ";" << m_stateConvert[state.name];
        }
        file << ";" << m_finishState.name << std::endl;
    } else {
        file << ";" << m_finishState.name;
        for (auto state = m_states.rbegin(); state != m_states.rend(); ++state) {
            file << ";" << m_stateConvert[state->name];
        }
        file << std::endl;
    }
    /// Transition table
    for (const auto &inSymbol: m_inSymbols) {
        file << inSymbol;
        if (m_type == GrammarType::LG) {
            std::string finishEmptyTransitions;
            for (auto &transition: m_finishState.transitions) {
                if (m_transitions[transition].in == inSymbol) {
                    const std::string toState = m_transitions[transition].to == -1
                                                ? ""
                                                : m_stateConvert[m_states[m_transitions[transition].to].name];
                    finishEmptyTransitions += (finishEmptyTransitions.empty() ? "" : ",") + toState;
                }
            }
            file << ";" << finishEmptyTransitions;
        }
        auto setTransitions = [&](MooreState &state) {
            std::string emptyTransitions;
            for (auto &transition: state.transitions) {
                if (m_transitions[transition].in == inSymbol) {
                    const std::string toState = m_transitions[transition].to == -1
                                                ? m_finishState.name
                                                : m_stateConvert[m_states[m_transitions[transition].to].name];
                    emptyTransitions += (emptyTransitions.empty() ? "" : ",") + toState;
                }
            }
            file << ";" << emptyTransitions;
        };

        // Write transitions
        if (m_type == GrammarType::RG || m_type == GrammarType::N) {
            for (auto state: m_states) {
                setTransitions(state);
            }
        } else {
            for (auto state = m_states.rbegin(); state != m_states.rend(); ++state) {
                setTransitions(*state);
            }
        }
        if (m_type == GrammarType::RG || m_type == GrammarType::N) {
            file << ";" << std::endl;
        } else
            file << std::endl;

    }
    file.close();
};
