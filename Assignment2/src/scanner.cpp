/**
 * --------------------------------------
 * CUHK-SZ CSC4180: Compiler Construction
 * Assignment 2: Oat v.1 Scanner
 * --------------------------------------
 * Author: Mr.Liu Yuxuan
 * Position: Teaching Assisant
 * Date: February 27th, 2024
 * Email: yuxuanliu1@link.cuhk.edu.cn
 * 
 * File: scanner.cpp
 * ------------------------------------------------------------
 * This file implements scanner function defined in scanner.hpp
 */

#include "scanner.hpp"

DFA::~DFA() {
    for (auto state : states) {
        state->transition.clear();
        delete state;
    }
}

void DFA::print() {
    printf("DFA:\n");
    for (auto state : states)
        state->print();
}

/**
 * Epsilon NFA
 * (Start) -[EPSILON]-> (End)
 */
NFA::NFA() {
    start = new State();
    end = new State();
    start->transition[EPSILON] = {end};
}

/**
 * NFA for a single character
 * (Start) -[c]-> (End)
 * It acts as the unit of operations like union, concat, and kleene star
 * @param c: a single char for NFA
 * @return NFA with only one char
 */
NFA::NFA(char c) {
    start = new State();
    end = new State();
    start->transition[c] = {end};
}

NFA::~NFA() {
    for (auto state : iter_states()) {
        state->transition.clear();
        delete state;
    }
}

/**
 * Concat a string of char
 * Start from the NFA of the first char, then merge all following char NFAs
 * @param str: input string
 * @return
 */
NFA* NFA::from_string(std::string str) {
    NFA* startNFA = new NFA(str.at(0));
    for (int i = 1; i < str.length(); i++) {
        NFA* curNFA = new NFA(str.at(i));
        startNFA->concat(curNFA);
    }
    return startNFA;
}

/**
 * RegExp: [a-zA-Z]
 * @return
 */
NFA* NFA::from_letter() {
    NFA* result = new NFA('a');
    std::set<NFA*> toBeMerged;
    for (char cha = 'b'; cha <= 'z'; ++cha) {
        toBeMerged.insert(new NFA(cha));
    }
    for (char cha = 'A'; cha <= 'Z'; ++cha) {
        toBeMerged.insert(new NFA(cha));
    }
    result->set_union(toBeMerged);
    return result;
}

/**
 * RegExp: [0-9]
 * @return
 */
NFA* NFA::from_digit() {
    NFA* result = new NFA('0');
    std::set<NFA *> toBeMerged;
    for (char cha = '1'; cha <= '9'; ++cha)
    {
        toBeMerged.insert(new NFA(cha));
    }
    result->set_union(toBeMerged);
    return result;
}

/**
 * NFA for any char (ASCII 0-127)
 */
NFA* NFA::from_any_char() {
    NFA *result = new NFA(char(0));
    std::set<NFA *> toBeMerged;
    for (int i = 1; i <= 127; ++i) {
        toBeMerged.insert(new NFA(char(i)));
    }
    result->set_union(toBeMerged);
    return result;
}
/**
 * Concatanat two NFAs
 * @param from: NFA pointer to be concated after the current NFA
 * @return: this -> from
 */
void NFA::concat(NFA* from) {
    end->accepted = false;
    end->transition[EPSILON].insert(from->start);
    from->start = start;
    end = from->end;
}

/**
 * Set Union with another NFA
 * @param
 */
void NFA::set_union(NFA* from) {
    State* newStart = new State();
    newStart->transition[EPSILON].insert(this->start);
    newStart->transition[EPSILON].insert(from->start);
    this->start = newStart;
    from->start = newStart;
    State* newEnd = new State();
    this->end->transition[EPSILON].insert(newEnd);
    from->end->transition[EPSILON].insert(newEnd);
    end = newEnd;
}

/**
 * Set Union with a set of NFAs
 */
void NFA::set_union(std::set<NFA*> set) {
    State *newStart = new State();
    State *newEnd = new State();
    newStart->transition[EPSILON].insert(start);
    end->transition[EPSILON].insert(newEnd);
    for (NFA* element : set) {
        newStart->transition[EPSILON].insert(element->start);
        element->end->transition[EPSILON].insert(newEnd);
    }
    start = newStart;
    end = newEnd;
}

