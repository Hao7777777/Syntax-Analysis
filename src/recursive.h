#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

class Recursive {
private:
    void proE();
    void proT();
    void proF();
    void error();
    void currentInput();
    void accept();
    std::pair<std::string, std::string>* p;
    std::ofstream& fout;
    bool acc;

public:
    Recursive(std::ofstream& _fout);
    ~Recursive();
    void solve(const std::vector<std::pair<std::string, std::string>>& token);
};
