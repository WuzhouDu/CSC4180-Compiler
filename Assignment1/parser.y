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
 * This file implements some syntax analysis rules and works as a parser
 * The grammar tree is generated based on the rules and MIPS Code is generated
 * based on the grammar tree and the added structures and functions implemented
 * in File: added_structure_function.c
 */

%{
/* C declarations used in actions */
#include <cstdio>     
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <ctype.h>

#include "node.hpp"

int yyerror (char *s);

int yylex();

extern int cst_only;

Node* root_node = nullptr;


%}

// TODO: define yylval data types with %union
%union{
	int i32;
	const char* string;
	struct Node* node;
}
// TODO: define terminal symbols with %token. Remember to set the type.
%token <string> BEGIN_ END READ WRITE LPAREN RPAREN SEMICOLON COMMA ASSIGNOP PLUSOP MINUSOP SCANEOF ID
%token <i32> INTLITERAL

// Start Symbol
/* %start start */

// TODO: define Non-Terminal Symbols with %type. Remember to set the type.
%type <node> start program statement_list statement expression id_list expr_list primary
%%
/**
 * Format:
 * Non-Terminal  :  [Non-Terminal, Terminal]+ (production rule 1)   { parser actions in C++ }
 *                  | [Non-Terminal, Terminal]+ (production rule 2) { parser actions in C++ }
 *                  ;
 */