/**
 * Kleene Star Operation
 */
void NFA::kleene_star() {
    State* newStart = new State();
    State* newEnd = new State();
    newStart->transition[EPSILON].insert(start);
    newStart->transition[EPSILON].insert(newEnd);
    start = newStart;
    end->transition[EPSILON].insert(newEnd);
    end->transition[EPSILON].insert(newStart);
    end = newEnd;
}

/**
 * Determinize NFA to DFA by subset construction
 * @return DFA
 */
DFA* NFA::to_DFA() {
    DFA* dfa = new DFA();

    // Map NFA states to one DFA state
    std::map<std::set<NFA::State *>, DFA::State *> dfaStateMap;

    std::set<NFA::State *> startClosure = epsilon_closure(start);
    DFA::State *startState = new DFA::State();
    dfa->states.push_back(startState);
    dfaStateMap[startClosure] = startState;

    std::queue<std::set<NFA::State *>> stateQueue;
    stateQueue.push(startClosure);

    while (!stateQueue.empty()) {
        std::set<State *> currentStateSet = stateQueue.front();
        stateQueue.pop();

        // Get or create the corresponding DFA state
        DFA::State *dfaState = dfaStateMap[currentStateSet];

        // Check if this state contains an accepting state of the NFA
        unsigned int statePrec = 0;
        for (State *NFAState : currentStateSet)
        {
            if (NFAState->accepted)
            {
                dfaState->accepted = true;
                dfaState->token_class = NFAState->precedence > statePrec ? NFAState->token_class : dfaState->token_class;
                statePrec = std::max(statePrec, NFAState->precedence);
            }
        }

        for (int i = 0; i < 128; ++i) {
            std::set<NFA::State *> nextClosure = NFA::epsilon_closure(NFA::move(currentStateSet, char(i)));
            if (nextClosure.empty()) continue;
            if (dfaStateMap.find(nextClosure) == dfaStateMap.end()) {
                DFA::State *nextState = new DFA::State();
                dfa->states.push_back(nextState);
                dfaStateMap[nextClosure] = nextState;
                dfaState->transition[char(i)] = nextState;
                stateQueue.push(nextClosure);
            }
            else {
                dfaState->transition[char(i)] = dfaStateMap[nextClosure];
            }
        }
    }
    return dfa;
}

/**
 * Get the closure of the given Nstates set
 * It means all the Nstates can be reached with the given Nstates set without any actions
 * @param state: State* , the starting state of getting epsilon closure
 * @return {set<State&>} The closure of state
 */
std::set<NFA::State*> NFA::epsilon_closure(NFA::State* state) {
    std::set<NFA::State*> result;
    std::queue<NFA::State *> q;

    result.insert(state);
    q.push(state);

    while (!q.empty()){
        State* curState = q.front();
        q.pop();
        if (curState->transition.find(EPSILON) == curState->transition.end()) {
            continue;
        }
        for (State* accessibleState : curState->transition[EPSILON]) {
            if (result.find(accessibleState) == result.end()) {
                result.insert(accessibleState);
                q.push(accessibleState);
            }
        }
    }
    return result;
}

/**
 * Get the closure of the given Nstates set
 * It means all the Nstates can be reached with the given Nstates set without any actions
 * @param states: set<State*> , the starting state of getting epsilon closure
 * @return {set<State&>} The closure of state
 */
std::set<NFA::State *> NFA::epsilon_closure(std::set<NFA::State*> states)
{
    std::set<NFA::State *> result;
    std::queue<NFA::State *> q;

    result.insert(states.begin(), states.end());
    for (auto state : states) {
        q.push(state);
    }


    while (!q.empty())
    {
        State *curState = q.front();
        q.pop();
        if (curState->transition.find(EPSILON) == curState->transition.end())
        {
            continue;
        }
        for (State *accessibleState : curState->transition[EPSILON])
        {
            if (result.find(accessibleState) == result.end())
            {
                result.insert(accessibleState);
                q.push(accessibleState);
            }
        }
    }
    return result;
}

/**
 * Get the set of neighbor states from the closure of starting state through char c
 * @param closure
 * @param c
 * @return
 */
