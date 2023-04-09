/****************************************************/
/* File: analyze.c                                  */
/* Semantic analyzer implementation                 */
/* for the C-MINUS compiler                         */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "util.h"
#include "symtab.h"
#include "analyze.h"

static ScopeList currentScope = NULL;
int DEBUG = FALSE;

void redefSymbol(char * name, int lineno){
  fprintf(listing, "Error: Symbol \"%s\" is redefined at line %d\n", name, lineno);
  Error = TRUE;
}

void undeclFunc(char * name, int lineno){
  fprintf(listing, "Error: Undeclared function \"%s\" is called at line %d\n", name, lineno);
  Error = TRUE;
}

void undeclVar(char * name, int lineno){
  fprintf(listing, "Error: Undeclared variable \"%s\" is used at line %d\n", name, lineno);
  Error = TRUE;
}

void voidVar(char * name, int lineno){
  fprintf(listing, "Error: The void-type variable is declared at line %d (name : \"%s\")\n", lineno, name);
  Error = TRUE;
}

void nonIntIdx(char * name, int lineno){
  fprintf(listing, "Error: Invalid array indexing at line %d (name : \"%s\"). Indices should be integer\n", lineno, name);
  Error = TRUE;
}

void nonArrIdx(char * name, int lineno){
  fprintf(listing, "Error: Invalid array indexing at line %d (name : \"%s\"). Indexing can only be allowed for int[] variables\n", lineno, name);
  Error = TRUE;
}

void invalidFuncCall(char * name, int lineno){
  fprintf(listing, "Error: Invalid function call at line %d (name : \"%s\")\n", lineno, name);
  Error = TRUE;
}

void invalidRet(int lineno){
  fprintf(listing, "Error: Invalid return at line %d\n", lineno);
  Error = TRUE;
}

void invalidAssign(int lineno){
  fprintf(listing, "Error: Invalid assignment at line %d\n", lineno);
  Error = TRUE;
}

void invalidOp(int lineno){
  fprintf(listing, "Error: Invalid operation at line %d\n", lineno);
  Error = TRUE;
}

void invalidCond(int lineno){
  fprintf(listing, "Error: Invalid condition at line %d\n", lineno);
  Error = TRUE;
}

/* Procedure traverse is a generic recursive 
 * syntax tree traversal routine:
 * it applies preProc in preorder and postProc 
 * in postorder to tree pointed to by t
 */
static void traverse( TreeNode * t,
               void (* preProc) (TreeNode *),
               void (* postProc) (TreeNode *) )
{ if (t != NULL)
  { preProc(t);
    { int i;
      for (i=0; i < MAXCHILDREN; i++)
        traverse(t->child[i],preProc,postProc);
    }
    postProc(t);
    traverse(t->sibling,preProc,postProc);
  }
}

/* Procedure insertNode inserts 
 * identifiers stored in t into 
 * the symbol table 
 */
