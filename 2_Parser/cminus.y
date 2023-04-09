/****************************************************/
/* File: cminus.y                                     */
/* The C-MINUS Yacc/Bison specification file           */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/
%{
#define YYPARSER /* distinguishes Yacc output from other code files */

#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"

#define YYSTYPE TreeNode *
static int savedLineNo;  /* ditto */
static TreeNode * savedTree; /* stores syntax tree for later return */
static int yylex(void); // added 11/2/11 to ensure no conflict with lex

%}

%token IF ELSE WHILE RETURN INT VOID
%token ID NUM 
%token ASSIGN EQ NE LT LE GT GE PLUS MINUS TIMES OVER LPAREN RPAREN LBRACE RBRACE LCURLY RCURLY SEMI COMMA
%token ERROR

%% /* Grammar for C-MINUS */

program     : declaration_list
              { savedTree = $1;} 
            ;
declaration_list  : declaration_list declaration
                    {
                      YYSTYPE t = $1;
                      if (t != NULL)
                      {
                        while (t->sibling != NULL)
                            t = t->sibling;
                        t->sibling = $2;
                        $$ = $1; 
                      }
                      else $$ = $2;
                    }
                  | declaration { $$ = $1; }
                  ;
declaration : var_declaration { $$ = $1; }
            | fun_declaration { $$ = $1; }
            ;
var_declaration : type_specifier id SEMI
                  {
                    $$ = newDeclNode(VarK);
                    $$->type = $1->type;
                    $$->attr.name = $2->attr.name;
                  }
                | type_specifier id LBRACE num RBRACE SEMI
                  {
                    $$ = newDeclNode(VarK);
                    $$->type = $1->type + 1;
                    $$->attr.name = $2->attr.name;
                    $$->child[0] = $4;
                  }
                ;
type_specifier  : INT 
                  {
                    $$ = newExpNode(ConstK);
                    $$->type = Integer;
                  }
                | VOID
                  {
                    $$ = newExpNode(ConstK);
                    $$->type = Void;
                  }
                ;
fun_declaration : type_specifier id
                  LPAREN params RPAREN compound_stmt
                  {
                    $$ = newDeclNode(FuncK);
                    $$->type = $1->type;
                    $$->attr.name = $2->attr.name;
                    $$->child[0] = $4;
                    $$->child[1] = $6;
                  }
                ;
params      : param_list { $$ = $1; }
            | VOID { $$ = newParamNode(VoidK); }
            ;
param_list  : param_list COMMA param
              {
                YYSTYPE t = $1;
                if (t != NULL)
                {
                  while (t->sibling != NULL)
                      t = t->sibling;
                  t->sibling = $3;
                  $$ = $1; 
                }
                else $$ = $3;
              }
            | param { $$ = $1; }
            ;
param       : type_specifier id
              {
                $$ = newParamNode(NonVoidK);
                $$->type = $1->type;
                $$->attr.name = $2->attr.name;
              }
            | type_specifier id LBRACE RBRACE
              {
                $$ = newParamNode(NonVoidK);
                $$->type = $1->type + 1;
                $$->attr.name = $2->attr.name;
              }
            ;
compound_stmt : LCURLY local_declarations statement_list RCURLY
                {
                  $$ = newStmtNode(CompK);
                  $$->child[0] = $2;
                  $$->child[1] = $3;
                }
              ;
local_declarations  : local_declarations var_declaration
                      {
                        YYSTYPE t = $1;
                        if (t != NULL)
                        {
                          while (t->sibling != NULL)
                              t = t->sibling;
                          t->sibling = $2;
                          $$ = $1; 
                        }
                        else $$ = $2;
                      }
                    | /* empty */ { $$ = NULL; }
                    ;
statement_list  : statement_list statement
                  {
                    YYSTYPE t = $1;
                    if (t != NULL)
                    {
                      while (t->sibling != NULL)
                          t = t->sibling;
                      t->sibling = $2;
                      $$ = $1; 
                    }
                    else $$ = $2;
                  }
                | /* empty */ { $$ = NULL; }
                ;
statement   : open_statement { $$ = $1; }
            | closed_statement { $$ = $1; }
            ;
open_statement  : IF LPAREN expression RPAREN statement
                  {
                    $$ = newStmtNode(IfK);
                    $$->child[0] = $3;
                    $$->child[1] = $5;
                  }
                | IF LPAREN expression RPAREN closed_statement ELSE open_statement
                  {
                    $$ = newStmtNode(IfElseK);
                    $$->child[0] = $3;
                    $$->child[1] = $5;
                    $$->child[2] = $7;
                  }
                | WHILE LPAREN expression RPAREN open_statement
                  {
                    $$ = newStmtNode(WhileK);
                    $$->child[0] = $3;
                    $$->child[1] = $5;
                  }
                ;
