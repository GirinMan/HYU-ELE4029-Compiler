/********************************************************/
/* File: symtab.c                                       */
/* Symbol table implementation for the C-MINUS compiler */
/* (allows only one symbol table)                       */
/* Symbol table is implemented as a chained             */
/* hash table                                           */
/* Compiler Construction: Principles and Practice       */
/* Kenneth C. Louden                                    */
/********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

/* the hash function */
static int hash ( char * key, int size )
{ int temp = 0;
  int i = 0;
  while (key[i] != '\0')
  { temp = ((temp << SHIFT) + key[i]) % size;
    ++i;
  }
  return temp;
}

/* scope lists */
static ScopeList scopeList[SCOPE_SIZE] = {NULL};

ScopeList create_scope(char * name, ScopeList currentScope)
{ int i;
  ScopeList s = NULL;
  s = (ScopeList) malloc(sizeof(struct ScopeListRec));
  s->name = name;
  s->isFunc = FALSE;
  s->curloc = 0;
  s->compidx = 0;
  s->parent = currentScope;
  for(i = 0; i < BUCKET_SIZE; i++) s->bucket[i] = NULL;
  for (i = 0; i < SCOPE_SIZE; i++)
    if (scopeList[i] == NULL)
    { scopeList[i] = s;
      break;
    }
  return s;
}

ScopeList find_scope(char * name){
  int i;
  ScopeList s = NULL;
  for(i = 0; i < SCOPE_SIZE; i++){
    s = scopeList[i];
    if(s != NULL && strcmp(s->name, name) == 0){
      break;
    }
  }
  return s;
}

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert( ScopeList s, char * name, int lineno, TreeNode * tree)
{ 
  int h_bucket = hash(name, BUCKET_SIZE);
  BucketList l = s->bucket[h_bucket];
  while ((l != NULL) && (strcmp(name,l->name) != 0))
    l = l->next;
  if (l == NULL) /* variable not yet in table */
  { l = (BucketList) malloc(sizeof(struct BucketListRec));
    l->name = name;
    l->node = tree;
    l->memloc = s->curloc++;

    l->lines = (LineList) malloc(sizeof(struct LineListRec));
    l->lines->lineno = lineno;
    l->lines->next = NULL;

    l->next = s->bucket[h_bucket];
    s->bucket[h_bucket] = l; }
} /* st_insert */

/* Procedure st_addLineNo inserts line numbers
 * and into the symbol table
 */
void st_addLineNo( BucketList l, int lineno){
  
  if(l == NULL){
    fprintf(listing, "Symbol %s at line %d is not found in scope\n", l->name, lineno);
    return;
  }

  LineList t = l->lines;
  while (t->next != NULL) t = t->next;
  t->next = (LineList) malloc(sizeof(struct LineListRec));
  t->next->lineno = lineno;
  t->next->next = NULL;
} /* st_addLineNo */

/* Function st_lookup returns the BucketList
 * containing the variable or NULL if not found
 */
BucketList st_lookup ( ScopeList s, char * name )
{ int h_bucket = hash(name, BUCKET_SIZE);
  do
  { BucketList l = s->bucket[h_bucket];
    while ((l != NULL) && (strcmp(name,l->name) != 0))
      l = l->next;
    if (l == NULL) s = s->parent;
    else return l;
  } while (s != NULL);
  return NULL;
}

/* Function st_lookup_excluding_parent returns the BucketList
 * containing the variable only in the specific scope
 * or NULL if not found
 */
BucketList st_lookup_excluding_parent ( ScopeList s, char * name )
{ int h_bucket = hash(name, BUCKET_SIZE);
  BucketList l = s->bucket[h_bucket];
  while ((l != NULL) && (strcmp(name,l->name) != 0))
    l = l->next;
  if (l == NULL) return NULL;
  else return l;
}

void printSymType(ExpType type){
  switch(type){
    case Void:
      fprintf(listing, "%-14s ", "void");
      break;
    case VoidArr:
      fprintf(listing, "%-14s ", "void[]");
      break;
    case Integer:
      fprintf(listing, "%-14s ", "int");
      break;
    case IntegerArr:
      fprintf(listing, "%-14s ", "int[]");
      break;
    case Invalid:
    default:
      fprintf(listing, "%-14s ", "Error");
  }
}