static void insertNode( TreeNode * t)
{ BucketList l = NULL;
  switch (t->nodekind)
  { case StmtK:
      switch (t->kind.stmt)
      { case CompK:
          if(currentScope->isFunc == FALSE){
            int compidx = currentScope->compidx++;
            char * scopeName = NULL;

            scopeName = copyStringWithIdx(currentScope->name, compidx);
            t->attr.name = scopeName;

            ScopeList s = create_scope(scopeName, currentScope);
            currentScope = s;
            if(DEBUG) fprintf(listing, "Current scope changed to %s from parent %s\n", currentScope->name, currentScope->parent->name);
          }
          else{
            currentScope->isFunc = FALSE;
            //if(DEBUG) fprintf(listing, "First compound statement with current scope: %s (function)\n", currentScope->name);
          }     
          break;
        case IfK:
        case IfElseK:
        case WhileK:
        case ReturnK:
        default:
          break;
      }
      break;
    case ExpK:
      switch (t->kind.exp)
      { case VarArrK:
          l = st_lookup(currentScope, t->attr.name);
          if (l == NULL){
          /* not yet in table, undeclared variable is used */
          // TODO if node found is not an var/array node,
          // call undeclVar
            //undeclVar(t->attr.name, t->lineno);
          }
          else
          { /* already in table, so ignore location, 
             add line number of use only */ 
            st_addLineNo(l, t->lineno);
            if(DEBUG) fprintf(listing, "Symbol %s used at line %d with scope %s\n", t->attr.name, t->lineno, currentScope->name);
          }
          break;
        case CallK:
          l = st_lookup(currentScope, t->attr.name);
          if (l == NULL){
          /* not yet in table, undeclared function is used */
          // TODO if node found is not an function node,
          // call undeclFunc
            //undeclFunc(t->attr.name, t->lineno);
          }
          else
          { /* already in table, so ignore location, 
             add line number of use only */
            st_addLineNo(l, t->lineno);
            if(DEBUG) fprintf(listing, "Symbol %s used at line %d with scope %s\n", t->attr.name, t->lineno, currentScope->name);
          }
          break;
        case ConstK:
        case AssignK:
        case OpK:
        default:
          break;
      }
      break;
    case DeclK:
      switch(t->kind.decl)
      { case VarK:
          l = st_lookup_excluding_parent(currentScope, t->attr.name);
          if (l == NULL){
          /* not yet in table, variable name added to symbol table */
            if(t->type != Void && t->type != VoidArr)
              st_insert(currentScope, t->attr.name, t->lineno, t);
            else{
              voidVar(t->attr.name, t->lineno);
            }
          }
          else
          { /* already in table, so the symbol is redefined. */ 
            redefSymbol(t->attr.name, t->lineno);
          }
          break;
        case FuncK:
          l = st_lookup_excluding_parent(currentScope, t->attr.name);
          if (l == NULL)
          { /* not yet in table, variable name added to symbol table
               and new scope is created. */
            ScopeList s = create_scope(t->attr.name, currentScope);
            s->isFunc = TRUE;
            st_insert(currentScope, t->attr.name, t->lineno, t);
            currentScope = s;
            if(DEBUG) fprintf(listing, "Current scope changed to %s from parent %s\n", currentScope->name, currentScope->parent->name);
          }
          else
          { /* already in table, so the symbol is redefined. */ 
            redefSymbol(t->attr.name, t->lineno);
          }
          break;
        default:
          break;
      }
      break;
    case ParamK:
      switch(t->kind.param)
      { case VoidK:
          break;
        case NonVoidK:
          l = st_lookup_excluding_parent(currentScope, t->attr.name);
          if (l == NULL)
          { /* not yet in table, variable name added to symbol table */
            if(t->type != Void && t->type != VoidArr){
              st_insert(currentScope, t->attr.name, t->lineno, t);
            }
            else{
              voidVar(t->attr.name, lineno);
            }
          }
          else
          { /* already in table, so the symbol is redefined. */ 
            redefSymbol(t->attr.name, t->lineno);
          }
          break;
        default:
          break;  
      }
      break;
    default:
      break;
  }
}

/* Procedure postInsert pops back 
 * current Scope if it is created 
 * during insertion. 
 */
static void postInsert( TreeNode * t)
{ ScopeList s;
  switch (t->nodekind)
  { case StmtK:
      switch (t->kind.stmt)
      { case CompK:
          if(currentScope->isFunc == FALSE){ 
            if(DEBUG) fprintf(listing, "Current scope changed to %s from child %s\n", currentScope->parent->name, currentScope->name);
            currentScope = currentScope->parent;
          }          
          break;
        case IfK:
        case IfElseK:
        case WhileK:
        case ReturnK:
        default:
          break;
      }
      break;
    case ExpK:
      switch (t->kind.exp)
      { case VarArrK:
        case CallK:
        case ConstK:
        case AssignK:
        case OpK:
        default:
          break;
      }
      break;
    case DeclK:
      switch(t->kind.decl)
      { case VarK:
          break;
        case FuncK:
          s = find_scope(t->attr.name);
          s->isFunc = TRUE;
          //if(DEBUG) fprintf(listing, "Function scope name: %s, isFunc: %d\n", s->name, s->isFunc);
          //printScopeTab(listing);
          break;
        default:
          break;
      }
      break;
    case ParamK:
      switch(t->kind.param)
      { case VoidK:
        case NonVoidK:
        default:
          break;  
      }
      break;
    default:
      break;
  }
}

void init_global_scope_with_builtins()
{
  char * savedName = (char *)malloc(sizeof(char) * 40);
  savedName = copyString("{global}");
  ScopeList global = create_scope(savedName, currentScope);
  currentScope = global;

  TreeNode * input = newDeclNode(FuncK);
  TreeNode * output = newDeclNode(FuncK);
  TreeNode * value = newParamNode(NonVoidK);

  savedName = copyString("input");
  input->type = Integer;
  input->attr.name = savedName;
  input->lineno = 0;
  input->child[0] = newParamNode(VoidK);
  input->child[1] = newStmtNode(CompK);
  input->sibling = output;

  savedName = copyString("output");
  output->type = Void;
  output->attr.name = savedName;
  output->lineno = 0;
  output->child[0] = value;
  output->child[1] = newStmtNode(CompK);

  savedName = copyString("value");
  value->type = Integer;
  value->attr.name = savedName;
  value->lineno = 0;

  DEBUG *= TraceAnalyze;
  traverse(input, insertNode, postInsert);
}


