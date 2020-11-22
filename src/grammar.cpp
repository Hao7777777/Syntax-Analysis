#include "grammar.h"
void Grammar::getSymbols() {
    for (const auto& rule : rules)
        nonTerminalSymbols.emplace(rule.first);
    for (const auto& rule : rules)
        for (const auto& statement : rule.second)
            for (const auto& symbol : statement)
                if (isTerminal(symbol))
                    terminalSymbols.emplace(symbol);
}
Grammar::Grammar(std::ifstream& fin) {
    std::string s;
    auto split = [](std::string& s, const std::string& pattern) -> std::string {
        std::string res;
        size_t pos = 0;
        if ((pos = s.find(pattern)) != std::string::npos) {
            res = s.substr(0, pos);
            while (res.back() == ' ')
                res.pop_back();
            pos += pattern.size();
            while (s[pos] == ' ')
                ++pos;
            s.erase(0, pos);
            return res;
        } else {
            res = s;
            s = "";
            return res;
        }
    };
    unsigned int row = 0;
    while (getline(fin, s)) {
        if (s.empty())
            continue;
        ++row;
        auto nonTerminal = split(s, "->");
        if (nonTerminal.find(' ') != std::string::npos)
            throw std::runtime_error("Row " + std::to_string(row) + ": Too much symbols in the left of rule in line.");
        if (s.empty())
            throw std::runtime_error("Row " + std::to_string(row) + " Illegal production rule in line.");
        if (row == 1)
            startSymbol = nonTerminal;
        nonTerminalSymbols.emplace(nonTerminal);
        while (!s.empty()) {
            auto ss = split(s, "|");
            std::vector<std::string> statements;
            while (!ss.empty()) {
                auto symbol = split(ss, " ");
                if (!symbol.empty())
                    statements.emplace_back(symbol);
            }
            rules[nonTerminal].emplace_back(statements);
        }
    }
}
Grammar::~Grammar() {
}

