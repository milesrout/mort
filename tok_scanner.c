#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "tok.h"
#include "nfa.h"
#include "tok_scanner.h"

static const char *alpha      = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_";
static const char *alnum      = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";
static const char *nonzero    = "123456789";
static const char *digit      = "0123456789";
static const char *hexdigit   = "0123456789abcdefABCDEF";
static const char *octaldigit = "01234567";

static struct nfa_graph *octal_digit(void)
{
	return nfa_symbol(octaldigit);
}

static struct nfa_graph *hex_quad(void)
{
	return nfa_concat(
	    nfa_concat(
	        nfa_symbol(hexdigit),
	        nfa_symbol(hexdigit)),
	    nfa_concat(
	        nfa_symbol(hexdigit),
	        nfa_symbol(hexdigit)));
}

static char *esc(char c);
#define DEFINE(tok, expr) tokens[tok] = (struct tok_defn){tok, expr}
void init_tokens(struct tok_defn **_tokens)
{
	struct tok_defn *tokens = emalloc(NUM_TOKENS * (sizeof *tokens));

	struct nfa_graph *simple_escape_sequence = nfa_symbol("\'\"?\\abfnrtv");
	struct nfa_graph *universal_character_name =
	    nfa_union(
	        nfa_concat(nfa_symbol("u"), hex_quad()),
	        nfa_concat(nfa_symbol("u"), nfa_concat(hex_quad(), hex_quad())));
	struct nfa_graph *octal_escape_sequence =
	    nfa_union(
	        octal_digit(),
	        nfa_union(
	            nfa_concat(octal_digit(), octal_digit()),
	            nfa_concat(octal_digit(), nfa_concat(octal_digit(), octal_digit()))
	        ));
	struct nfa_graph *hexadecimal_escape_sequence =
	    nfa_concat(
	        nfa_symbol("x"),
	        nfa_concat(nfa_symbol(hexdigit), nfa_kleene_star(nfa_symbol(hexdigit))));

	struct nfa_graph *escape_sequence = nfa_concat(
	    nfa_symbol("\\"),
	    nfa_union(
	        nfa_union(universal_character_name, simple_escape_sequence),
	        nfa_union(octal_escape_sequence, hexadecimal_escape_sequence)
	    ));

	// synthetic tokens
	DEFINE(TOKEN_EOF,       nfa_never());

	// brackets
	DEFINE(TOKEN_LPAREN,    nfa_symbol("("));
	DEFINE(TOKEN_RPAREN,    nfa_symbol(")"));
	DEFINE(TOKEN_LBRACE,    nfa_symbol("{"));
	DEFINE(TOKEN_RBRACE,    nfa_symbol("}"));
	DEFINE(TOKEN_LBRACK,    nfa_symbol("["));
	DEFINE(TOKEN_RBRACK,    nfa_symbol("]"));
	DEFINE(TOKEN_LANGLE,    nfa_symbol("<"));
	DEFINE(TOKEN_RANGLE,    nfa_symbol(">"));

	// single-character punctuators
	DEFINE(TOKEN_ASTERISK,  nfa_symbol("*"));
	DEFINE(TOKEN_PLUS,      nfa_symbol("+"));
	DEFINE(TOKEN_MINUS,     nfa_symbol("-"));
	DEFINE(TOKEN_TILDE,     nfa_symbol("~"));
	DEFINE(TOKEN_FWDSLASH,  nfa_symbol("/"));
	DEFINE(TOKEN_BACKSLASH, nfa_symbol("\\"));
	DEFINE(TOKEN_PERCENT,   nfa_symbol("%"));
	DEFINE(TOKEN_HAT,       nfa_symbol("^"));
	DEFINE(TOKEN_PIPE,      nfa_symbol("|"));
	DEFINE(TOKEN_AMPERSAND, nfa_symbol("&"));
	DEFINE(TOKEN_EXCLAIM,   nfa_symbol("!"));
	DEFINE(TOKEN_SEMI,      nfa_symbol(";"));
	DEFINE(TOKEN_COLON,     nfa_symbol(":"));
	DEFINE(TOKEN_COMMA,     nfa_symbol(","));
	DEFINE(TOKEN_DOT,       nfa_symbol("."));
	DEFINE(TOKEN_EQUAL,     nfa_symbol("="));
	DEFINE(TOKEN_QUESTION,  nfa_symbol("?"));

	// multi-character punctuators
	DEFINE(TOKEN_ARROW,     nfa_string("->"));
	DEFINE(TOKEN_INCR,      nfa_string("++"));
	DEFINE(TOKEN_DECR,      nfa_string("--"));
	DEFINE(TOKEN_LSHIFT,    nfa_string("<<"));
	DEFINE(TOKEN_RSHIFT,    nfa_string(">>"));
	DEFINE(TOKEN_LEQUAL,    nfa_string("<="));
	DEFINE(TOKEN_REQUAL,    nfa_string(">="));
	DEFINE(TOKEN_EQUALS,    nfa_string("=="));
	DEFINE(TOKEN_NEQUAL,    nfa_string("!="));
	DEFINE(TOKEN_AND,       nfa_string("&&"));
	DEFINE(TOKEN_OR,        nfa_string("||"));
	DEFINE(TOKEN_MULEQUAL,  nfa_string("*="));
	DEFINE(TOKEN_DIVEQUAL,  nfa_string("/="));
	DEFINE(TOKEN_MODEQUAL,  nfa_string("%="));
	DEFINE(TOKEN_ADDEQUAL,  nfa_string("+="));
	DEFINE(TOKEN_SUBEQUAL,  nfa_string("-="));
	DEFINE(TOKEN_LSHEQUAL,  nfa_string("<<="));
	DEFINE(TOKEN_RSHEQUAL,  nfa_string(">>="));
	DEFINE(TOKEN_BANDEQUAL, nfa_string("&="));
	DEFINE(TOKEN_BXOREQUAL, nfa_string("^="));
	DEFINE(TOKEN_BOREQUAL,  nfa_string("|="));
	DEFINE(TOKEN_ELLIPSIS,  nfa_string("..."));

	// keywords
	DEFINE(TOKEN_BREAK,     nfa_string("break")),
	DEFINE(TOKEN_CASE,      nfa_string("case")),
	DEFINE(TOKEN_CONTINUE,  nfa_string("continue")),
	DEFINE(TOKEN_DEFAULT,   nfa_string("default")),
	DEFINE(TOKEN_CHAR,      nfa_string("char")),
	DEFINE(TOKEN_DO,        nfa_string("do")),
	DEFINE(TOKEN_ELSE,      nfa_string("else")),
	DEFINE(TOKEN_ENUM,      nfa_string("enum")),
	DEFINE(TOKEN_EXTERN,    nfa_string("extern")),
	DEFINE(TOKEN_FLOAT,     nfa_string("float")),
	DEFINE(TOKEN_FOR,       nfa_string("for")),
	DEFINE(TOKEN_GOTO,      nfa_string("goto")),
	DEFINE(TOKEN_IF,        nfa_string("if")),
	DEFINE(TOKEN_INT,       nfa_string("int")),
	DEFINE(TOKEN_LONG,      nfa_string("long")),
	DEFINE(TOKEN_OPEN,      nfa_string("open")),
	DEFINE(TOKEN_CLOSED,    nfa_string("closed")),
	DEFINE(TOKEN_RETURN,    nfa_string("return")),
	DEFINE(TOKEN_SHORT,     nfa_string("short")),
	DEFINE(TOKEN_SIGNED,    nfa_string("signed")),
	DEFINE(TOKEN_SIZEOF,    nfa_string("sizeof")),
	DEFINE(TOKEN_STATIC,    nfa_string("static")),
	DEFINE(TOKEN_STRUCT,    nfa_string("struct")),
	DEFINE(TOKEN_SWITCH,    nfa_string("switch")),
	DEFINE(TOKEN_UNION,     nfa_string("union")),
	DEFINE(TOKEN_UNSIGNED,  nfa_string("unsigned")),
	DEFINE(TOKEN_VOID,      nfa_string("void")),
	DEFINE(TOKEN_VOLATILE,  nfa_string("volatile")),
	DEFINE(TOKEN_WHILE,     nfa_string("while")),

	// blank tokens
	DEFINE(TOKEN_NEWLINE,   nfa_symbol("\n"));
	DEFINE(TOKEN_WS,        nfa_union(nfa_symbol(" "), nfa_symbol("\t")));

	// variable-content tokens
	DEFINE(TOKEN_IDENT,     nfa_concat(nfa_symbol(alpha), nfa_kleene_star(nfa_symbol(alnum))));
	DEFINE(TOKEN_INTEGER,   nfa_concat(nfa_symbol(nonzero), nfa_kleene_star(nfa_symbol(digit))));
	DEFINE(TOKEN_STRING,    nfa_concat(
	                          nfa_symbol("\""), 
	                          nfa_concat(
	                            nfa_kleene_star(
	                              nfa_union(nfa_anybut("\"\\\n"), escape_sequence)
	                            ),
	                            nfa_symbol("\"")
	                          )
	                        ));
	DEFINE(TOKEN_CHARACTER, nfa_concat(nfa_symbol("\'"), nfa_concat(nfa_symbol(alnum), nfa_symbol("\'"))));

	*_tokens = tokens;
}
#undef DEFINE

