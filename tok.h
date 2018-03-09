enum token_type {
	TOKEN_EOF,
	TOKEN_LPAREN,
	TOKEN_RPAREN,
	TOKEN_LBRACE,
	TOKEN_RBRACE,
	TOKEN_LBRACK,
	TOKEN_RBRACK,
	TOKEN_LANGLE,
	TOKEN_RANGLE,
	TOKEN_ASTERISK,
	TOKEN_PLUS,
	TOKEN_MINUS,
	TOKEN_TILDE,
	TOKEN_FWDSLASH,
	TOKEN_BACKSLASH,
	TOKEN_PERCENT,
	TOKEN_HAT,
	TOKEN_PIPE,
	TOKEN_AMPERSAND,
	TOKEN_EXCLAIM,
	TOKEN_SEMI,
	TOKEN_COLON,
	TOKEN_COMMA,
	TOKEN_DOT,
	TOKEN_EQUAL,
	TOKEN_QUESTION,
	TOKEN_ARROW,
	TOKEN_INCR,
	TOKEN_DECR,
	TOKEN_LSHIFT,
	TOKEN_RSHIFT,
	TOKEN_LEQUAL,
	TOKEN_REQUAL,
	TOKEN_EQUALS,
	TOKEN_NEQUAL,
	TOKEN_AND,
	TOKEN_OR,
	TOKEN_ELLIPSIS,
	TOKEN_MULEQUAL,
	TOKEN_DIVEQUAL,
	TOKEN_MODEQUAL,
	TOKEN_ADDEQUAL,
	TOKEN_SUBEQUAL,
	TOKEN_LSHEQUAL,
	TOKEN_RSHEQUAL,
	TOKEN_BANDEQUAL,
	TOKEN_BXOREQUAL,
	TOKEN_BOREQUAL,
	TOKEN_NEWLINE,
	TOKEN_BREAK,
	TOKEN_CASE,
	TOKEN_CONTINUE,
	TOKEN_DEFAULT,
	TOKEN_CHAR,
	TOKEN_DO,
	TOKEN_ELSE,
	TOKEN_ENUM,
	TOKEN_EXTERN,
	TOKEN_FLOAT,
	TOKEN_FOR,
	TOKEN_GOTO,
	TOKEN_IF,
	TOKEN_INT,
	TOKEN_LONG,
	TOKEN_OPEN,
	TOKEN_CLOSED,
	TOKEN_RETURN,
	TOKEN_SHORT,
	TOKEN_SIGNED,
	TOKEN_SIZEOF,
	TOKEN_STATIC,
	TOKEN_STRUCT,
	TOKEN_SWITCH,
	TOKEN_UNION,
	TOKEN_UNSIGNED,
	TOKEN_VOID,
	TOKEN_VOLATILE,
	TOKEN_WHILE,
	TOKEN_WS,
	TOKEN_IDENT,
	TOKEN_INTEGER,
	TOKEN_STRING,
	TOKEN_CHARACTER,
	NUM_TOKENS
};
struct token {
	int   type;
	char *string;
	const char *filename;
	int   line;
	int   col;
};
extern const char *token_name(int type);
extern char *token_stringify(struct token *);
struct tok_defn {
	int type;
	struct nfa_graph *pattern;
};
struct tok_tokenlist {
	struct token **tokens;
	ptrdiff_t num_tokens;
	ptrdiff_t capacity;
};
extern struct tok_tokenlist *tok_tokenlist_new(void);
extern void tok_tokenlist_push(struct tok_tokenlist *, struct token *);
extern void tok_tokenlist_pushclosure(struct tok_tokenlist *, struct token *);
extern void tok_tokenlist_pushmatching(struct tok_tokenlist *, struct token *, char c);
extern int tok_tokenlist_contains(struct tok_tokenlist *, struct token *);
extern void tok_tokenlist_expand(struct tok_tokenlist *);
extern void tok_tokenlist_clear(struct tok_tokenlist *);
#define trace_tokenlist(code, list)\
	do {\
		struct tok_tokenlist *_l = (list);\
		int _i;\
		tracefx(" - " code "ns:%lu last:", _l->num_tokens);\
		for (_i = 0; _i < _l->num_tokens; _i++) {\
			fprintf(stderr, "{%s} ", token_stringify(_l->tokens[_i]));\
		}\
		fprintf(stderr, "\n");\
	} while (0)