std::set<NFA::State*> NFA::move(std::set<NFA::State*> closure, char c) {
    std::set<NFA::State*> result;
    for (NFA::State* element : closure) {
        if (element->transition.find(c) != element->transition.end()) {
            result.insert(element->transition[c].begin(), element->transition[c].end());
        }
    }
    return result;
}

void NFA::print() {
    printf("NFA:\n");
    for (auto state : iter_states())
        state->print();
}

/**
 * BFS Traversal
 */
std::vector<NFA::State*> NFA::iter_states() {
    std::vector<State*> states{};
    states.emplace_back(start);
    std::queue<State*> states_to_go{};
    states_to_go.emplace(start);
    std::set<State*> visited_states = {start};
    while(!states_to_go.empty()) {
        State* state = states_to_go.front();
        states_to_go.pop();
        for (auto trans : state->transition) {
            for (auto neighbor : trans.second) {
                if (visited_states.find(neighbor) == visited_states.end()) {
                    states_to_go.emplace(neighbor);
                    visited_states.insert(neighbor);
                    states.emplace_back(neighbor);
                }
            }
        }
    }
    return states;
}

/**
 * Constructor: Scanner
 * @usage: Scanner origin;
 *         Scanner scanner(reserved_word_strs, token_strs, reserced_word_num, token_num);
 * --------------------------------------------------------------------------------------
 * Create a Scanner object. The default constructor will not be used, and the second form
 * will create the NFA and DFA machines based on the given reserved words and tokens
 */
Scanner::Scanner() {
    nfa = new NFA(' ');
}

/**
 * Given a filename of a source program, print all the tokens of it
 * @param {string} filename
 * @return 0 for success, -1 for failure
 */ 
int Scanner::scan(std::string &filename) {
    // Open the file
    std::ifstream file(filename);
    std::ofstream output("scannerOutput.txt");
    if (!file.is_open())
    {
        std::cerr << "Error: Unable to open file: " << filename << std::endl;
        return -1; // Return -1 for failure
    }
    unsigned int line_number = 1;
    char c;
    std::string token = "";
    DFA::State *current_state = dfa->states[0]; // Start from the initial state

    // Read the file character by character
    while (file.get(c)){
        // std::cout << "current char:" << c << std::endl;
        if (isspace(c) && current_state == dfa->states[0]) continue;
        
        // Check if the current state has a transition for the character
        if (current_state->transition.find(c) != current_state->transition.end()) {
            // Update the current state
            current_state = current_state->transition[c];
            token += c;
        }
        else {
            file.unget();
            // If no transition is available, check if the current state is an accepting state
            if (current_state->accepted) {
                // Print the token class
                // std::cout << "Token Class: " << token_class_to_str(current_state->token_class) << ": " << token << std::endl;
                output << token_class_to_str(current_state->token_class) << " " << token << std::endl;
                token = "";
                // Reset the DFA state to the initial state
                current_state = dfa->states[0];
            }
            else {
                // If not an accepting state, print an error message
                std::cerr << "Error: Unrecognized token at line " << line_number << "token: "  << token << std::endl;
                token = "";
                current_state = dfa->states[0];
                return -1;
            }
        }

        // Increment line number when encountering newline character
        if (c == '\n')
        {
            line_number++;
        }
    }

    if (current_state->accepted)
    {
        // Print the token class
        // std::cout << "Token Class: " << token_class_to_str(current_state->token_class) << ": " << token << std::endl;
        output << token_class_to_str(current_state->token_class) << " " << token << std::endl;
        token = "";
        // Reset the DFA state to the initial state
        current_state = dfa->states[0];
    }
    else
    {
        // If not an accepting state, print an error message
        std::cerr << "Error: Unrecognized token at line " << line_number << "token: " << token << std::endl;
        token = "";
        current_state = dfa->states[0];
        return -1;
    }

    // Close the file
    output.close();
    file.close();

    return 0;
}

/**
 * Add string tokens, usually for reserved words, punctuations, and operators
 * @param token_str: exact string to match for token recognition
 * @param token_class
 * @param precedence: precedence of token, especially for operators
 * @return
 */
