#include "LL1Parser.hpp"
#include <fstream>
#include <stack>

using namespace std;

void LL1Parser::readGrammar(const char *filename)
{
    ifstream file(filename, ifstream::in);
    string line;
    std::unordered_set<std::string> allTokens;
    bool start = true;
    while (getline(file, line))
    {
        size_t pos = line.find("::=");
        if (pos != string::npos)
        {
            string nonTerminal = line.substr(0, pos - 1);
            nonTerminals.insert(nonTerminal);
            if (start)
            {
                startSymbol = nonTerminal;
                start = false;
            }
            vector<string> rightSymbols;
            string productions = line.substr(pos + 3);
            size_t start = 0, end;
            while ((end = productions.find(' ', start)) != string::npos)
            {
                string symbol = productions.substr(start, end - start);
                start = end + 1;
                if (symbol != "")
                {
                    rightSymbols.push_back(symbol);
                    allTokens.insert(symbol);
                }
            }
            string symbol = productions.substr(start);
            if (symbol != "")
            {
                rightSymbols.push_back(symbol);
                allTokens.insert(symbol);
            }

            grammar[nonTerminal].push_back(new productRule(nonTerminal, rightSymbols));
        }
    }

    // find all the terminals
    for (string token : allTokens)
    {
        if (nonTerminals.find(token) == nonTerminals.end())
        {
            terminals.insert(token);
        }
    }
    terminals.erase("''");

    file.close();
}

void LL1Parser::computeFirstSet()
{
    // Initialize FIRST sets for all non-terminals with empty sets
    for (const auto &nonTerminal : grammar)
    {
        first[nonTerminal.first] = {};
        nullable[nonTerminal.first] = false;
    }

    bool changed = true;
    while (changed)
    {
        changed = false;
        // Iterate over each production rule
        for (const auto &nonTerminal : grammar)
        {
            const auto &rules = nonTerminal.second;
            for (const auto &rule : rules)
            {
                const auto &symbols = rule->right;
                if (symbols.size() == 1 && symbols[0] == "''")
                {
                    if (!nullable[nonTerminal.first])
                        changed = true;
                    nullable[nonTerminal.first] = true;
                    continue;
                }

                bool allNullable = true;
                size_t i = 0;
                while (i < symbols.size())
                {
                    const std::string &symbol = symbols[i];
                    // If the symbol is a terminal, add it to the FIRST set of the non-terminal
                    if (terminals.find(symbol) != terminals.end())
                    {
                        if (first[nonTerminal.first].find(symbol) == first[nonTerminal.first].end())
                        {
                            first[nonTerminal.first].insert(symbol);
                            changed = true;
                        }
                        allNullable = false;
                        break;
                    }
                    else if (nonTerminals.find(symbol) != nonTerminals.end())
                    {
                        // If the symbol is a non-terminal
                        // Add the FIRST set of that non-terminal to the current non-terminal's FIRST set
                        for (const auto &first_symbol : first[symbol])
                        {
                            if (first[nonTerminal.first].find(first_symbol) == first[nonTerminal.first].end())
                            {
                                first[nonTerminal.first].insert(first_symbol);
                                changed = true;
                            }
                        }
                        // If the non-terminal is not nullable, break the loop
                        if (!nullable[symbol])
                        {
                            allNullable = false;
                            break;
                        }
                    }
                    else
                    {
                        // Error: Symbol not found
                        cout << "ERROR: symbol not found: " << symbol << "\n";
                        return;
                    }
                    ++i;
                }
                // Update nullable map
                if (allNullable)
                {
                    if (!nullable[nonTerminal.first])
                    {
                        nullable[nonTerminal.first] = true;
                        changed = true;
                    }
                }
            }
        }
    }
}

