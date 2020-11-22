#pragma once

#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

enum class State { NORMAL,
    COMMENT_LINE,
    COMMENT_BLOCK,
    DEFINE_INCLUDE };
class LexicalAnalysis {
private:
    std::ifstream& file;
    unsigned int row;
    unsigned int charCount;
    unsigned int symbolCount;
    State state;
    std::map<std::string, unsigned int> symbolTable;
    std::vector<std::string> comment;
    std::vector<std::string> error;
    std::vector<std::pair<std::string, std::string>> tokenTable;

    const std::set<std::string> RESERVED_WORD = { "auto", "break", "case", "char", "const", "continue", "default", "do", "double", "else", "main", "enum", "extern", "float", "for", "goto",
        "if", "int", "long", "register", "return", "short", "signed", "sizeof", "static", "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while" };
    const std::set<std::string> OPERATOR = {
        "+", "-", "*", "/", "^", "|", "&", "(", ")", ";", "{", "}", "[", "]", ",", ":", "?", ".", "%", "~", "<=", ">=",
        "==", "!=", "&&", "||", "...", "=", "*=", "/=", "%=", "+=", "-=", "<<=", ">>=", "&=", "^=", "|="
    };
    const std::set<std::string> ESCAPE = {
        "\\'", "\\\"", "\\?", "\\\\", "\\a", "\\b", "\\f", "\\n", "\\r", "\\t", "\\v"
    };
    void changeRow(char C, int v);
    bool isBlankChar(char C);
    void changeCharCount(char C, int v);
    void retract();
    char getChar();
    unsigned int getSymbol(std::string token);
    void readOrder(char& C, std::string& token);    

public:
    LexicalAnalysis(std::ifstream& fin)
        : file(fin), row(1), charCount(0), symbolCount(0), state(State::NORMAL) {
        file.seekg(0, std::ios::beg);
    }
    unsigned int getrow() const;
    unsigned int getCharCount() const;
    std::map<std::string, unsigned int> getSymbolTable() const;
    std::vector<std::pair<std::string, std::string>> getTokenTable() const;
    std::vector<std::string> getComment() const;
    std::vector<std::string> getError() const;
    bool solve();
    void printToken();
};
