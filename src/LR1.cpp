#include "LR1.h"


LR1::~LR1() {
}
void LR1::work() {
    addStart();
    getSymbols();
    getFirst();
    // printFirst();
    getTable();
}
void LR1::addStart() {
    std::string symbol = startSymbol + "'";
    nonTerminalSymbols.emplace(symbol);
    rules[symbol] = { { startSymbol } };
    startSymbol = symbol;
}
void LR1::getTable() {
    auto getClosure = [&](itemSet& cur) -> void {
        bool flag = true;
        while (flag) {
            flag = false;
            for (const auto& _item : cur) {
                if (_item.dotPos == _item.statement.size())
                    continue;
                const std::string& next = _item.statement[_item.dotPos];
                if (isTerminal(next))
                    continue;
                std::vector<std::string> statement;
                for (size_t pos = _item.dotPos + 1; pos != _item.statement.size(); ++pos)
                    statement.emplace_back(_item.statement[pos]);
                auto _first = findFirst(statement);
                if (_first.find(epsilon) != _first.end()) {
                    _first.erase(epsilon);
                    _first.emplace(_item.lookahead);
                }
                for (const auto& rule : rules[next])
                    for (const auto& lookahead : _first) {
                        item newItem(next, rule, lookahead, 0);
                        if (cur.find(newItem) == cur.end()) {
                            cur.emplace(newItem);
                            flag = true;
                        }
                    }
            }
        }
    };
    itemSet startItemSet;
    startItemSet.emplace(item(startSymbol, rules[startSymbol].front(), dollar, 0));
    getClosure(startItemSet);
    itemSets.emplace_back(startItemSet);

    auto run = [&](const itemSet& cur, const std::string& symbol) -> int {
        itemSet newItemSet;
        for (const auto& _item : cur) {
            if (_item.dotPos == _item.statement.size())
                continue;
            const std::string& next = _item.statement[_item.dotPos];
            if (next == symbol) {
                item newItem(_item);
                ++newItem.dotPos;
                newItemSet.emplace(newItem);
            }
        }
        getClosure(newItemSet);
        if (newItemSet.empty())
            return -1;
        for (size_t i = 0; i < itemSets.size(); ++i) {
            if (itemSets[i] == newItemSet)
                return i;
        }
        itemSets.emplace_back(std::move(newItemSet));
        return itemSets.size() - 1;
    };
    for (size_t i = 0; i < itemSets.size(); ++i) {
        for (const auto& symbol : nonTerminalSymbols) {
            int go = run(itemSets[i], symbol);
            if (go > 0) {
                table[i][symbol] = std::make_pair(Action::SHIFT, go);
                goTo.emplace(symbol);
            }
        }
        for (const auto& symbol : terminalSymbols) {
            int go = run(itemSets[i], symbol);
            if (go > 0) {
                table[i][symbol] = std::make_pair(Action::SHIFT, go);
                action.emplace(symbol);
            }
        }
        for (const auto& _item : itemSets[i])
            if (_item.dotPos == _item.statement.size()) {
                if (_item.nonTerminal == startSymbol)
                    table[i][_item.lookahead] = std::make_pair(Action::ACCEPT, 0);
                else
                    table[i][_item.lookahead] = std::make_pair(Action::REDUCE, 0);
                action.emplace(_item.lookahead);
            }
    }
}
void LR1::printDFA(std::ofstream& fout) {
    fout << "LR1 DFA:\n";
    for (size_t i = 0; i < itemSets.size(); ++i) {
        fout << "Item Set " << i << ":\n";
        for (const auto& _item : itemSets[i]) {
            fout << _item.nonTerminal << " -> ";
            for (size_t j = 0; j < _item.statement.size(); ++j) {
                if (j == _item.dotPos)
                    fout << "· ";
                fout << _item.statement[j] << ' ';
            }
            if (_item.statement.size() == _item.dotPos)
                fout << ". ";
            fout << ", " << _item.lookahead << '\n';
        }
    }
    fout << '\n';
}
void LR1::printTable(std::ofstream& fout) {
    fout << "LR1 Table:\n";
    const int len = 10;
    int actionLen = action.size() * len;
    int goToLen = goTo.size() * len;
    fout << std::setw(len) << std::setfill(' ') << ' ';

    fout << std::setw((actionLen - 5) / 2) << std::setfill(' ') << ' ';
    fout << "action";
    fout << std::setw(actionLen - 5 - (actionLen - 5) / 2) << std::setfill(' ') << ' ';

    fout << std::setw((goToLen - 5) / 2) << std::setfill(' ') << ' ';
    fout << "goto";
    fout << std::setw(goToLen - 5 - (goToLen - 5) / 2) << std::setfill(' ') << ' ';

    fout << '\n';
    fout << std::setw(len) << std::setfill(' ') << "status"
         << "|";
    for (const auto& symbol : action)
        fout << std::setw(len) << std::setfill(' ') << symbol;
    fout << "|";
    for (const auto& symbol : goTo)
        fout << std::setw(len) << std::setfill(' ') << symbol;
    fout << '\n';
    for (size_t i = 0; i < itemSets.size(); ++i) {
        fout << std::setw(len) << std::setfill(' ') << std::to_string(i) << "|";

        for (const auto& symbol : action) {
            if (table[i].find(symbol) == table[i].end())
                fout << std::setw(len) << std::setfill(' ') << ' ';
            else {
                auto type = table[i][symbol].first;
                auto go = table[i][symbol].second;
                if (type == Action::SHIFT)
                    fout << std::setw(len) << std::setfill(' ') << "S" + std::to_string(go);
                else if (type == Action::REDUCE) {
                    std::string out("R ");
                    std::unordered_set<std::string> _rule;
                    for (const auto& _item : itemSets[i]) {
                        if (_item.dotPos != _item.statement.size())
                            continue;
                        std::string __rule;
                        __rule = _item.nonTerminal + "->";
                        for (const auto& symbol : _item.statement)
                            __rule += symbol;
                        _rule.emplace(__rule);
                    }
                    if (_rule.size() > 1) {
                        // for (const auto& __rule : _rule)
                        //     std::cerr << i << ' ' << __rule << '\n';
                        throw std::runtime_error("Reduce/Reduce Conflicts.");
                    }
                    for (const auto& __rule : _rule)
                        out += __rule;

                    fout << std::setw(len) << std::setfill(' ') << out;
                } else {
                    fout << std::setw(len) << std::setfill(' ') << "ACC";
                }
            }
        }
        fout << "|";
        for (const auto& symbol : goTo) {
            if (table[i].find(symbol) == table[i].end())
                fout << std::setw(len) << std::setfill(' ') << ' ';
            else
                fout << std::setw(len) << std::setfill(' ') << std::to_string(table[i][symbol].second);
        }
        fout << '\n';
    }
    fout << '\n';
}
void LR1::solve(const std::vector<std::pair<std::string, std::string>>& token, std::ofstream& fout) {
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
        static std::pair<int, std::string> status[100000];
        int cur1 = 0, cur2 = 0;
        for (size_t i = left; i < right; ++i) {
            fout << token[i].second;
            input[cur1++] = token[i];
        }
        fout << "  :\n";
        input[cur1++] = std::make_pair("ACC", "$");
        status[cur2++] = std::make_pair(0, "$");
        auto p1 = &input[0];
        auto p2 = &status[cur2 - 1];
        for (;;) {
            std::string out = "";
            for (int i = 0; i < cur2; ++i)
                out += status[i].second + std::to_string(status[i].first);
            fout << out << std::setw(len - out.size()) << std::setfill(' ') << '|';
            out = "";
            for (int i = p1 - &input[0]; i < cur1; ++i)
                out += input[i].second;
            fout << std::setw(len) << std::setfill(' ') << out << '|';
            out = "";

            std::string symbol;
            if (p1->first == "number")
                symbol = "num";
            else if (p1->first == "operator" || p1->first == "ACC")
                symbol = p1->second;
            if (table[p2->first].find(symbol) == table[p2->first].end()) {
                out = ("'" + symbol + "'" + " is missmatching.");
                fout << out << std::setw(len - out.size()) << std::setfill(' ') << '\n';
                break;
            }
            auto action = table[p2->first][symbol];
            if (action.first == Action::SHIFT) {
                out = "shift " + std::to_string(action.second);
                status[cur2++] = std::make_pair(action.second, symbol);
                ++p2;
                ++p1;
            } else if (action.first == Action::REDUCE) {

                out = "reduce by ";
                std::string nonTerminal;
                int cnt = 0;
                for (const auto& _item : itemSets[p2->first]) {
                    if (_item.dotPos != _item.statement.size())
                        continue;
                    nonTerminal = _item.nonTerminal;
                    out += _item.nonTerminal + "->";
                    for (const auto& symbol : _item.statement) {
                        out += symbol;
                        ++cnt;
                    }
                    break;
                }
                while (cnt--) {
                    --cur2;
                    --p2;
                }
                auto goTo = table[p2->first][nonTerminal];
                status[cur2++] = std::make_pair(goTo.second, nonTerminal);
                ++p2;
            } else {
                out = "accept";
            }
            fout << out << std::setw(len - out.size()) << std::setfill(' ') << '\n';
            if (out == "accept")
                break;
        }
        fout << '\n';
        left = right;
    }
}