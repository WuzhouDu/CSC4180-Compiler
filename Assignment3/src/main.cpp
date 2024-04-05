#include "LL1Parser.hpp"
#include <fstream>

using namespace std;

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        cout << "Usage: " << argv[0] << " <grammar_file> <input_file> <output_file> \n";
        return 1;
    }

    LL1Parser parser = LL1Parser();
    parser.readGrammar(argv[1]);
    // parser.printGrammar();
    // parser.printNonTerminals();
    // parser.printTerminals();
    parser.computeFirstSet();
    // parser.printFirst();
    // parser.printNullable();
    parser.computeFollowSet();
    // parser.printFollow();
    parser.buildParsingTable();
    // parser.printParsingTable();
    parser.parseFromFile(argv[2], argv[3]);

    return 0;
}