/* Function buildSymtab constructs the symbol 
 * table by preorder traversal of the syntax tree
 */
void buildSymtab(TreeNode * syntaxTree)
{ init_global_scope_with_builtins();
  traverse(syntaxTree,insertNode,postInsert);
  if (TraceAnalyze)
  { fprintf(listing,"\n\n< Symbol Table >\n");
    printSymTab(listing);
    fprintf(listing,"\n\n< Functions >\n");
    printFuncSymTab(listing);
    fprintf(listing,"\n\n< Global Symbols >\n");
    printGlobalSymTab(listing);
    fprintf(listing,"\n\n< Scopes >\n");
    printScopeTab(listing);
  }
}

/* Procedure postInsert pops back 
 * current Scope if it is created 
 * during insertion. 
 */
static void preCheck( TreeNode * t)
{ ScopeList s;
  switch (t->nodekind)
  { case StmtK:
      switch (t->kind.stmt)
      { case CompK:
          if(currentScope->isFunc == FALSE){
            s = find_scope(t->attr.name);
            currentScope = s;
            if(DEBUG) fprintf(listing, "Current scope changed to %s from parent %s\n", currentScope->name, currentScope->parent->name);
          }
          else{
            currentScope->isFunc = FALSE;
          }
          break;
        case IfK:
        case IfElseK:
        case WhileK:
        case ReturnK:
        default:
          break;
      }
      break;
    case ExpK:
      switch (t->kind.exp)
      { case VarArrK:
        case CallK:
        case ConstK:
        case AssignK:
        case OpK:
        default:
          break;
      }
      break;
    case DeclK:
      switch(t->kind.decl)
      { case VarK:
          break;
        case FuncK:
          s = find_scope(t->attr.name);
          currentScope = s;
          if(DEBUG) fprintf(listing, "Current scope changed to %s from parent %s\n", currentScope->name, currentScope->parent->name);
          if(currentScope->isFunc != TRUE){
            if(DEBUG) fprintf(listing, "Scope %s is not a function scope\n", currentScope->name);
          }
          break;
        default:
          break;
      }
      break;
    case ParamK:
      switch(t->kind.param)
      { case VoidK:
        case NonVoidK:
        default:
          break;  
      }
      break;
    default:
      break;
  }
}

/* Procedure checkNode performs
 * type checking at a single tree node
 */
