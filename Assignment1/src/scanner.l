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
 * This file implements some lexical analysis rules to scan the input file 
 * and return the tokens which will be used for parser 
 
 */


%{
/* C/C++ Stuff (headers, declarations, variables, etc.) */


#include <cstdio>
#include <string>
#include <vector>
#include "parser.hpp"
#include "node.hpp"

int yyerror (char *s);

int yylex();

int over = 0;

/**
 * scan_only indicates whehter the flex scanner should print out the token information for debug.
 * The token information is in the following form:
 *   <token-class, lexeme>
 */
extern int scan_only;
%}

/* TODO: define some regular expressions here */
ID [a-zA-Z][a-zA-Z0-9_]{0,31}
INTLITERAL -?[0-9]+
COMMENT --.*\n

%%
{COMMENT}

begin {
        if (scan_only) printf("BEGIN_ begin\n");
        yylval.string = "begin";
        return BEGIN_;
    }
end {
        if (scan_only) printf("END end\n");
        yylval.string = "end";
        return END;
    }
read {
        if (scan_only) printf("READ read\n");
        yylval.string = "read";
        return READ;
    }
write {
        if (scan_only) printf("WRITE write\n");
        yylval.string = "write";
        return WRITE;
    }
\( {
        if (scan_only) printf("LPAREN (\n");
        yylval.string = "(";
        return LPAREN;
    }
\) {
        if (scan_only) printf("RPAREN )\n");
        yylval.string = ")";
        return RPAREN;
    }
\; {
        if (scan_only) printf("SEMICOLON ;\n");
        yylval.string = ";";
        return SEMICOLON;
    }
, {
        if (scan_only) printf("COMMA ,\n");
        yylval.string = ",";
        return COMMA;
    }
":=" {
        if (scan_only) printf("ASSIGNOP :=\n");
        yylval.string = ":=";
        return ASSIGNOP;
    }
\+ {
        if (scan_only) printf("PLUSOP +\n");
        yylval.string = "+";
        return PLUSOP;
    }
\- {
        if (scan_only) printf("MINUSOP -\n");
        yylval.string = "-";
        return MINUSOP;
    }
{ID} {
        if (scan_only) printf("ID %s\n", yytext);
        yylval.string = strdup(yytext);
        return ID;
    }
{INTLITERAL} {
        if (scan_only) printf("INTLITERAL %s\n", yytext);
        yylval.i32 = atoi(yytext);
        return INTLITERAL;
    }

<<EOF>> {
        if (over) {
            yyterminate();
        }
        else {
            over = 1;
            if (scan_only) printf("SCANEOF\n");
            yylval.string = "SCANEOF";
            return SCANEOF;
        }

    }

\n

.


%%

int yywrap (void) {return 1;}