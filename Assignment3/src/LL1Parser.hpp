#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <iostream>

class productRule {
public:
    std::string left;
    std::vector<std::string> right;
    productRule(std::string _left, std::vector<std::string> _right) : left(_left), right(_right) {}

    void print()
    {
        std::cout << left << " -> ";
        for (auto rightSymbol : right)
        {
            std::cout << rightSymbol << " ";
        }
        std::cout << "\n";
    }
};

class LL1Parser
{
    /* Constructor and Destructor */
public:
    LL1Parser() = default;

private:
    std::string startSymbol;
    std::unordered_map<std::string, std::vector<productRule *>> grammar;
    std::unordered_set<std::string> nonTerminals;
    std::unordered_set<std::string> terminals;
    std::unordered_map<std::string, bool> nullable;
    std::unordered_map<std::string, std::unordered_set<std::string>> first;
    std::unordered_map<std::string, std::unordered_set<std::string>> follow;
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<productRule*>>> parsingTable;

public:
    void readGrammar(const char *);
    void computeFirstSet();
    void computeFollowSet();
    bool parseFromFile(const std::string& inputFilename, const std::string& outputFilename);
    void buildParsingTable();

// debug useage
    void printGrammar()
    {
        for (auto nonTerminal : grammar)
        {
            std::cout << "left: " << nonTerminal.first << "\n";
            for (auto rule : nonTerminal.second)
            {
                rule->print();
            }
            std::cout << "\n";
        }
    }

    void printTerminals()
    {
        for (auto terminal : terminals)
        {
            std::cout << terminal << " ";
        }
        std::cout << "\n";
    }

    void printNonTerminals()
    {
        for (auto nonTerminal : nonTerminals)
        {
            std::cout << nonTerminal << " ";
        }
        std::cout << "\n";
    }

    void printFirst() {
        for (auto nonTerminal : nonTerminals) {
            std::cout << "non-terminal: " << nonTerminal << " first set size: " << first[nonTerminal].size() << "\n";
            for (auto firstSymbol : first[nonTerminal]) {
                std::cout << firstSymbol << " ";
            }
            std::cout << "\n\n";
        }
    }

    void printNullable() {
        for (auto nonTerminal : nonTerminals) {
            std::cout << "non-terminal: " << nonTerminal << " nullable: " << nullable[nonTerminal] << "\n";
        }
    }

    void printFollow() {
        for (auto nonTerminal : nonTerminals)
        {
            std::cout << "non-terminal: " << nonTerminal << " follow set size: " << follow[nonTerminal].size() << "\n";
            for (auto followSymbol : follow[nonTerminal])
            {
                std::cout << followSymbol << " ";
            }
            std::cout << "\n\n";
        }
    }

    void printParsingTable() {
        for (auto nonTerminal : parsingTable) {
            std::cout << "non-terminal: " << nonTerminal.first << "\n";
            for (auto terminal : nonTerminal.second) {
                if (terminal.second.empty()) {
                    continue;
                }
                std::cout << "terminal: " << terminal.first << "\n";
                for (auto rule : terminal.second) {
                    rule->print();
                }
            }
            std::cout << "\n";
        }
    }
};