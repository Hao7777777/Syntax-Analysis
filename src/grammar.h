#pragma once

#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Grammar {
private:
protected:
    bool isTerminal(const std::string& symbol);
    void getSymbols();
    void getFirst();
    void getFollow();
    void eliminateLeftRecursion();
    void eliminateLeftCommonFactor();
    std::unordered_set<std::string> findFirst(const std::vector<std::string>& statement);

    const std::string epsilon = "epsilon";
    const std::string dollar = "$";
    const std::vector<std::string> synch = { "synch" };
    std::string startSymbol;
    std::unordered_set<std::string> nonTerminalSymbols;
    std::unordered_set<std::string> terminalSymbols;
    std::unordered_map<std::string, std::vector<std::vector<std::string>>> rules;
    std::unordered_map<std::string, std::unordered_set<std::string>> first;
    std::unordered_map<std::string, std::unordered_set<std::string>> follow;

public:
    Grammar(std::ifstream& fin);
    ~Grammar();
    void printRule(std::ofstream& fout);
    void printFirst();
    void printFollow();
};
