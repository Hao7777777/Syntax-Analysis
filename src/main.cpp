#include "LL1.h"
#include "LR1.h"
#include "grammar.h"
#include "lexical.h"
#include "recursive.h"
#include <cstring>
#include <fstream>
int main(int argc, char** argv) {
    try {
        if (argc < 4)
            throw std::runtime_error("Too few arguments");
        std::ifstream finGrammar(argv[1]);
        std::ifstream fin(argv[2]);
        std::ofstream fout(argv[3]);
        if (!finGrammar.is_open())
            throw std::runtime_error(argv[1] + std::string(" is unable to open."));
        if (!fin.is_open())
            throw std::runtime_error(argv[2] + std::string(" is unable to open."));
        LexicalAnalysis lex(fin);
        lex.LexicalAnalysis::solve();
        // lex.LexicalAnalysis::printToken();
        if (!strcmp(argv[4], "-LL1")) {
            LL1 LL1Grammar(finGrammar);
            LL1Grammar.LL1::work();
            LL1Grammar.LL1::printRule(fout);
            LL1Grammar.LL1::printTable(fout);
            LL1Grammar.LL1::solve(lex.LexicalAnalysis::getTokenTable(), fout);
        } else if (!strcmp(argv[4], "-LR1")) {
            LR1 LR1Grammar(finGrammar);
            LR1Grammar.LR1::work();
            LR1Grammar.LR1::printRule(fout);
            LR1Grammar.LR1::printDFA(fout);
            LR1Grammar.LR1::printTable(fout);
            LR1Grammar.LR1::solve(lex.LexicalAnalysis::getTokenTable(), fout);
        } else if (!strcmp(argv[4], "-recursive")) {
            Recursive RE(fout);
            RE.Recursive::solve(lex.LexicalAnalysis::getTokenTable());
        } else
            throw std::runtime_error("parameter error");
        finGrammar.close();
        fin.close();
        fout.close();
    } catch (const std::exception& e) {
        std::cout << e.what() << '\n';
    }
    return 0;
}