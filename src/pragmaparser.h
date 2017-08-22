#define PDES 257
#define EXCLUDE 258
#define MAP 259
#define FORWARD 260
#define REVERSE 261
#define COMMIT 262
#define ORIGINAL 263
#define EMPTY 264
#define IDENT 265
#define INTEGER 266
#ifdef YYSTYPE
#undef  YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
#endif
#ifndef YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
typedef union {
char* idstring; /* IDENT */
long intval; /* INTEGER */
} YYSTYPE;
#endif /* !YYSTYPE_IS_DECLARED */
extern YYSTYPE pragmaparserlval;
