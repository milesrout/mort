#define _GNU_SOURCE
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "util.h"

#include "tok.h"
#include "nfa.h"

// The scanner, parser, etc. have a 'pull' structure. Rather than reading the
// entire input file into memory, turning it into tokens, then parsing the rest
// of the file, the file is tokenised lazily as the tokens are required by the
// parser.

const char *alpha = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_";
const char *alnum = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";

void init_tokens(struct tok_defn **_tokens)
{
	struct tok_defn *tokens = emalloc(NUM_TOKENS * (sizeof *tokens));
	tokens[TOKEN_EOF]     = (struct tok_defn){TOKEN_EOF,     nfa_no()};
	tokens[TOKEN_LPAREN]  = (struct tok_defn){TOKEN_LPAREN,  nfa_symbol("(")};
	tokens[TOKEN_RPAREN]  = (struct tok_defn){TOKEN_RPAREN,  nfa_symbol(")")};
	tokens[TOKEN_LBRACE]  = (struct tok_defn){TOKEN_LBRACE,  nfa_symbol("{")};
	tokens[TOKEN_RBRACE]  = (struct tok_defn){TOKEN_RBRACE,  nfa_symbol("}")};
	tokens[TOKEN_LBRACK]  = (struct tok_defn){TOKEN_LBRACK,  nfa_symbol("[")};
	tokens[TOKEN_RBRACK]  = (struct tok_defn){TOKEN_RBRACK,  nfa_symbol("]")};
	tokens[TOKEN_NEWLINE] = (struct tok_defn){TOKEN_NEWLINE, nfa_symbol("\n")};
	tokens[TOKEN_IF]      = (struct tok_defn){TOKEN_IF,      nfa_string("if")};
	tokens[TOKEN_ELSE]    = (struct tok_defn){TOKEN_ELSE,    nfa_string("else")};
	tokens[TOKEN_RETURN]  = (struct tok_defn){TOKEN_RETURN,  nfa_string("return")};
	tokens[TOKEN_IDENT]   = (struct tok_defn){TOKEN_IDENT,   nfa_concatenation(nfa_symbol(alpha), nfa_kleene_star(nfa_symbol(alnum)))};

	*_tokens = tokens;
}

static char *onechar(char c)
{
	char *s = emalloc(2);
	s[0] = c;
	s[1] = '\0';
	return s;
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

static char *escape(char c)
{
	if (c == '\n') {
		return twochar('\\', 'n');
	}
	if (c == ' ') {
		return threechar('\xE2', '\x90', '\xA3');
	}
	return onechar(c);
}

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

		// tracef(" - c:'%s'", escape(c));
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

struct mort_parser {
	FILE *f;
	struct tok_defn *tokens;
	const char *filename;
};

struct token *get_next_token(struct mort_parser *p)
{
	struct token *t;
	while (!feof(p->f) && !ferror(p->f)) {
		int i, n;
		long int off;
		char c = fgetc(p->f);
		if (c == EOF) {
			break;
		}
		ungetc(c, p->f);
		for (i = 0; i < NUM_TOKENS; i++) {
			//fprintf(stderr, "Trying %s.\n", token_name(p->tokens[i].type));
			n = simulate(p->tokens[i].pattern, p->f);
			if (n > 0) {
				long int m = ftell(p->f);
				char *s = emalloc(n + 1);
				struct token *t = emalloc(sizeof *t);

				fseek(p->f, -n, SEEK_CUR);
				fgets(s, n + 1, p->f);
				*t = (struct token){.line = 0, .col = m, .filename = p->filename, .type = p->tokens[i].type, .string = s};
				//fprintf(stderr, "Matched \"%s\" at [%ld:%ld) to token %s.\n", escapes(s), m - n, m, token_name(p->tokens[i].type));
				return t;
			}
			fseek(p->f, n, SEEK_CUR);
		}

		off = ftell(p->f);
		c = fgetc(p->f);
		if (c != EOF) {
			fprintf(stderr, "Cannot match %c at %ld to any token.\n", c, off);
			return NULL;
		}
	}
	t = emalloc(sizeof *t);
	*t = (struct token){.line = 0, .col = -1, .filename = p->filename, .type = TOKEN_EOF, .string = ""};
	return t;
}

int main(int argc, char **argv)
{
	struct mort_parser p;
	struct tok_tokenlist *list = tok_tokenlist_new();
	char *procpath;
	char filename[1024] = {0};

	init_tokens(&p.tokens);

	p.f = fopen("input.txt", "rb");
	if (p.f == NULL) {
		fprintf(stderr, "No such file\n");
		return -1;
	}

	asprintf(&procpath, "/proc/self/fd/%d", fileno(p.f));
	memset(filename, 0, sizeof(filename));
	readlink(procpath, filename, sizeof(filename) - 1);
	free(procpath);
	p.filename = &filename[0];

	// Algorithm:
	// tokens = []
	// while True:
	//   for (name, pattern) in tokens:
	//     run pattern on stream
	//     if it matched:
	//       record token
	//       break for loop / continue while loop
	//     else:
	//       jump back in stream by the number of characters seen by running the pattern
	//   raise an error "could not match any token here"

	while (!feof(p.f) && !ferror(p.f)) {
		struct token *t = get_next_token(&p);
		tok_tokenlist_push(list, t);
		trace_tokenlist("t", list);
	}

	return 0;
}
