#pragma onse

#include <string>
#include <fstream>
#include <sstream>
#include "MooreGraph.h"
#include "MealyGraph.h"

void MooreGraph::FillGraphFromCSVFile(const std::string &fileName) {
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
            std::getline(ssOut, outSymbol, ';');
            std::getline(ssStates, state, ';');
            std::getline(ssStates, state, ';');
            m_states.emplace_back(state, outSymbol, std::set<int>());
            while (std::getline(ssOut, outSymbol, ';')) {
                std::getline(ssStates, state, ';');
                if (!state.empty() && state != "\"\"" && !outSymbol.empty() && outSymbol != "\"\"") {
                    m_states.emplace_back(state, outSymbol, std::set<int>());
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
                if (transition == "\"\"" || transition.empty()) {
                    m_transitions.emplace_back(index, -1, inSymbol);
                } else {
                    m_transitions.emplace_back(index, FindStateByString(transition), inSymbol);
                }
                m_states[index].transitions.insert(m_transitions.size() - 1);
                index++;
            }
        }
    }

    file.close();
}

int MooreGraph::FindStateByString(const std::string &state) {
    for (int i = 0; i < m_states.size(); ++i) {
        if (m_states[i].state == state) {
            return i;
        }
    }
    throw std::invalid_argument("State dont exist");
}

MealyGraph MooreGraph::ToMealyGraph() {
    MealyGraph newGraph;
    newGraph.m_inSymbols = m_inSymbols;
    for (auto &state: m_states) {
        newGraph.m_states.emplace_back(state.state, state.transitions);
    }
    for (auto &transition: m_transitions) {
        std::string outSymbol = transition.m_to != -1 ? m_states[transition.m_to].outSymbol : "";
        MealyTransition mealyTransition(transition.m_from, transition.m_to, transition.m_inSymbol, outSymbol);
        newGraph.m_transitions.push_back(mealyTransition);
    }

    return newGraph;
}


void MooreGraph::WriteToCSVFile(const std::string &filename) {
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
                    file << ";" << (m_transitions[transition].m_to != -1
                                    ? m_states[m_transitions[transition].m_to].state : "");

                    break;
                }
            }
        }
        file << std::endl;
    }

    file.close();
}