off_t simulate(struct nfa_graph *graph, FILE *stream)
{
	int c = '\0';
	ptrdiff_t i = 0;
	int count = 0;
	int has_matched = 0;
	int matched_count = -1;
	struct nfa_statelist *current = nfa_statelist_new();
	struct nfa_statelist *next = nfa_statelist_new();
	struct nfa_statelist *tmp = NULL;

	nfa_statelist_pushclosure(current, graph->initial_state);

	while ((c = fgetc(stream)) != EOF) {
		count += 1;

		// tracef(" - c:'%s'", esc(c));
		// trace_statelist("c", current);
		for (i = 0; i < (ptrdiff_t)current->num_states; i++) {
			nfa_statelist_pushmatching(next, current->states[i], (char)c);
		}
		// trace_statelist("n", next);

		if (next->num_states == 0) {
			break;
		}

		if (nfa_statelist_contains(next, graph->final_state)) {
			has_matched = 1;
			matched_count = count;
		}

		nfa_statelist_clear(current);
		tmp = current;
		current = next;
		next = tmp;
	}

	if (has_matched) {
		fseek(stream, matched_count - count, SEEK_CUR);
		return matched_count;
	}
	return -count;
}

static char *twochar(char c, char d)
{
	char *s = emalloc(3);
	s[0] = c;
	s[1] = d;
	s[2] = '\0';
	return s;
}

