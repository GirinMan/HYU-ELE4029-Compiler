/****************************************************/
/* File: symtab.h                                   */
/* Symbol table interface for the C-MINUS compiler     */
/* (allows only one symbol table)                   */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"

#ifndef _SYMTAB_H_
#define _SYMTAB_H_

/* SCOPE_SIZE is the size of the hash table */
#define SCOPE_SIZE 50

/* BUCKET_SIZE is the size of the bucket list */
#define BUCKET_SIZE 211

/* SHIFT is the power of two used as multiplier
   in hash function  */
#define SHIFT 4

/* the list of line numbers of the source 
 * code in which a variable is referenced
 */
typedef struct LineListRec
   { int lineno;
     struct LineListRec * next;
   } * LineList;

/* The record in the bucket lists for
 * each variable, including name, 
 * assigned memory location, and
 * the list of line numbers in which
 * it appears in the source code
 */
typedef struct BucketListRec
   { char * name;
     TreeNode * node; /* memory location for variable */
     LineList lines;
     int memloc ; /* memory location for variable */
     struct BucketListRec * next;
   } * BucketList;

/*  The record for each scope,
    including name, it bucket,
    and parent scope.
*/
typedef struct ScopeListRec
   { char * name;
     int curloc;
     int compidx;
     int isFunc;
     BucketList bucket[BUCKET_SIZE];
     struct ScopeListRec * parent;    
   } * ScopeList;

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert( ScopeList s, char * name, int lineno, TreeNode * t );

void st_addLineNo( BucketList l, int lineno);

ScopeList create_scope(char * name, ScopeList currentScope);

ScopeList find_scope(char * name);

/* Function st_lookup returns the memory 
 * location of a variable or NULL if not found
 */
BucketList st_lookup ( ScopeList s, char * name );

/* Function st_lookup_excluding_parent returns the memory 
 * location of a variable only in the specific scope
 * or NULL if not found
 */
BucketList st_lookup_excluding_parent ( ScopeList s, char * name );

/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */
void printSymTab(FILE * listing);
void printFuncSymTab(FILE * listing);
void printGlobalSymTab(FILE * listing);
void printScopeTab(FILE * listing);

#endif
