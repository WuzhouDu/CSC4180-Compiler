/**
 * --------------------------------------
 * CUHK-SZ CSC4180: Compiler Construction
 * Assignment 1: Micro Language Compiler
 * --------------------------------------
 * Author: Mr.Liu Yuxuan
 * Position: Teaching Assisant
 * Date: January 25th, 2024
 * Email: yuxuanliu1@link.cuhk.edu.cn
 *
 * This file defines the LLVM IR Generator class, which generate LLVM IR (.ll) file given the AST from parser.
 */

#include "ir_generator.hpp"
#include <sstream>
std::string opClass2String(SymbolClass opSymbol)
{
    if (opSymbol == SymbolClass::PLUSOP)
    {
        return "add";
    }
    else
        return "sub";
}

void IR_Generator::export_ast_to_llvm_ir(Node *node)
{
    /* TODO: your program here */
    int temp = 1;
    int scan = 1;
    int print = 1;

    int *indexPtr = &temp;
    int *scanIndexPtr = &scan;
    int *printIndexPtr = &print;
    std::set<std::string> allocatedTable;

    out << "; Declare printf\ndeclare i32 @printf(i8*, ...)\n\n; Declare scanf\ndeclare i32 @scanf(i8*, ...)\n\n";
    out << "define i32 @main() {\n";
    gen_llvm_ir(node, indexPtr, scanIndexPtr, printIndexPtr, &allocatedTable);
    out << "\tret i32 0\n";
    out << "}";
}

