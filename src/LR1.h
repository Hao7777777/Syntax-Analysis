#pragma once

#include "grammar.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

class LR1 : public Grammar {
private:
    enum class Action { REDUCE,
        SHIFT,
        ACCEPT };
    std::unordered_map<std::string, std::pair<Action, size_t>> table[10000];
    std::unordered_set<std::string> goTo, action;

    struct item {
        std::string nonTerminal;
        std::vector<std::string> statement;
        std::string lookahead;
        size_t dotPos;
        item(std::string _nonTerminal, std::vector<std::string> _statement, std::string _lookahead, size_t _dotPos)
            : nonTerminal(_nonTerminal), statement(_statement), lookahead(_lookahead), dotPos(_dotPos) {
        }
        item(const item& others) {
            nonTerminal = others.nonTerminal;
            statement = others.statement;
            lookahead = others.lookahead;
            dotPos = others.dotPos;
        }
        bool operator==(const item& others) const {
            return nonTerminal == others.nonTerminal &&
                   statement == others.statement &&
                   lookahead == others.lookahead &&
                   dotPos == others.dotPos;
        }
    };
    struct itemHasher {
        size_t operator()(const item& self) const {
            return std::hash<std::string>()(self.nonTerminal) << 16 + std::hash<std::string>()(self.statement.front()) + self.dotPos;
        }
    };
    using itemSet = std::unordered_set<item, itemHasher>;
    std::vector<itemSet> itemSets;

    void addStart();

public:
    using Grammar::Grammar;
    // LR1(std::ifstream& fin);
    ~LR1();
    void work();
    void solve(const std::vector<std::pair<std::string, std::string>>& token, std::ofstream& fout);
    void getTable();
    void printDFA(std::ofstream& fout);
    void printTable(std::ofstream& fout);
};