bool Grammar::isTerminal(const std::string& symbol) {
    return nonTerminalSymbols.find(symbol) == nonTerminalSymbols.end();
}
void Grammar::getFirst() {
    for (const auto& symbol : terminalSymbols)
        first[symbol].emplace(symbol);
    first[epsilon].emplace(epsilon);
    bool flag = true;
    while (flag) {
        flag = false;
        for (const auto& rule : rules) {
            auto symbolA = rule.first;
            auto ruleA = rule.second;
            size_t size = first[symbolA].size();
            auto putFirst = [&](const std::string& symbolA, const std::string& symbolB) -> void {
                for (const auto& symbol : first[symbolB])
                    if (symbol != epsilon)
                        first[symbolA].emplace(symbol);
            };
            std::function<void(std::vector<std::string>::iterator, std::vector<std::string>::iterator)> putStatement = [&](std::vector<std::string>::iterator symbol, std::vector<std::string>::iterator end) -> void {
                if (symbol == end) {
                    first[symbolA].emplace(epsilon);
                    return;
                }
                std::string symbolB = *symbol;
                if (symbolB == epsilon) {
                    first[symbolA].emplace(epsilon);
                } else if (isTerminal(symbolB)) {
                    first[symbolA].emplace(symbolB);
                } else {
                    putFirst(symbolA, symbolB);
                    if (first[symbolB].find(epsilon) != first[symbolB].end()) {
                        ++symbol;
                        putStatement(symbol, end);
                    }
                }
            };
            for (auto& statementA : ruleA)
                putStatement(statementA.begin(), statementA.end());
            if (first[symbolA].size() > size)
                flag = true;
        }
    }
}
void Grammar::getFollow() {
    follow[startSymbol].emplace(dollar);
    bool flag = true;
    while (flag) {
        flag = false;
        for (const auto& rule : rules) {
            auto symbolA = rule.first;
            auto ruleA = rule.second;
            auto putFirst = [&](const std::string& symbolA, const std::string& symbolB) -> void {
                for (const auto& symbol : first[symbolB])
                    if (symbol != epsilon)
                        follow[symbolA].emplace(symbol);
            };
            auto putFollow = [&](const std::string& symbolA, const std::string& symbolB) -> void {
                for (const auto& symbol : follow[symbolB])
                    follow[symbolA].emplace(symbol);
            };
            auto hasEpsilon = [&](const std::string& symbol) -> bool {
                return symbol == epsilon || first[symbol].find(epsilon) != first[symbol].end();
            };
            for (const auto& statementA : ruleA) {
                putFollow(statementA.back(), symbolA);
                bool flag1 = true;
                for (int i = statementA.size() - 2; i >= 0; --i) {
                    auto symbolB = statementA[i];
                    auto symbolC = statementA[i + 1];
                    size_t size = follow[symbolB].size();
                    if (!isTerminal(symbolB)) {
                        putFirst(symbolB, symbolC);
                        flag1 &= hasEpsilon(symbolC);
                        if (flag1)
                            putFollow(symbolB, symbolA);
                    }
                    if (follow[symbolB].size() > size)
                        flag = true;
                }
            }
        }
    }
}
void Grammar::printFirst() {
    getSymbols();
    for (const auto& symbol : nonTerminalSymbols) {
        std::cerr << "First[ " << symbol << " ] = { ";
        for (const auto& symbol_ : first[symbol]) {
            std::cerr << symbol_ << ' ';
        }
        std::cerr << "}\n";
    }
}
void Grammar::printFollow() {
    getSymbols();
    for (const auto& symbol : nonTerminalSymbols) {
        std::cerr << "Follow[ " << symbol << " ] = { ";
        for (const auto& symbol_ : follow[symbol]) {
            std::cerr << symbol_ << ' ';
        }
        std::cerr << "}\n";
    }
}
void Grammar::eliminateLeftRecursion() {
    auto topoSort = [&](const std::string& startSymbol) -> std::vector<std::string> {
        std::vector<std::string> res{ startSymbol };
        size_t i = 0;
        bool flag = true;
        while (flag) {
            flag = false;
            size_t size = rules.size();
            for (; i < size; ++i)
                for (const auto& [lhs, rhs] : rules) {
                    if (lhs != res[i])
                        continue;
                    for (const auto& statement : rhs)
                        for (const auto& symbol : statement) {
                            bool has = true;
                            if (isTerminal(symbol))
                                has = false;
                            for (size_t j = 0; has && j < res.size(); ++j)
                                if (symbol == res[j])
                                    has = false;
                            if (has)
                                res.emplace_back(symbol);
                        }
                }
            if (size < rules.size())
                flag = true;
        }
        return res;
    };
    std::vector<std::string> nonTerminal = topoSort(startSymbol);
    for (const auto& symbolA : nonTerminal) {
        for (const auto& symbolB : nonTerminal) {
            if (symbolA == symbolB)
                break;
            std::vector<std::vector<std::string>> tmp;
            for (auto statementA = rules[symbolA].begin(); statementA != rules[symbolA].end();) {
                if (statementA->front() != symbolB) {
                    ++statementA;
                    continue;
                }

                for (const auto& statementB : rules[symbolB]) {
                    std::vector<std::string> statementA_ = statementB;
                    for (size_t i = 1; i < statementA->size(); ++i)
                        statementA_.emplace_back((*statementA)[i]);
                    tmp.emplace_back(statementA_);
                }
                statementA = rules[symbolA].erase(statementA);
            }
            for (const auto& statement : tmp)
                rules[symbolA].emplace_back(statement);
        }
        auto isLeftRecursion = [this](const std::string& symbol) -> bool {
            for (const auto& statement : rules[symbol])
                if (statement.front() == symbol)
                    return true;
            return false;
        };
        if (isLeftRecursion(symbolA)) {
            auto symbolA_ = symbolA + "'";
            for (auto statementA = rules[symbolA].begin(); statementA != rules[symbolA].end();) {
                if (statementA->front() != symbolA) {
                    ++statementA;
                    continue;
                }
                statementA->erase(statementA->begin());
                statementA->emplace_back(symbolA_);
                rules[symbolA_].emplace_back(*statementA);
                statementA = rules[symbolA].erase(statementA);
            }
            for (auto& statementA : rules[symbolA])
                statementA.emplace_back(symbolA_);
            rules[symbolA_].emplace_back(std::vector<std::string>{ epsilon });
        }
    }
}
void Grammar::eliminateLeftCommonFactor() {
    bool flag = true;
    while (flag) {
        flag = false;
        for (auto& [symbolA, ruleA] : rules) {
            std::unordered_set<std::string> Set;
            std::string commonSymbol;
            for (const auto& statementA : ruleA) {
                const auto& symbol = statementA.front();
                if (symbol == epsilon)
                    continue;
                if (Set.find(symbol) != Set.end()) {
                    commonSymbol = symbol;
                    break;
                }
                Set.emplace(symbol);
            }
            if (commonSymbol.empty())
                continue;
            auto symbolA_ = symbolA + "'";
            bool hasEpsilon = false;
            for (auto statementA = ruleA.begin(); statementA != ruleA.end();) {
                if (statementA->front() != commonSymbol) {
                    ++statementA;
                    continue;
                }
                statementA->erase(statementA->begin());
                if (statementA->empty())
                    hasEpsilon = true;
                else
                    rules[symbolA_].emplace_back(*statementA);
                statementA = ruleA.erase(statementA);
            }
            if (hasEpsilon)
                rules[symbolA_].emplace_back(std::vector<std::string>{ epsilon });
            ruleA.emplace_back(std::vector<std::string>{ commonSymbol, symbolA_ });
            flag = true;
            break;
        }
    }
}
void Grammar::printRule(std::ofstream& fout) {
    fout << "Starting Symbol:\n"
         << startSymbol << "\n\n";
    fout << "Non Terminal Symbols:\n";
    for (const auto& symbol : nonTerminalSymbols)
        fout << symbol << ' ';
    fout << "\n\n";
    fout << "Terminal Symbols:\n";
    for (const auto& symbol : terminalSymbols)
        fout << symbol << ' ';
    fout << "\n\n";
    fout << "Production Rules:\n";
    for (const auto& rule : rules) {
        fout << rule.first << " -> ";
        for (auto statement = rule.second.begin(); statement != rule.second.end(); ++statement) {
            for (const auto symbol : *statement)
                fout << symbol << ' ';
            if (statement + 1 != rule.second.end())
                fout << "| ";
        }
        fout << '\n';
    }
    fout << "\n";
}
std::unordered_set<std::string> Grammar::findFirst(const std::vector<std::string>& statement) {
    std::unordered_set<std::string> res;
    bool flag = true;
    for (const auto& symbol : statement) {
        flag = false;
        for (const auto& symbol_ : first[symbol])
            if (symbol_ == epsilon)
                flag = true;
            else
                res.emplace(symbol_);
        if (!flag)
            break;
    }
    if (flag)
        res.emplace(epsilon);
    return std::move(res);
}
