#include "LL1.h"


LL1::~LL1() {
}
void LL1::work() {
    eliminateLeftRecursion();
    eliminateLeftCommonFactor();
    getSymbols();
    getFirst();
    getFollow();
    // printFirst();
    // printFollow();
    getTable();
}
void LL1::getTable() {
    for (const auto& rule : rules) {
        std::string nonTerminal = rule.first;
        row.emplace(nonTerminal);
        for (const auto& statement : rule.second) {
            auto _first = findFirst(statement);
            bool flag = false;
            for (const auto& symbol : _first) {
                if (symbol == epsilon)
                    flag = true;
                else {
                    table[nonTerminal][symbol] = statement;
                    column.emplace(symbol);
                }
            }
            if (flag) {
                auto& _follow = follow[nonTerminal];
                for (const auto& symbol : _follow) {
                    table[nonTerminal][symbol] = statement;
                    column.emplace(symbol);
                }
            }
        }
    }
    for (const auto& nonTerminal : row) {
        const auto& _follow = follow[nonTerminal];
        for (const auto& symbol : _follow) {
            if (table.find(nonTerminal) == table.end() || table[nonTerminal].find(symbol) == table[nonTerminal].end()) {
                column.emplace(symbol);
                table[nonTerminal][symbol] = synch;
            }
        }
    }
}
void LL1::printTable(std::ofstream& fout) {
    fout << "LL1 Table:\n";
    const int len = 10;
    int size = column.size();
    fout << std::setw(len) << std::setfill(' ') << ' ';
    for (const auto& symbol : column)
        fout << std::setw(len) << std::setfill(' ') << symbol;
    fout << '\n';
    for (const auto& nonTerminal : row) {
        fout << std::setw(len) << std::setfill(' ') << nonTerminal << "|";
        if (table.find(nonTerminal) == table.end())
            for (int i = 0; i < size; ++i)
                fout << std::setw(len) << std::setfill(' ') << ' ';
        else
            for (const auto& symbol : column) {
                if (table[nonTerminal].find(symbol) == table[nonTerminal].end())
                    fout << std::setw(len) << std::setfill(' ') << ' ';
                else {
                    int _size = 0;
                    for (const auto& symbol_ : table[nonTerminal][symbol])
                        _size += symbol_.size();
                    for (int i = 0; i < len - _size; ++i)
                        fout << ' ';
                    for (const auto& symbol_ : table[nonTerminal][symbol])
                        fout << symbol_;
                }
            }
        fout << '\n';
    }
    fout << '\n';
}
void LL1::solve(const std::vector<std::pair<std::string, std::string>>& token, std::ofstream& fout) {
    fout << "Process:\n";
    const int len = 35;
    for (size_t left = 0; left < token.size(); ++left) {
        size_t right = left;
        for (; right < token.size(); ++right)
            if (token[right].first == "SPLIT")
                break;
        if (left == right)
            continue;
        fout << "Parse  ";
        static std::pair<std::string, std::string> input[100000];
        static std::string statement[100000];
        int cur1 = 0, cur2 = 0;
        for (size_t i = left; i < right; ++i) {
            fout << token[i].second;
            input[cur1++] = token[i];
        }
        fout << "  :\n";
        input[cur1++] = std::make_pair("ACC", "$");
        statement[cur2++] = "$";
        statement[cur2++] = startSymbol;
        auto p1 = &input[0];
        auto p2 = &statement[cur2 - 1];
        do {
            std::string out = "";
            for (int i = 0; i < cur2; ++i)
                out += statement[i];
            fout << out << std::setw(len - out.size()) << std::setfill(' ') << '|';
            out = "";
            for (int i = p1 - &input[0]; i < cur1; ++i)
                out += input[i].second;
            fout << std::setw(len) << std::setfill(' ') << out << '|';
            out = "";
            if (isTerminal(*p2) || *p2 == "$") {
                if (compare(*p2, *p1)) {
                    --p2;
                    --cur2;
                    ++p1;
                } else {
                    out = "ERROR: pop " + *p2;
                    --p2;
                    --cur2;
                }
            } else {
                std::string symbol = p1->first == "number" ? "num" : p1->second;
                if (table.find(*p2) != table.end() && table[*p2].find(symbol) != table[*p2].end()) {
                    auto rule = table[*p2][symbol];
                    if (rule.front() == "synch") {
                        out = "ERROR: pop " + *p2;
                        --p2;
                        --cur2;
                    } else {
                        out = *p2 + "->";
                        for (const auto& symbol_ : rule)
                            out += symbol_;
                        --cur2;
                        --p2;
                        if (rule.front() != epsilon)
                            for (int i = (int)rule.size() - 1; i >= 0; --i) {
                                statement[cur2++] = rule[i];
                                ++p2;
                            }
                    }
                } else {
                    out = "ERROR: skip " + p1->second;
                    ++p1;
                }
            }
            fout << out << std::setw(len - out.size()) << std::setfill(' ') << '\n';
        } while (*p2 != "$" && (p1 - &input[0] != cur1));
        if (*p2 == "$" && p1->second == "$") {
            fout << "$" << std::setw(len - 1) << std::setfill(' ') << '|';
            fout << std::setw(len) << std::setfill(' ') << "$" << '|';
            fout << "ACCEPT" << std::setw(len - 6) << std::setfill(' ') << '\n';
        }
        left = right;
        fout << '\n';
    }
}
bool LL1::compare(const std::string& A, const std::pair<std::string, std::string>& B) {
    if (B.first == "operator") {
        return A == B.second;
    } else if (B.first == "number") {
        return A == "num";
    } else if (B.first == "ACC") {
        return B.second == "$";
    }
    std::cerr << A << ' ' << B.first << ' ' << B.second << '\n';
    throw std::runtime_error("Lexical Error.");
}