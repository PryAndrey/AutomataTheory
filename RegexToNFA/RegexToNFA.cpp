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
