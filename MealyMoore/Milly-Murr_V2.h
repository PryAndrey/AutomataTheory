#pragma once

#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <tuple>
#include <iomanip>
#include <fstream>
#include <map>
#include <set>
#include <queue>

using Table = std::vector<std::vector<std::string>>;

class MooreVer {
public:
    explicit MooreVer(std::string state = "", std::string outSymbol = "")
            : m_state(std::move(state)), m_outSymbol(std::move(outSymbol)) {}

    ~MooreVer() {
        for (auto &pair: m_transition) {
            delete pair.second;
        }
    }


    void SetState(std::string state) {
        m_state = std::move(state);
    }

    [[nodiscard]] std::string GetState() const {
        return m_state;
    }

    void SetOutSymbol(std::string outSymbol) {
        m_outSymbol = std::move(outSymbol);
    }

    [[nodiscard]] std::string GetOutSymbol() const {
        return m_outSymbol;
    }

    void AddTransition(const std::string &inSymbol, MooreVer *nextState) {
        m_transition[inSymbol] = nextState;
    }

    void RemoveTransition(const std::string &inSymbol) {
        auto transition = GetTransitionByKey(inSymbol);
        if (!transition) {
            return;
        }
        delete transition->second;
        m_transition.erase(transition->first);
    }

    [[nodiscard]] std::map<std::string, MooreVer *> GetTransitions() const {
        return m_transition;
    }

    [[nodiscard]] std::optional<std::pair<std::string, MooreVer *>>
    GetTransitionByKey(const std::string &inSymbol) const {
        auto it = m_transition.find(inSymbol);
        if (it != m_transition.end()) {
            return *it;
        }
        return std::nullopt;
    }

    std::string m_oldState;
private:

    std::string m_state;
    std::string m_outSymbol;
    std::map<std::string, MooreVer *> m_transition{};
};

class MealyVer {
public:
    explicit MealyVer(std::string state = "")
            : m_state(std::move(state)) {}

    ~MealyVer() {
        for (auto &pair: m_transition) {
            delete pair.second.second;
        }
    }

    void SetState(std::string state) {
        m_state = std::move(state);
    }

    [[nodiscard]] std::string GetState() const {
        return m_state;
    }

    void AddTransition(const std::string &inSymbol, const std::string &outSymbol, MealyVer *nextState) {
        m_transition[inSymbol] = std::pair(outSymbol, nextState);
    }

    void RemoveTransition(const std::string &inSymbol) {
        auto transition = GetTransitionByKey(inSymbol);
        if (!transition) {
            return;
        }
        delete transition->second.second;
        m_transition.erase(transition->first);
    }

    [[nodiscard]] std::map<std::string, std::pair<std::string, MealyVer *>> GetTransitions() const {
        return m_transition;
    }

    [[nodiscard]] std::optional<std::pair<std::string, std::pair<std::string, MealyVer *>>>
    GetTransitionByKey(const std::string &inSymbol) const {
        auto it = m_transition.find(inSymbol);
        if (it != m_transition.end()) {
            return *it;
        }
        return std::nullopt;
    }

    std::string m_oldState;
private:

    std::string m_state;
    //           <inSymbol> -> {outSymbol, MealyVer};
    std::map<std::string, std::pair<std::string, MealyVer *>> m_transition{};
};

std::pair<std::string, std::string> SplitMealyState(const std::string &input);

Table ReadMealyToTable(const std::string &filename);

void WriteTableCout(const Table &table, int space = 5);

void WriteMealyFromTable(const std::string &filename, Table &mealyTable);

Table MealyGraphToTable(MealyVer *startVer) ;

MealyVer *TableToMealyGraph(const Table &table);

void RenameMooreGraphStates(MooreVer *startVer, const std::string &symbol);


void WriteMooreFromTable(const std::string &filename, Table &mooreTable) ;

MooreVer *TableToMooreGraph(const Table &table) ;

Table MooreGraphToTable(MooreVer *startVer) ;

Table ReadMooreToTable(const std::string &filename);


MooreVer * MealyToMoore(MealyVer *startMealyVer);

MealyVer *MooreToMealy(MooreVer *startMooreVer) ;