void LL1Parser::computeFollowSet()
{
    // Initialize FOLLOW sets for all non-terminals with empty sets
    for (const auto &nonTerminal : grammar)
    {
        follow[nonTerminal.first] = {};
    }

    // // Add $ (end of input marker) to the FOLLOW set of the start symbol
    follow[startSymbol].insert("$");

    bool changed = true;
    while (changed)
    {
        changed = false;
        // Iterate over each production rule
        for (const auto &nonTerminal : grammar)
        {
            const auto &rules = nonTerminal.second;
            for (const auto &rule : rules)
            {
                const auto &symbols = rule->right;
                for (size_t i = 0; i < symbols.size(); ++i)
                {
                    const std::string &symbol = symbols[i];
                    if (nonTerminals.find(symbol) != nonTerminals.end())
                    {
                        // If the symbol is a non-terminal
                        // Add the FOLLOW set of the left-hand side of the rule to the symbol's FOLLOW set
                        size_t j = i + 1;
                        bool allNullable = true;
                        while (j < symbols.size() && allNullable)
                        {
                            const std::string &next_symbol = symbols[j];
                            if (terminals.find(next_symbol) != terminals.end())
                            {
                                if (follow[symbol].find(next_symbol) == follow[symbol].end())
                                {
                                    follow[symbol].insert(next_symbol);
                                    changed = true;
                                }
                                allNullable = false;
                            }
                            else if (nonTerminals.find(next_symbol) != nonTerminals.end())
                            {
                                for (const auto &first_symbol : first[next_symbol])
                                {
                                    if (follow[symbol].find(first_symbol) == follow[symbol].end())
                                    {
                                        follow[symbol].insert(first_symbol);
                                        changed = true;
                                    }
                                }
                                if (!nullable[next_symbol])
                                {
                                    allNullable = false;
                                }
                            }
                            ++j;
                        }
                        if (allNullable)
                        {
                            const std::string &left_symbol = rule->left;
                            for (const auto &follow_symbol : follow[left_symbol])
                            {
                                if (follow[symbol].find(follow_symbol) == follow[symbol].end())
                                {
                                    follow[symbol].insert(follow_symbol);
                                    changed = true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void LL1Parser::buildParsingTable()
{
    // Initialize parsing table
    for (const auto &nonTerminal : nonTerminals)
    {
        for (const auto &terminal : terminals)
        {
            parsingTable[nonTerminal][terminal] = {};
        }
        parsingTable[nonTerminal]["$"] = {};
    }

    // For each production rule A → α, where A is a non-terminal
    for (const auto &nonTerminal : grammar)
    {
        const std::string &A = nonTerminal.first;
        const auto &rules = nonTerminal.second;
        for (const auto &rule : rules)
        {
            bool allNullable = true;
            const auto &alpha = rule->right;

            // For each terminal 't' in FIRST(α), add A → α to M[A, t]
            for (const auto &symbol : alpha)
            {
                if (terminals.find(symbol) != terminals.end())
                {
                    parsingTable[A][symbol].push_back(rule);
                    if (parsingTable[A][symbol].size() > 1)
                    {
                        cout << "ERROR: not LL(1) grammar\n";
                        return;
                    }
                    allNullable = false;
                    break;
                }
                else if (nonTerminals.find(symbol) != nonTerminals.end())
                {
                    for (const auto &first_symbol : first[symbol])
                    {
                        parsingTable[A][first_symbol].push_back(rule);
                        if (parsingTable[A][first_symbol].size() > 1)
                        {
                            cout << "ERROR: not LL(1) grammar\n";
                            return;
                        }
                    }
                    if (!nullable[symbol])
                    {
                        allNullable = false;
                        break;
                    }
                }
            }

            // If ε (empty string) is in FIRST(α), for each terminal 't' in FOLLOW(A),
            // add A → α to M[A, t]
            if (allNullable)
            {
                for (const auto &follow_symbol : follow[A])
                {
                    parsingTable[A][follow_symbol].push_back(rule);
                    if (parsingTable[A][follow_symbol].size() > 1)
                    {
                        cout << "ERROR: not LL(1) grammar\n";
                        return;
                    }
                }
            }
        }
    }
}

bool LL1Parser::parseFromFile(const std::string &inputFilename, const std::string &outputFilename)
{
    std::ifstream inputFile(inputFilename);
    std::ofstream outputFile(outputFilename);
    if (!inputFile.is_open())
    {
        std::cout << "Error: Failed to open input file " << inputFilename << std::endl;
        return false;
    }

    // outputFile.open(outputFilename);
    if (!outputFile.is_open())
    {
        std::cout << "Error: Failed to open output file " << outputFilename << std::endl;
        inputFile.close();
        return false;
    }

    std::stack<std::string> stack;
    stack.push("$");
    stack.push(startSymbol);

    std::string token;
    while (inputFile >> token) {
        outputFile << "Token: " << token << std::endl;
        while (!stack.empty())
        {
            std::string top = stack.top();
            stack.pop();
            
            if (terminals.find(top) != terminals.end())
            {
                // If top of the stack is a terminal
                if (top != token)
                {
                    outputFile << "Error: Unexpected token '" << token << "', expected '" << top << "'" << std::endl;
                    inputFile.close();
                    outputFile.close();
                    return false;
                }
                outputFile << "Matched token: " << token << std::endl << std::endl;

                break; // Move to the next token
            }
            else if (nonTerminals.find(top) != nonTerminals.end())
            {
                // If top of the stack is a non-terminal
                auto it = parsingTable.find(top);
                if (it == parsingTable.end())
                {
                    outputFile << "Error: No parsing rule for non-terminal '" << top << std::endl;
                    inputFile.close();
                    outputFile.close();
                    return false;
                }
                auto &rules = it->second;
                auto rule_it = rules.find(token);
                if (rule_it == rules.end())
                {
                    outputFile << "Error: No parsing rule for non-terminal '" << top << "' token '" << token << "'" << std::endl;
                    inputFile.close();
                    outputFile.close();
                    return false;
                }
                if (rule_it->second.empty()) {
                    outputFile << "Error: No parsing rule for non-terminal '" << top << "' token '" << token << "'" << std::endl;
                    inputFile.close();
                    outputFile.close();
                    return false;
                }
                productRule *rule = rule_it->second[0];
                outputFile << "Applying rule for non-terminal '" << top << "' and token '" << token << "':" << std::endl;

                outputFile << "  " << rule->left << " -> ";
                for (const auto &symbol : rule->right)
                {
                    outputFile << symbol << " ";
                }
                outputFile << std::endl;
                for (auto it = rule->right.rbegin(); it != rule->right.rend(); ++it)
                {
                    stack.push(*it);
                }
            }
        }
    }

    if (inputFile.eof() && stack.top() == "prog") {
        stack.pop();
        if (stack.top() == "$") {
            outputFile << "Successfully parsing.";
            inputFile.close();
            outputFile.close();
            return true;
        }
        outputFile << "Error: Stack not empty after parsing input";
        inputFile.close();
        outputFile.close();
        return false;
    }

    outputFile << "Error: Stack not empty after parsing input";
    inputFile.close();
    outputFile.close();
    return false;
}