static char *threechar(char c, char d, char e)
{
	char *s = emalloc(3);
	s[0] = c;
	s[1] = d;
	s[2] = e;
	s[3] = '\0';
	return s;
}

static char *escapes(const char *string)
{
	ptrdiff_t i;
	char *p;
	char *new;

	if (string == NULL) {
		return twochar('\xCE', '\xb5');
	}

	p = new = emalloc(2 * strlen(string) + 1);

	for (i = 0; string[i] != '\0'; i++) {
		if (string[i] == '\n') {
			*p++ = '\\';
			*p++ = 'n';
		} else if (string[i] == '\t') {
			*p++ = '\\';
			*p++ = 't';
		} else {
			if (strchr("\"\\", string[i]) != NULL) {
				*p++ = '\\';
			}
			*p++ = string[i];
		}
	}
	*p++ = '\0';

	return erealloc(new, strlen(new) + 1);
}
static char *onechar(char c)
{
	char *s = emalloc(2);
	s[0] = c;
	s[1] = '\0';
	return s;
}
static char *esc(char c)
{
	if (c == '\n') {
		return twochar('\\', 'n');
	}
	if (c == '\t') {
		return twochar('\\', 't');
	}
	if (c == ' ') {
		return threechar('\xE2', '\x90', '\xA3');
	}
	return onechar(c);
}
struct token *get_token(struct tok_scanner *s)
{
	struct token *t;
	while (!feof(s->f) && !ferror(s->f)) {
		int i, n;
		long int off;
		char c = fgetc(s->f);
		if (c == EOF) {
			break;
		}
		ungetc(c, s->f);
		for (i = 0; i < NUM_TOKENS; i++) {
			// fprintf(stderr, "Trying %s.\n", token_name(s->tokens[i].type));
			n = simulate(s->tokens[i].pattern, s->f);
			if (n > 0) {
				long int m = ftell(s->f);
				char *str = emalloc(n + 1);
				struct token *t = emalloc(sizeof *t);

				fseek(s->f, -n, SEEK_CUR);
				fgets(str, n + 1, s->f);
				*t = (struct token){.line = 0, .col = m, .filename = s->filename, .type = s->tokens[i].type, .string = str};
				// fprintf(stderr, "Matched \"%s\" at [%ld:%ld) to token %s.\n", escapes(str), m - n, m, token_name(s->tokens[i].type));
				return t;
			}
			fseek(s->f, n, SEEK_CUR);
		}

		off = ftell(s->f);
		c = fgetc(s->f);
		if (c != EOF) {
			fprintf(stderr, "Cannot match '%c' at %ld to any token.\n", c, off);
			return NULL;
		}
	}
	t = emalloc(sizeof *t);
	*t = (struct token){.line = 0, .col = -1, .filename = s->filename, .type = TOKEN_EOF, .string = ""};
	return t;
}