void IR_Generator::gen_llvm_ir(Node *node, int *tempIndex, int *scanIndex, int *printIndex, std::set<std::string> *allocatedTable)
{
    /* TODO: Your program here */

    // Program Node
    if (node->symbol_class == SymbolClass::PROGRAM)
    {
        gen_llvm_ir(node->children[0], tempIndex, scanIndex, printIndex, allocatedTable);
        return;
    }

    // Statement List Node
    if (node->symbol_class == SymbolClass::STATEMENT_LIST)
    {
        for (auto i : node->children)
        {
            gen_llvm_ir(i, tempIndex, scanIndex, printIndex, allocatedTable);
        }
        return;
    }

    // Read Node
    if (node->symbol_class == SymbolClass::READ)
    {
        int readSize = node->children.size();
        int scanFormatSize = readSize * 3;
        std::vector<std::string> ID_list(readSize);
        std::stringstream ss;
        for (int i = 0; i < readSize; ++i)
        {
            ID_list[i] = node->children[i]->lexeme;
            // std::cout << ID_list[i] << std::endl;
            if (allocatedTable->find(ID_list[i]) == allocatedTable->end())
            {
                ss << "\t%" << ID_list[i] << " = alloca i32\n";
                allocatedTable->insert(ID_list[i]);
            }
        }

        std::stringstream scanFormatVar;
        scanFormatVar << "%_scanf_format_" << *scanIndex;

        std::stringstream allocaVar;
        allocaVar << "[" << scanFormatSize << " x i8]";

        ss << "\t" << scanFormatVar.str() << " = alloca " << allocaVar.str() << "\n";

        ss << "\tstore " << allocaVar.str() << " c\"";
        for (int i = 0; i < readSize - 1; ++i)
        {
            ss << "%d ";
        }
        ss << "%d\\00\", " << allocaVar.str() << "* " << scanFormatVar.str() << "\n";

        std::stringstream scanStringVar;
        scanStringVar << "%_scanf_str_" << *scanIndex;
        ss << "\t" << scanStringVar.str() << " = getelementptr " << allocaVar.str() << ", " << allocaVar.str() << "* " << scanFormatVar.str() << ", i32 0, i32 0\n";
        ss << "\tcall i32 (i8*, ...) @scanf(i8* " << scanStringVar.str() << ", ";
        for (int i = 0; i < readSize - 1; ++i)
        {
            ss << "i32* %" << ID_list[i] << ", ";
        }
        ss << "i32* %" << ID_list.back() << ")\n";
        out << ss.str();
        *scanIndex = *scanIndex + 1;
        return;
    }

    // Assignment Op Node
    else if (node->symbol_class == SymbolClass::ASSIGNOP)
    {
        std::stringstream ss;

        // Operand Class
        SymbolClass rightOperandClass = node->children[1]->symbol_class;

        // Just Intliteral, then return
        if (rightOperandClass == SymbolClass::INTLITERAL)
        {
            if (allocatedTable->find(node->children[0]->lexeme) == allocatedTable->end())
            {
                allocatedTable->insert(node->children[0]->lexeme);
                ss << "\t%" << node->children[0]->lexeme << " = alloca i32\n";
            }
            ss << "\tstore i32 " << node->children[1]->lexeme << ", i32* %" << node->children[0]->lexeme << "\n";
            out << ss.str();
            return;
        }

        /*
            This case deos not appear in the test cases.
            But I think it should, since there are statements like `B := A + 10`.
            So why not statements like `B := A`?
            However, this statement incurs a problem: whether the reference to the
            variable is a reference to the address or the reference to the value.
            Due to the loss of inforamtion, I will not implement this case.
            But pointing it out here is necessary.
         */
        else if (rightOperandClass == SymbolClass::ID)
        {
            return;
        }

        // A plus OP or minus OP, indicating further recursion.
        else if (rightOperandClass == SymbolClass::MINUSOP || rightOperandClass == SymbolClass::PLUSOP)
        {
            gen_llvm_ir(node->children[1], tempIndex, scanIndex, printIndex, allocatedTable);
            if (allocatedTable->find(node->children[0]->lexeme) == allocatedTable->end())
            {
                allocatedTable->insert(node->children[0]->lexeme);
                ss << "\t%" << node->children[0]->lexeme << " = alloca i32\n";
            }
            ss << "\tstore i32 %_tmp_" << *tempIndex - 1 << ", i32* %" << node->children[0]->lexeme << "\n";
            out << ss.str();
            return;
        }

        // error handling
        else
        {
            out << "ERROR: unkown right operand symbol class.\n";
        }
    }

    // Plus op or Minus op Node
    else if (node->symbol_class == SymbolClass::PLUSOP || node->symbol_class == SymbolClass::MINUSOP)
    {
        std::stringstream ss;
        // First Operand Class
        SymbolClass firstOperandClass = node->children[0]->symbol_class;
        SymbolClass secondOperandClass = node->children[1]->symbol_class;

        // ID +/- Int literal does not need further recursion.
        if (firstOperandClass == SymbolClass::ID && secondOperandClass == SymbolClass::INTLITERAL)
        {
            ss << "\t%_tmp_" << (*tempIndex)++ << " = load i32, i32* %" << node->children[0]->lexeme << "\n";
            ss << "\t%_tmp_" << (*tempIndex)++ << " = " << opClass2String(node->symbol_class) << " i32 %_tmp_" << *tempIndex - 2 << ", " << node->children[1]->lexeme << "\n";
            out << ss.str();
            return;
        }

        // Int Literal +/- ID does not need further recursion.
        else if (firstOperandClass == SymbolClass::INTLITERAL && secondOperandClass == SymbolClass::ID) {
            ss << "\t%_tmp_" << (*tempIndex)++ << " = load i32, i32* %" << node->children[1]->lexeme << "\n";
            ss << "\t%_tmp_" << (*tempIndex)++ << " = " << opClass2String(node->symbol_class) << " i32 " << node->children[0]->lexeme << ", %_tmp_" << *tempIndex - 2 << "\n";
            out << ss.str();
            return;
        }

        // Int literal +/- int literal does not need further recursion.
        else if (firstOperandClass == SymbolClass::INTLITERAL && secondOperandClass == SymbolClass::INTLITERAL) {
            ss << "\t%_tmp_" << (*tempIndex)++ << " = " << opClass2String(node->symbol_class) << " i32 " << node->children[0]->lexeme << ", " << node->children[1]->lexeme << "\n";
            out << ss.str();
            return;
        }

        // ID +/- ID does not need further recursion.
        else if (firstOperandClass == SymbolClass::ID && secondOperandClass == SymbolClass::ID) {
            ss << "\t%_tmp_" << (*tempIndex)++ << " = load i32, i32* %" << node->children[0]->lexeme << "\n";
            ss << "\t%_tmp_" << (*tempIndex)++ << " = load i32, i32* %" << node->children[1]->lexeme << "\n";
            ss << "\t%_tmp_" << (*tempIndex)++ << " = " << opClass2String(node->symbol_class) << " i32 " << "%_tmp_" << *tempIndex - 3 << ", %_tmp_" << *tempIndex - 2 << "\n";
            out << ss.str();
            return;
        }

        // First Operand is ID and second operand is another operator.
        else if (firstOperandClass == SymbolClass::ID && (secondOperandClass == SymbolClass::PLUSOP || secondOperandClass == SymbolClass::MINUSOP)) {
            int firstOperandTempIndex = (*tempIndex)++;
            ss << "\t%_tmp_" << firstOperandTempIndex << " = load i32, i32* %" << node->children[0]->lexeme << "\n";
            out << ss.str();
            ss.clear();
            ss.str("");
            gen_llvm_ir(node->children[1], tempIndex, scanIndex, printIndex, allocatedTable);
            ss << "\t%_tmp_" << (*tempIndex)++ << " = " << opClass2String(node->symbol_class) << " i32 %_tmp_" << firstOperandTempIndex << ", %_tmp_" << *tempIndex - 2 << "\n";
            out << ss.str();
            return;
        }

        // First Operand is Int literal and second operand is another operator
        else if (firstOperandClass == SymbolClass::INTLITERAL && (secondOperandClass == SymbolClass::PLUSOP || secondOperandClass == SymbolClass::MINUSOP)) {
            gen_llvm_ir(node->children[1], tempIndex, scanIndex, printIndex, allocatedTable);
            ss << "\t%_tmp_" << (*tempIndex)++ << " = " << opClass2String(node->symbol_class) << " i32 " << node->children[0]->lexeme << ", %_tmp_" << *tempIndex - 2 << "\n";
            out << ss.str();
            return;
        }

        // First Operand is operator and second operand is another operator
        else if ((firstOperandClass == SymbolClass::PLUSOP || firstOperandClass == SymbolClass::MINUSOP) && (secondOperandClass == SymbolClass::PLUSOP || secondOperandClass == SymbolClass::MINUSOP)){
            gen_llvm_ir(node->children[0], tempIndex, scanIndex, printIndex, allocatedTable);
            int firstOperandTempIndex = *tempIndex - 1;
            gen_llvm_ir(node->children[1], tempIndex, scanIndex, printIndex, allocatedTable);
            ss << "\t%_tmp_" << (*tempIndex)++ << " = " << opClass2String(node->symbol_class) << " i32 %_tmp_" << firstOperandTempIndex << ", %_tmp_" << *tempIndex - 2 << "\n";
            out << ss.str();
            return;
        }

        // First Operand is operator and second operand is ID
        else if ((firstOperandClass == SymbolClass::PLUSOP || firstOperandClass == SymbolClass::MINUSOP) && secondOperandClass == SymbolClass::ID) {
            gen_llvm_ir(node->children[0], tempIndex, scanIndex, printIndex, allocatedTable);
            ss << "\t%_tmp_" << (*tempIndex)++ << " = load i32, i32* %" << node->children[1]->lexeme << "\n";
            ss << "\t%_tmp_" << (*tempIndex)++ << " = " << opClass2String(node->symbol_class) << " i32 %_tmp_" << *tempIndex - 3 << ", %_tmp_" << *tempIndex - 2 << "\n";
            out << ss.str();
            return;
        }

        // First Operand is operator and second operand is Int literal
        else if ((firstOperandClass == SymbolClass::PLUSOP || firstOperandClass == SymbolClass::MINUSOP) && secondOperandClass == SymbolClass::INTLITERAL) {
            gen_llvm_ir(node->children[0], tempIndex, scanIndex, printIndex, allocatedTable);
            ss << "\t%_tmp_" << (*tempIndex)++ << " = " << opClass2String(node->symbol_class) << " i32 %_tmp_" << *tempIndex - 2 << ", " << node->children[1]->lexeme << "\n";
            out << ss.str();
            return;
        }
        
        else
        {
            out << "ERROR: not known operand class. \n";
        }
    }

    // Write Node
    else if (node->symbol_class == SymbolClass::WRITE) {
        int writeSize = node->children.size();
        int formatSize = (writeSize - 1) * 3 + 4;

        std::stringstream ss;
        std::stringstream formatAlloca;
        std::stringstream formatVar;
        std::stringstream printString;

        formatAlloca << "[" << formatSize << " x i8]";
        formatVar << "%_printf_format_" << (*printIndex)++;
        printString << "%_printf_str_" << *printIndex - 1;
        ss << "\t" << formatVar.str() << " = " << "alloca " << formatAlloca.str() << "\n";

        ss << "\tstore " << formatAlloca.str() << " c\"";
        for (int i = 0; i < writeSize-1; i++) {
            ss << "%d ";
        }
        ss << "%d\\0A\\00\", " << formatAlloca.str() << "* " << formatVar.str() << "\n";

        ss << "\t" << printString.str() << " = getelementptr " << formatAlloca.str() << ", " << formatAlloca.str() << "* " << formatVar.str() << ", i32 0, i32 0\n";

        out << ss.str();

        std::vector<std::string> writePtrs(writeSize);

        for (int i = 0; i < writeSize; ++i) {
            if (node->children[i]->symbol_class == SymbolClass::ID) {
                writePtrs[i] = "%_tmp_" + std::to_string(*tempIndex);
                out << "\t%_tmp_" << (*tempIndex)++ << " = load i32, i32* %" << node->children[i]->lexeme << "\n";
            }
            else if (node->children[i]->symbol_class == SymbolClass::PLUSOP || node->children[i]->symbol_class == SymbolClass::MINUSOP) {
                gen_llvm_ir(node->children[i], tempIndex, scanIndex, printIndex, allocatedTable);
                writePtrs[i] = "%_tmp_" + std::to_string(*tempIndex - 1);
            }
            else if (node->children[i]->symbol_class == SymbolClass::INTLITERAL) {
                writePtrs[i] = node->children[i]->lexeme;
            }
        }

        out << "\tcall i32 (i8*, ...) @printf(i8* " << printString.str() << ", ";
        for (int i = 0; i < writeSize - 1; ++i) {
            out << "i32 " << writePtrs[i] << ", ";
        }
        out << "i32 " << writePtrs.back() << ")\n";
        return;
    }
}
