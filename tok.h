enum token_type {
	TOKEN_EOF,
	TOKEN_LPAREN,
	TOKEN_RPAREN,
	TOKEN_LBRACE,
	TOKEN_RBRACE,
	TOKEN_LBRACK,
	TOKEN_RBRACK,
	TOKEN_NEWLINE,
	TOKEN_IF,
	TOKEN_ELSE,
	TOKEN_RETURN,
	TOKEN_IDENT,
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
extern char *token_stringify(struct token *t);
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
