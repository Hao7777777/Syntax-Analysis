#include "lexical.h"

void LexicalAnalysis::changeRow(char C, int v) {
    if (C == '\n')
        row += v;
}
bool LexicalAnalysis::isBlankChar(char C) {
    return (C == ' ' || C == '\t' || C == '\v' || C == '\r' || C == '\n');
}
void LexicalAnalysis::changeCharCount(char C, int v) {
    if (C != EOF)
        charCount += v;
}
void LexicalAnalysis::retract() {
    // file.seekg(-1, ios::cur);
    file.unget();
    char C = file.peek();
    // cerr<<(int)C<<'\n';
    changeRow(C, -1);
    changeCharCount(C, -1);
}

char LexicalAnalysis::getChar() {
    char C = file.get();
    changeRow(C, 1);
    changeCharCount(C, 1);
    return C;
}
unsigned int LexicalAnalysis::getSymbol(std::string token) {
    if (symbolTable.find(token) == symbolTable.end()) {
        symbolTable[token] = symbolCount++;
    } else {
        return symbolTable[token];
    }
    return symbolCount;
}
void LexicalAnalysis::readOrder(char& C, std::string& token) {
    if (isBlankChar(C)) {

    } else if (isdigit(C) || C == '.') {
        bool isInt = false, isFloat = false, isOper = false;
        token += C;
        if (C == '.' || (C == '0' && file.peek() == '.') || C != '0') {
            int numberState = C == '.';
            while (!isInt && !isFloat && !isOper) {
                C = getChar();
                switch (numberState) {
                case 0:
                    if (isdigit(C))
                        break;
                    else if (C == '.')
                        numberState = 2;
                    else if (C == 'e' || C == 'E')
                        numberState = 3;
                    else
                        isInt = true;
                    break;
                case 1:
                    if (isdigit(C))
                        numberState = 2;
                    else if (C == '.')
                        numberState = 6;
                    else
                        isOper = true;
                    break;
                case 2:
                    if (isdigit(C))
                        break;
                    else if (C == 'e' || C == 'E')
                        numberState = 3;
                    else
                        isFloat = true;
                    break;
                case 3: // e E
                    if (C == '+' || C == '-')
                        numberState = 4;
                    else if (isdigit(C))
                        numberState = 5;
                    else
                        throw std::runtime_error("Row " + std::to_string(row) + ": Bad Exponential");
                    break;
                case 4:
                    if (isdigit(C))
                        numberState = 5;
                    else
                        throw std::runtime_error("Row " + std::to_string(row) + ": Bad Exponential");
                    break;
                case 5:
                    if (isdigit(C))
                        break;
                    else
                        isFloat = true;
                    break;
                case 6:
                    if (C == '.')
                        numberState = 7;
                    else
                        throw std::runtime_error("Row " + std::to_string(row) + ": Syntax error " + token + std::string(1, C));
                    break;
                case 7:
                    isOper = true;
                    break;
                default:
                    throw std::runtime_error("Row " + std::to_string(row) + ": Intra Automaton Error");
                    break;
                }
                if (isInt || isFloat || isOper)
                    retract();
                else
                    token += C;
            }
        } else if (file.peek() == 'X' || file.peek() == 'x') {
            C = getChar();
            token += C;
            isInt = true;
            while (true) {
                C = getChar();
                if (isdigit(C) || (C >= 'A' && C <= 'F') || (C >= 'a' && C <= 'f'))
                    token += C;
                else
                    break;
            }
            retract();
            if (token.length() <= 2)
                throw std::runtime_error("Row " + std::to_string(row) + ": Invalid 0X");
        } else {
            isInt = true;
            while (true) {
                C = getChar();
                if (C >= '0' && C <= '7')
                    token += C;
                else
                    break;
            }
            retract();
        }
        if (isOper)
            return;
        int numberState = isInt ? 3 : 2;
        bool tag = true;
        while (tag) {
            C = getChar();
            switch (numberState) {
            case 0:
                if (isalnum(C))
                    break;
                else
                    throw std::runtime_error("Row " + std::to_string(row) + ": Bad Suffix");
                break;
            case 1:
                if (isalnum(C))
                    numberState = 0;
                else
                    tag = false;
                break;
            case 2:
                if (C == 'f' || C == 'F' || C == 'l' || C == 'L')
                    numberState = 1;
                else if (isalnum(C))
                    numberState = 0;
                else
                    tag = false;
                break;
            case 3:
                if (C == 'u' || C == 'U')
                    numberState = 4;
                else if (C == 'l' || C == 'L')
                    numberState = 5;
                else if (isalnum(C))
                    numberState = 0;
                else
                    tag = false;
                break;
            case 4:
                if (C == 'l' || C == 'L')
                    numberState = 1;
                else if (isalnum(C))
                    numberState = 0;
                else
                    tag = false;
                break;
            case 5:
                if (C == 'u' || C == 'U')
                    numberState = 1;
                else if (isalnum(C))
                    numberState = 0;
                else
                    tag = false;
                break;
            default:
                throw std::runtime_error("Row " + std::to_string(row) + ": Intra Automaton Error");
                break;
            }
            if (tag)
                token += C;
            else
                retract();
        }
        tokenTable.emplace_back(make_pair("number", token));
    } else if (isalpha(C) || C == '_') { //标识符
        while (isalnum(C) || C == '_') {
            token += C;
            C = getChar();
        }
        // cerr<<(int)C<<'\n';
        retract();
        if (RESERVED_WORD.find(token) != RESERVED_WORD.end()) {
            tokenTable.emplace_back(make_pair("reserved", token));
        } else {
            int id = getSymbol(token);
            tokenTable.emplace_back(make_pair("identifier", token));
        }
    } else if (C == '"' || C == '\'') { //字符串 字符
        char CBegin = C;
        while (true) {
            C = getChar();
            if (C == EOF) {
                retract();
                break;
            }
            if (C == CBegin) {
                break;
            }
            if (C == '\n') {
                throw std::runtime_error("Row " + std::to_string(row - 1) + ": missing terminating ' character");
            }
            token += C;
            if (C == '\\') {
                std::string ss(1, C);
                C = getChar();
                ss += C;
                if (C == '\n') {
                    token.pop_back();
                } else if (ESCAPE.find(ss) != ESCAPE.end()) {
                    token += C;
                } else {
                    throw std::runtime_error("Row " + std::to_string(row) + ": Invalid escape char");
                }
            }
        }
        if (CBegin == '\'') {
            if (token.size() <= 0) {
                throw std::runtime_error("Row " + std::to_string(row) + ": Too few argument for char const");
            }
            tokenTable.emplace_back(make_pair("char", token));
        } else {
            tokenTable.emplace_back(make_pair("string", token));
        }

    } else if (C == '#') {
        state = State::DEFINE_INCLUDE;
    } else if (C == '/' && file.peek() == '/') { // 行注释开头
        C = getChar();
        state = State::COMMENT_LINE;
    } else if (C == '/' && file.peek() == '*') { // 块注释开头
        C = getChar();
        state = State::COMMENT_BLOCK;
    } else {
        token += C;
        while (true) {
            C = getChar();
            if (isBlankChar(C) || isalnum(C) || C == EOF) {
                retract();
                break;
            } else {
                if (OPERATOR.find(token + C) == OPERATOR.end()) {
                    retract();
                    break;
                } else {
                    token += C;
                }
            }
        }
        if (OPERATOR.find(token) != OPERATOR.end()) {
            tokenTable.emplace_back(make_pair("operator", token));
        } else {
            throw std::runtime_error("Row " + std::to_string(row) + ": Invalid Operator");
        }
    }
}
unsigned int LexicalAnalysis::getrow() const {
    return row;
}
unsigned int LexicalAnalysis::getCharCount() const {
    return charCount;
}
std::map<std::string, unsigned int> LexicalAnalysis::getSymbolTable() const {
    return symbolTable;
}
std::vector<std::pair<std::string, std::string>> LexicalAnalysis::getTokenTable() const {
    return tokenTable;
}
std::vector<std::string> LexicalAnalysis::getComment() const {
    return comment;
}
std::vector<std::string> LexicalAnalysis::getError() const {
    return error;
}
bool LexicalAnalysis::solve() {
    if (file.peek() == EOF) {
        return true;
    }
    char C;
    std::string token = "";
    while ((C = getChar()) != EOF) {
        switch (state) {
        case State::NORMAL:
            try {
                readOrder(C, token);
            } catch (const std::exception& e) {
                error.emplace_back(e.what());
            }

            token.clear();
            break;
        case State::COMMENT_LINE:
            if (C == '\n') {
                tokenTable.emplace_back(std::make_pair("SPLIT",""));
                comment.emplace_back(token);
                token.clear();
                state = State::NORMAL;
                break;
            }
            token += C;
            break;
        case State::COMMENT_BLOCK:
            if (C == '*') {
                if (getChar() == '/') {
                    tokenTable.emplace_back(std::make_pair("SPLIT",""));
                    comment.emplace_back(token);
                    token.clear();
                    state = State::NORMAL;
                } else {
                    retract();
                }
            }
            token += C;
            break;
        case State::DEFINE_INCLUDE:
            if (C == '\n')
                state = State::NORMAL;
            break;
        default:
            return false;
            break;
        }
    }
    return true;
}
void LexicalAnalysis::printToken() {
    for (const auto& token : tokenTable) {
        std::cerr << token.first << ' ' << token.second << '\n';
    }
}