// TODO: your production rule here
// The tree generation logic should be in the operation block of each production rule
	start : program SCANEOF {
								if (cst_only) {
									$$ = new Node(SymbolClass::START);
									Node* eof_node = new Node(SymbolClass::SCANEOF, $2);
									$$->append_child($1);
									$$->append_child(eof_node);
									root_node = $$;
								}
								else {
									root_node = $1;
								}

							}
		  ;

	program: BEGIN_ statement_list END {
											if (cst_only){
												$$ = new Node(SymbolClass::PROGRAM);
												Node* begin_node = new Node(SymbolClass::BEGIN_, $1);
												Node* end_node = new Node(SymbolClass::END, $3);
												$$->append_child(begin_node);
												$$->append_child($2);
												$$->append_child(end_node);
											}
											else {
												$$ = new Node(SymbolClass::PROGRAM);
												$$->append_child($2);
											}
										}
			;

	statement_list: statement {
								if (cst_only) {
									$$ = new Node(SymbolClass::STATEMENT_LIST);
									$$->append_child($1);
								}
								else {
									$$ = new Node(SymbolClass::STATEMENT_LIST);
									$$->append_child($1);
								}
							  }
					| statement_list statement {
													if (cst_only) {
														$$ = new Node(SymbolClass::STATEMENT_LIST);
														$$->append_child($1);
														$$->append_child($2);
													}
													else {
														$$ = new Node(SymbolClass::STATEMENT_LIST);
														for (int i = 0; i < $1->children.size(); i++) {
															$$->append_child($1->children[i]);
														}
														$$->append_child($2);
													}
					}
					;

	statement: ID ASSIGNOP expression SEMICOLON {
													if (cst_only) {
														$$ = new Node(SymbolClass::STATEMENT);
														Node* ID_node = new Node(SymbolClass::ID, $1);
														Node* assign_node = new Node(SymbolClass::ASSIGNOP, $2);
														Node* semicol_node = new Node(SymbolClass::SEMICOLON, $4);
														$$->append_child(ID_node);
														$$->append_child(assign_node);
														$$->append_child($3);
														$$->append_child(semicol_node);
													}
													else {
														$$ = new Node(SymbolClass::ASSIGNOP, $2);
														$$->append_child(new Node(SymbolClass::ID, $1));
														// 这里expression是啥待会儿回来看！！！！！
														$$->append_child($3);
													}
	}
			| READ LPAREN id_list RPAREN SEMICOLON {
														if (cst_only) {
															$$ = new Node(SymbolClass::STATEMENT);
															Node* read_node = new Node(SymbolClass::READ, $1);
															Node* lparen_node = new Node(SymbolClass::LPAREN, $2);
															Node* rparen_node = new Node(SymbolClass::RPAREN, $4);
															Node* semicol_node = new Node(SymbolClass::SEMICOLON, $5);
															$$->append_child(read_node);
															$$->append_child(lparen_node);
															$$->append_child($3);
															$$->append_child(rparen_node);
															$$->append_child(semicol_node);
														}
														else {
															$$ = new Node(SymbolClass::READ, $1);
															// 这里先假设id_list的node是父节点跟着一堆子节点ID
															for (int i = 0; i < $3->children.size(); i++) {
																$$->append_child($3->children[i]);
															}
														}
			}
			| WRITE LPAREN expr_list RPAREN SEMICOLON {
														if (cst_only) {
															$$ = new Node(SymbolClass::STATEMENT);
															Node* write_node = new Node(SymbolClass::WRITE, $1);
															Node* lparen_node = new Node(SymbolClass::LPAREN, $2);
															Node* rparen_node = new Node(SymbolClass::RPAREN, $4);
															Node* semicol_node = new Node(SymbolClass::SEMICOLON, $5);
															$$->append_child(write_node);
															$$->append_child(lparen_node);
															$$->append_child($3);
															$$->append_child(rparen_node);
															$$->append_child(semicol_node);
														}
														else {
															$$ = new Node(SymbolClass::WRITE, $1);
															// 这里假设expr_list的node是父节点跟着子节点一堆expression, expression现在是什么我也不知道，先写着。
															for (int i = 0; i < $3->children.size(); i++) {
																$$->append_child($3->children[i]);
															}
														}

			}
			;
	
	id_list: ID {
					$$ = new Node(SymbolClass::ID_LIST);
					$$->append_child(new Node(SymbolClass::ID, $1));
	}
			| id_list COMMA ID {
									if (cst_only) {
										$$ = new Node(SymbolClass::ID_LIST);
										Node* comma_node = new Node(SymbolClass::COMMA, $2);
										Node* id_node = new Node(SymbolClass::ID, $3);
										$$->append_child($1);
										$$->append_child(comma_node);
										$$->append_child(id_node);
									}
									else {
										$$ = new Node(SymbolClass::ID_LIST);
										for (int i = 0; i < $1->children.size(); i++) {
											$$->append_child($1->children[i]);
										}
										$$->append_child(new Node(SymbolClass::ID, $3));
									}
			}
			;

	expr_list: expression {
							if (cst_only) {
								$$ = new Node(SymbolClass::EXPRESSION_LIST);
								$$->append_child($1);
							}
							else {
								// 这里不确定expression是啥，暂时先这么写。
								$$ = new Node(SymbolClass::EXPRESSION_LIST);
								$$->append_child($1);
							}
							

	}
			| expr_list COMMA expression {
											if (cst_only) {
												$$ = new Node(SymbolClass::EXPRESSION_LIST);
												$$->append_child($1);
												Node* comma_node = new Node(SymbolClass::COMMA, $2);
												$$->append_child(comma_node);
												$$->append_child($3);
											}
											else {
												$$ = new Node(SymbolClass::EXPRESSION_LIST);
												for (int i = 0; i < $1->children.size(); i++) {
													$$->append_child($1->children[i]);
												}
												$$->append_child($3);
											}
			}
			;
	
	expression: primary {
		if (cst_only) {
			$$ = new Node(SymbolClass::EXPRESSION);
			$$->append_child($1);
		}
		else {
			// 不确定primary是啥，先这么写。
			$$ = $1;
		}

	}
			// 为什么这里 expression PLUSOP primary 和 primary PLUSOP expression结果不一样🤔, 如果对testcase2.m修改版进行测试的话会很明显。
			| expression PLUSOP primary {
				if (cst_only) {
					$$ = new Node(SymbolClass::EXPRESSION);
					$$->append_child($1);
					Node* plus_node = new Node(SymbolClass::PLUSOP, $2);
					$$->append_child(plus_node);
					$$->append_child($3);
				}
				else {
					$$ = new Node(SymbolClass::PLUSOP, $2);
					$$->append_child($1);
					$$->append_child($3);
				}

			}
			| expression MINUSOP primary {
				if (cst_only){
					$$ = new Node(SymbolClass::EXPRESSION);
					$$->append_child($1);
					Node* minus_node = new Node(SymbolClass::MINUSOP, $2);
					$$->append_child(minus_node);
					$$->append_child($3);
				}
				else {
					$$ = new Node(SymbolClass::MINUSOP, $2);
					$$->append_child($1);
					$$->append_child($3);
				}
			}
			;

	primary: LPAREN expression RPAREN {
		if (cst_only) {
			$$ = new Node(SymbolClass::PRIMARY);
			Node* lparen_node = new Node(SymbolClass::LPAREN, $1);
			$$->append_child(lparen_node);
			$$->append_child($2);
			Node* rparen_node = new Node(SymbolClass::RPAREN, $3);
			$$->append_child(rparen_node);
		}
		else {
			$$ = $2;
		}
	}
			| ID {
				if (cst_only) {
					$$ = new Node(SymbolClass::PRIMARY);
					$$->append_child(new Node(SymbolClass::ID, $1));
				}
				else {
					$$ = new Node(SymbolClass::ID, $1);
				}

			}
			| INTLITERAL {
				if (cst_only) {
					$$ = new Node(SymbolClass::PRIMARY);
					$$->append_child(new Node(SymbolClass::INTLITERAL, std::to_string($1)));
				}
				else {
					$$ = new Node(SymbolClass::INTLITERAL, std::to_string($1));
				}

			}
			;
		


%%
int yyerror(char *s) {
	printf("Syntax Error on line %s\n", s);
	return 0;
}