closed_statement  : simple_statement { $$ = $1; }
                  | IF LPAREN expression RPAREN closed_statement ELSE closed_statement
                    {
                      $$ = newStmtNode(IfElseK);
                      $$->child[0] = $3;
                      $$->child[1] = $5;
                      $$->child[2] = $7;
                    }
                  | WHILE LPAREN expression RPAREN closed_statement
                    {
                      $$ = newStmtNode(WhileK);
                      $$->child[0] = $3;
                      $$->child[1] = $5;
                    }
                  ;
simple_statement  : expression_stmt { $$ = $1; }
                  | compound_stmt { $$ = $1; }
                  | return_stmt { $$ = $1; }
                  ;
expression_stmt : expression SEMI { $$ = $1; }
                | SEMI { $$ = NULL; }
                ;
return_stmt   : RETURN SEMI { $$ = newStmtNode(ReturnK); }
              | RETURN expression SEMI
                {
                  $$ = newStmtNode(ReturnK);
                  $$->child[0] = $2;
                }
              ;
expression    : var ASSIGN expression
                {
                  $$ = newExpNode(AssignK);
                  $$->child[0] = $1;
                  $$->child[1] = $3;
                }
              | simple_expression { $$ = $1; }
              ;
var           : id
                {
                  $$ = newExpNode(VarArrK);
                  $$->attr.name = $1->attr.name;
                }
              | id LBRACE expression RBRACE
                {
                  $$ = newExpNode(VarArrK);
                  $$->attr.name = $1->attr.name;
                  $$->child[0] = $3;
                }
              ;
simple_expression : additive_expression relop additive_expression
                    {
                      $$ = newExpNode(OpK);
                      $$->attr.op = (TokenType)$2;
                      $$->child[0] = $1;
                      $$->child[1] = $3;
                    }
                  | additive_expression { $$ = $1; }
                  ;
relop       : LE { $$ = (void *)LE; }
            | LT { $$ = (void *)LT; }
            | GT { $$ = (void *)GT; }
            | GE { $$ = (void *)GE; }
            | EQ { $$ = (void *)EQ; }
            | NE { $$ = (void *)NE; }
            ;
additive_expression : additive_expression addop term
                      {
                        $$ = newExpNode(OpK);
                        $$->attr.op = (TokenType)$2;
                        $$->child[0] = $1;
                        $$->child[1] = $3;
                      }
                    | term { $$ = $1; }
                    ;
addop       : PLUS { $$ = (void *)PLUS; }
            | MINUS { $$ = (void *)MINUS; }
            ;
term        : term mulop factor
              {
                $$ = newExpNode(OpK);
                $$->attr.op = (TokenType)$2;
                $$->child[0] = $1;
                $$->child[1] = $3;
              }
            | factor { $$ = $1; }
            ;
mulop       : TIMES { $$ = (void *)TIMES; }
            | OVER { $$ = (void *)OVER; }
            ;
factor      : LPAREN expression RPAREN { $$ = $2; }
            | var { $$ = $1; }
            | call{ $$ = $1; }
            | num { $$ = $1; }
            ;
call        : id LPAREN args RPAREN
              {
                $$ = newExpNode(CallK);
                $$->attr.name = $1->attr.name;
                $$->child[0] = $3;
              }
            ;
args        : arg_list { $$ = $1; }
            | /* empty */ { $$ = NULL; }
            ;
arg_list    : arg_list COMMA expression
              {
                YYSTYPE t = $1;
                if (t != NULL)
                {
                  while (t->sibling != NULL)
                      t = t->sibling;
                  t->sibling = $3;
                  $$ = $1; 
                }
                else $$ = $3;
              }
            | expression { $$ = $1; }
            ;
id          : ID
              {
                $$ = newExpNode(VarArrK);
                $$->attr.name = copyString(tokenString);
                //printf("%s\n", $$->attr.name);
              }
            ;
num         : NUM
              {
                $$ = newExpNode(ConstK);
                $$->attr.val = atoi(tokenString);
              }
            ;

%%

int yyerror(char * message)
{ fprintf(listing,"Syntax error at line %d: %s\n",lineno,message);
  fprintf(listing,"Current token: ");
  printToken(yychar,tokenString);
  Error = TRUE;
  return 0;
}

/* yylex calls getToken to make Yacc/Bison output
 * compatible with ealier versions of the C-MINUS scanner
 */
static int yylex(void)
{ return getToken(); }

TreeNode * parse(void)
{ yyparse();
  return savedTree;
}