/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */
void printSymTab(FILE * listing)
{ int i, j;
  fprintf(listing,"Symbol Name    Symbol Kind   Symbol Type    Scope Name   Location  Line Numbers\n");
  fprintf(listing,"-------------  -----------  -------------  ------------  --------  ------------\n");
  for (i=0; i<SCOPE_SIZE;++i)
  { if (scopeList[i] != NULL)
    { for (j=0;j<BUCKET_SIZE;++j)   
      { BucketList l = scopeList[i]->bucket[j];
        while (l != NULL)
        { LineList t = l->lines;
          fprintf(listing,"%-14s ",l->name);

          if(l->node->nodekind == DeclK){
            if(l->node->kind.decl == FuncK)
              fprintf(listing,"%-12s ", "Function");
            else if(l->node->kind.decl == VarK)
              fprintf(listing,"%-12s ", "Variable");
          }
          else if(l->node->nodekind == ParamK){
            if(l->node->kind.param == NonVoidK)
              fprintf(listing,"%-12s ", "Variable");
          }
          else
            fprintf(listing,"%-12s ", "Unknown");

          printSymType(l->node->type);

          fprintf(listing,"%-13s ",scopeList[i]->name);

          fprintf(listing,"%-8d  ",l->memloc);

          while (t != NULL)
          { fprintf(listing,"%4d ",t->lineno);
            t = t->next;
          }
          fprintf(listing,"\n");
          l = l->next;
        }
      }
    }
  }
} /* printSymTab */

/* Procedure printFuncSymTab prints a formatted 
 * listing of the function symbol table contents 
 * to the listing file
 */
void printFuncSymTab(FILE * listing)
{ int i, j;
  fprintf(listing,"Function Name   Return Type   Parameter Name  Parameter Type\n");
  fprintf(listing,"-------------  -------------  --------------  --------------\n");
  for (i=0; i<SCOPE_SIZE;++i)
  { if (scopeList[i] != NULL)
    { for (j=0;j<BUCKET_SIZE;++j)   
      { BucketList l = scopeList[i]->bucket[j];
        while (l != NULL)
        { if(l->node->nodekind == DeclK && l->node->kind.decl == FuncK)
          { fprintf(listing,"%-14s ",l->name);

            printSymType(l->node->type);

            fprintf(listing,"%-16s", " ");

            if(l->node->child[0]->kind.param == VoidK)
            { fprintf(listing, "%-14s ", "void");
              fprintf(listing, "\n");
            }
            else
            { fprintf(listing,"\n");
              TreeNode * param = l->node->child[0];
              while(param != NULL){
                fprintf(listing,"%-14s ", "-");
                fprintf(listing,"%-14s ", "-");
                fprintf(listing,"%-15s ", param->attr.name);
                printSymType(param->type);
                fprintf(listing, "\n");
                param = param->sibling;
              }
            }
          }
          l = l->next;
        }
      }
    }
  }
} /* printFuncSymTab */

/* Procedure printGlobalSymTab prints a formatted 
 * listing of the  global scopes' symbol table contents 
 * to the listing file
 */
void printGlobalSymTab(FILE * listing)
{ int i;
  ScopeList global = scopeList[0];

  fprintf(listing,"Symbol Name    Symbol Kind   Symbol Type\n");
  fprintf(listing,"-------------  -----------  -------------\n");
  for (i=0; i<BUCKET_SIZE; ++i)   
  { BucketList l = global->bucket[i];
    while (l != NULL)
    { fprintf(listing,"%-14s ",l->name);

      if(l->node->nodekind == DeclK){
        if(l->node->kind.decl == FuncK)
          fprintf(listing,"%-12s ", "Function");
        else if(l->node->kind.decl == VarK)
          fprintf(listing,"%-12s ", "Variable");
      }
      else
        fprintf(listing,"%-12s ", "Unknown");

      printSymType(l->node->type);

      fprintf(listing,"\n");
      l = l->next;
    }
  }
} /* printGlobalSymTab */

/* Procedure printScopeTab prints a formatted 
 * listing of the scopes with symbol table contents 
 * to the listing file
 */
void printScopeTab(FILE * listing)
{ int i, j;
  fprintf(listing,"Scope Name    Nested Level   Symbol Name    Symbol Type    Scope Kind \n");
  fprintf(listing,"------------  ------------  -------------  -------------  ------------\n");
  for (i=1; i<SCOPE_SIZE;++i)
  { if (scopeList[i] != NULL)
    { int empty = TRUE;
      for (j=0; j<BUCKET_SIZE; ++j)   
      { BucketList l = scopeList[i]->bucket[j];
        while (l != NULL)
        { fprintf(listing,"%-13s ",scopeList[i]->name);
          ScopeList s = scopeList[i];
          int level = 0;
          while(s->parent != NULL)
          { s = s->parent;
            level++;
          }
          fprintf(listing, "%-13d ", level);
          fprintf(listing,"%-14s ", l->name);
          printSymType(l->node->type);

          if(scopeList[i]->isFunc == TRUE)
            fprintf(listing, "%-13s ", "Function");
          else
            fprintf(listing, "%-13s ", "Non-Function");
          fprintf(listing,"\n");
          l = l->next;
          empty = FALSE;
        }
      }
      if(empty != TRUE) fprintf(listing, "\n");
    }
  }
} /* printScopeTab */