#include "Recursive.h"


Recursive::Recursive(std::ofstream& _fout)
    : fout(_fout) {
}

Recursive::~Recursive() {
}

void Recursive::solve(const std::vector<std::pair<std::string, std::string>>& token) {
    fout << "Process:\n";
    for (size_t left = 0; left < token.size(); ++left) {
        size_t right = left;
        for (; right < token.size(); ++right)
            if (token[right].first == "SPLIT")
                break;
        if (left == right)
            continue;
        acc = true;
        fout << "Parse  ";
        static std::pair<std::string, std::string> input[100000];
        int cur1 = 0;
        for (size_t i = left; i < right; ++i) {
            fout << token[i].second;
            input[cur1++] = token[i];
        }
        input[cur1++] = std::make_pair("ACC", "$");
        fout << "  :\n";
        try {
            p = &input[0];
            proE();
        } catch (const std::exception& e) {
            fout << e.what() << '\n';
        }
        fout << '\n';

        left = right;
    }
}
void Recursive::currentInput() {
    for (auto i = p; i->first != "ACC"; ++i)
        fout << i->second;
    fout << '\n';
}
void Recursive::error() {
    throw std::runtime_error("syntax error.");
}
void Recursive::accept() {
    if (acc)
        fout << "accept.\n";
    acc = false;
}
void Recursive::proE() {
    fout << "E: ";
    currentInput();
    proT();
    if (p->second == "+" || p->second == "-") {
        ++p;
        proE();
    } else if (p->second == "$")
        accept();
}
void Recursive::proT() {
    fout << "T: ";
    currentInput();
    proF();
    if (p->second == "*" || p->second == "/") {
        ++p;
        proT();
    } else if (p->second == "$")
        accept();
}
void Recursive::proF() {
    fout << "F: ";
    currentInput();
    if (p->second == "(") {
        ++p;
        proE();
        if (p->second == ")")
            ++p;
        else
            error();
    } else if (p->first == "number")
        ++p;
    else
        error();
    if (p->second == "$")
        accept();
}