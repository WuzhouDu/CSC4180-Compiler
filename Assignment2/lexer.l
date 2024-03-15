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
 * File: lexer.l
 * -----------------------------
 * This file implements some lexical analysis rules to scan the input file 
 * and return the tokens which will be used later for parser
 */

%{
/* C/C++ Stuff (headers, declarations, variables, etc.) */
#include <cstdio>
#include <string>
#include <cstring>
#include <vector>
#include <iostream>
#include <fstream>
#include <ios>
#include "tokens.hpp"

int yyerror (char *s);

int yylex();

std::ofstream outputFile;

void printToken(TokenClass tokenType, char* lexeme);


%}

integer [0-9]+
id [a-zA-Z][a-zA-Z0-9_]*
comment \/\*([^*]|\*+[^*/])*\*+\/
string \"(.)*\"

%%
{comment} {
  printToken(TokenClass::COMMENT, yytext);
}

null {
  printToken(TokenClass::NUL, yytext);
}

true {
  printToken(TokenClass::TRUE, yytext);
}

false {
  printToken(TokenClass::FALSE, yytext);
}

void {
  printToken(TokenClass::TVOID, yytext);
}

int {
  printToken(TokenClass::TINT, yytext);
}

string {
  printToken(TokenClass::TSTRING, yytext);
}

bool {
  printToken(TokenClass::TBOOL, yytext);
}

if {
  printToken(TokenClass::IF, yytext);
}

else {
  printToken(TokenClass::ELSE, yytext);
}

while {
  printToken(TokenClass::WHILE, yytext);
}

for {
  printToken(TokenClass::FOR, yytext);
}

return {
  printToken(TokenClass::RETURN, yytext);
}

new {
  printToken(TokenClass::NEW, yytext);
}

var {
  printToken(TokenClass::VAR, yytext);
}

global {
  printToken(TokenClass::GLOBAL, yytext);
}

; {
  printToken(TokenClass::SEMICOLON, yytext);
}

, {
  printToken(TokenClass::COMMA, yytext);
}

\{ {
  printToken(TokenClass::LBRACE, yytext);
}

\} {
  printToken(TokenClass::RBRACE, yytext);
}

\( {
  printToken(TokenClass::LPAREN, yytext);
}

\) {
  printToken(TokenClass::RPAREN, yytext);
}

\[ {
  printToken(TokenClass::LBRACKET, yytext);
}

\] {
  printToken(TokenClass::RBRACKET, yytext);
}

\* {
  printToken(TokenClass::STAR, yytext);
}

\+ {
  printToken(TokenClass::PLUS, yytext);
}

\- {
  printToken(TokenClass::MINUS, yytext);
}

\<< {
  printToken(TokenClass::LSHIFT, yytext);
}

\>> {
  printToken(TokenClass::RLSHIFT, yytext);
}

\>>> {
  printToken(TokenClass::RASHIFT, yytext);
}

\< {
  printToken(TokenClass::LESS, yytext);
}

\<= {
  printToken(TokenClass::LESSEQ, yytext);
}

\> {
  printToken(TokenClass::GREAT, yytext);
}

\>= {
  printToken(TokenClass::GREATEQ, yytext);
}

\== {
  printToken(TokenClass::EQ, yytext);
}

\!= {
  printToken(TokenClass::NEQ, yytext);
}

\& {
  printToken(TokenClass::LAND, yytext);
}

\| {
  printToken(TokenClass::LOR, yytext);
}

\[\&\] {
  printToken(TokenClass::BAND, yytext);
}

\[\|\] {
  printToken(TokenClass::BOR, yytext);
}

\! {
  printToken(TokenClass::NOT, yytext);
}

\~ {
  printToken(TokenClass::TILDE, yytext);
}

= {
  printToken(TokenClass::ASSIGN, yytext);
}

{id} {
  printToken(TokenClass::ID, yytext);
}

{integer} {
  printToken(TokenClass::INTLITERAL, yytext);
}

{string} {
  printToken(TokenClass::STRINGLITERAL, yytext);
}

.
\n

%%

void printToken(TokenClass tokenType, char* lexeme) {
  outputFile << token_class_to_str(tokenType) << " " << lexeme << "\n";
  if (outputFile.is_open()) std::cout << token_class_to_str(tokenType) << " " << lexeme << "\n";
}

int main(int argc, char **argv)
{
  std::cout << argc;
  outputFile.open("lexOutput.txt", std::ios_base::out);
  yylex();
  yywrap();
  outputFile.close();
}

int yywrap (void) {return 1;}