void Scanner::add_token(std::string token_str, TokenClass token_class, unsigned int precedence) {
    auto keyword_nfa = NFA::from_string(token_str);
    keyword_nfa->set_token_class_for_end_state(token_class, precedence);
    nfa->set_union(keyword_nfa);
}

/**
 * Token: ID (Identifier)
 * RegExp: [a-zA-Z][a-zA-Z0-9_]*
 * @param token_class
 * @param precedence
 * @return
 */
void Scanner::add_identifier_token(TokenClass token_class, unsigned int precedence) {
    NFA* idNFA = NFA::from_letter();
    NFA* secondPart = NFA::from_letter();
    secondPart->set_union(NFA::from_digit());
    secondPart->set_union(new NFA('_'));
    secondPart->kleene_star();
    idNFA->concat(secondPart);

    idNFA->set_token_class_for_end_state(token_class, precedence);
    nfa->set_union(idNFA);
}
/**
 * Token: INTEGER
 * RegExp: [0-9]+
 * Negative integer is recognized by unary operator MINUS
 * @param token_class
 * @param precedence
 * @return
 */
void Scanner::add_integer_token(TokenClass token_class, unsigned int precedence) {
    auto intLit = NFA::from_digit();
    auto temp = NFA::from_digit();
    temp->kleene_star();
    intLit->concat(temp);

    intLit->set_token_class_for_end_state(token_class, precedence);
    nfa->set_union(intLit);
}

/**
 * Token Class: STRINGLITERAL
 * RegExp: "(.|")*"
 * @param token_class
 * @param precedence
 * @return
 */
void Scanner::add_string_token(TokenClass token_class, unsigned int precedence) {
    NFA* stringLit = new NFA('"');
    // auto temp = NFA::from_any_char();
    NFA* temp = new NFA(char(0));
    std::set<NFA*> anyCharExceptQuote;
    for (int i = 1; i < 34; ++i) {
        anyCharExceptQuote.insert(new NFA(char(i)));
    }
    // except quote sign
    for (int i = 35; i < 128; ++i) {
        anyCharExceptQuote.insert(new NFA(char(i)));
    }
    temp->set_union(anyCharExceptQuote);
    temp->kleene_star();
    stringLit->concat(temp);
    stringLit->concat(new NFA('"'));

    stringLit->set_token_class_for_end_state(token_class, precedence);
    nfa->set_union(stringLit);
}

/**
 * Token Class: COMMENT
 * RegExp: //\s(.)*
 * @param token_class
 * @param precedence
 * @return
 */
void Scanner::add_comment_token(TokenClass token_class, unsigned int precedence) {
    NFA* commentNFA = new NFA('/');
    commentNFA->concat(new NFA('*'));

    std::set<NFA *> anyCharExceptStarAndSlash;
    std::set<NFA*> anyCharExceptStar;
    NFA *group1 = new NFA(char(0));
    // except star sign
    for (int i = 1; i < 42; ++i)
    {
        anyCharExceptStarAndSlash.insert(new NFA(char(i)));
        anyCharExceptStar.insert(new NFA(char(i)));
    }
    // except slash sign
    for (int i = 43; i < 47; ++i)
    {
        anyCharExceptStarAndSlash.insert(new NFA(char(i)));
        anyCharExceptStar.insert(new NFA(char(i)));
    }
    for (int i = 48; i < 128; ++i)
    {
        anyCharExceptStarAndSlash.insert(new NFA(char(i)));
        anyCharExceptStar.insert(new NFA(char(i)));
    }
    anyCharExceptStar.insert(new NFA(char(47)));
    group1->set_union(anyCharExceptStar);

    NFA *group2 = new NFA(char(42));
    auto temp1 = new NFA(char(42));
    temp1->kleene_star();
    group2->concat(temp1);
    NFA* temp2 = new NFA(char(0));
    temp2->set_union(anyCharExceptStarAndSlash);
    group2->concat(temp2);

    group1->set_union(group2);
    group1->kleene_star();

    commentNFA->concat(group1);
    commentNFA->concat(new NFA(char(42)));
    auto temp3 = new NFA(char(42));
    temp3->kleene_star();
    commentNFA->concat(temp3);
    commentNFA->concat(new NFA(char(47)));

    commentNFA->set_token_class_for_end_state(token_class, precedence);
    nfa->set_union(commentNFA);
}