static void checkNode(TreeNode * t)
{ BucketList l;
  ScopeList s;
  ExpType type;
  switch (t->nodekind)
  { case StmtK:
      switch (t->kind.stmt)
      { case CompK:
          if(currentScope->isFunc == FALSE){ 
            if(DEBUG) fprintf(listing, "Current scope changed to %s from child %s\n", currentScope->parent->name, currentScope->name);
            currentScope = currentScope->parent;
          }
          break;    
        case IfK:
        case IfElseK:
        case WhileK:
          type = t->child[0]->type;
          if(type != Integer){
            invalidCond(t->lineno);
          }
          break;
        case ReturnK:
          s = currentScope;
          l = NULL;

          if(t->child[0] == NULL)
            type = Void;
          else
            type = t->child[0]->type;

          while(s != NULL){
            l = st_lookup(currentScope, s->name);
            if(l != NULL && l->node->nodekind == DeclK && l->node->kind.decl == FuncK)
              break; // Found bucketList containing function declaration
            else
              s = s->parent; // Find with parent scope
          }
          if(s != NULL && l != NULL){ // Function declaration is found
            if(l->node->type == type) // Return type == exp type
              t->type = type;
            else{
              invalidRet(t->lineno);
              if(DEBUG) fprintf(listing, "Expression type mismatch with return type of function %s\n", l->node->attr.name);
            }
          }
          else{ // Function declaration is not found
            invalidRet(t->lineno);
            if(DEBUG) fprintf(listing, "Function declaration for return statement at line %d is not found\n", t->lineno);
          }
          break;
        default:
          break;
      }
      break;
    case ExpK:
      switch (t->kind.exp)
      { case VarArrK:
          l = NULL;
          l = st_lookup(currentScope, t->attr.name);
          if(l == NULL || (l->node->nodekind == DeclK && l->node->kind.decl == FuncK)){ // Symbol table lookup failed.
            if(t->child[0] == NULL) undeclVar(t->attr.name, t->lineno); // t is containing array expression
            else nonArrIdx(t->attr.name, t->lineno);
            t->type = Invalid;
            if(DEBUG) fprintf(listing, "Symbol %s lookup failed with scope %s\n", t->attr.name, currentScope->name);
          }
          else{ // Found in symbol table.
            if(t->child[0] == NULL){ // Accessing variable
              if(l->node->type == Integer){
                t->type = Integer;
              }
              else if(l->node->type == IntegerArr){
                t->type = IntegerArr;
              }
              else{ // Void variable. Never reached.
                voidVar(t->attr.name, t->lineno);
                t->type = Invalid;
              }
            }
            else{ // Accesing integer array
              if(l->node->type != IntegerArr){ // Symbol found is not an int array
                nonArrIdx(t->attr.name, t->lineno);
                t->type = Invalid;
              }
              else{ // Symbol found is an int array
                if(t->child[0]->type == Integer){ // Index is an integer
                  t->type = Integer;
                }
                else{ // Index is not an integer
                  nonIntIdx(t->attr.name, t->lineno);
                  t->type = Integer;
                }
              }
            }
          }
          break;
        case CallK:
          l = NULL;
          l = st_lookup(currentScope, t->attr.name);
          if(l == NULL || (l->node->nodekind != DeclK) /* Symbol table lookup failed */
            || (l->node->nodekind == DeclK && l->node->kind.decl != FuncK)){
              undeclFunc(t->attr.name, t->lineno);
              invalidFuncCall(t->attr.name, t->lineno);
              if(DEBUG) fprintf(listing, "Symbol %s lookup failed with scope %s\n", t->attr.name, currentScope->name);
          }
          else{ /* Symbol table lookup successful */
            if(l->node->child[0]->kind.param == VoidK){ // Void Parameter
              if(t->child[0] == NULL){ // Function call without arguments
                t->type = l->node->type;
              }
              else{
                t->type = l->node->type;
                invalidFuncCall(t->attr.name, t->lineno);
                if(DEBUG) fprintf(listing, "Function %s(void) called with arguments\n", t->attr.name);
              }
            }
            else{ // Non-Void Parameter
              TreeNode * param, * arg;
              int matched = TRUE;
              param = l->node->child[0];
              arg = t->child[0];

              while(param != NULL && arg != NULL){
                if(param->type != arg->type){ // Type mismatch
                  if(DEBUG) fprintf(listing, "Function %s called with wrong type arguments\n", t->attr.name);
                  matched = FALSE;
                  break;
                }
                param = param->sibling;
                arg = arg->sibling;
              }
              // Wrong numbers of arguments
              if(matched == TRUE && (param != NULL || arg != NULL)){ 
                if(DEBUG) fprintf(listing, "Function %s called with wrong number arguments\n", t->attr.name);
                matched = FALSE;
              }

              if(matched == TRUE){
                t->type = l->node->type;
              }
              else{
                t->type = l->node->type;
                invalidFuncCall(t->attr.name, t->lineno);
              }
            }
          }
          break;
        case ConstK:
          t->type = Integer;
          break;
        case AssignK:
          l = st_lookup(currentScope, t->child[0]->attr.name);
          if(l != NULL){
            if(t->child[0]->type == Invalid || t->child[1]->type == Invalid){
              invalidAssign(t->lineno);
            }
            else if(t->child[0]->type != t->child[1]->type){
              invalidAssign(t->lineno);
            }
            else{
              t->type = t->child[0]->type;
            }
          }
          else{
            invalidAssign(t->lineno);
          }
          break;
        case OpK:
          if ((t->child[0]->type != Integer) || (t->child[1]->type != Integer)){
            invalidOp(t->lineno);
            t->type = Invalid;
          }
          else t->type = Integer;
        default:
          break;
      }
      break;
    case DeclK:
      switch(t->kind.decl)
      { case VarK:
          break;
        case FuncK:
          s = find_scope(t->attr.name);
          s->isFunc = TRUE;
          if(DEBUG) fprintf(listing, "Function scope name: %s, isFunc: %d\n", s->name, s->isFunc);
          //printScopeTab(listing);
          break;
        default:
          break;
      }
      break;
    case ParamK:
      switch(t->kind.param)
      { case VoidK:
          break;
        case NonVoidK:
          break;
        default:
          break;  
      }
      break;
    default:
      break;
  }
}

/* Procedure typeCheck performs type checking 
 * by a postorder syntax tree traversal
 */
void typeCheck(TreeNode * syntaxTree)
{ traverse(syntaxTree,preCheck,checkNode);
}
