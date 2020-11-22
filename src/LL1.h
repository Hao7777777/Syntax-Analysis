#pragma once

#include "grammar.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class LL1 : public Grammar {
private:
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> table;
    std::set<std::string> row;
    std::set<std::string> column;
    bool compare(const std::string& A, const std::pair<std::string, std::string>& B);
public:
    using Grammar::Grammar;
    // LL1(std::ifstream& fin);
    ~LL1();
    void work();
    void solve(const std::vector<std::pair<std::string, std::string>>& token, std::ofstream& fout);
    void getTable();
    void printTable(std::ofstream& fout);
};
