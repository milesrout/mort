#include <unistd.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "util.h"

#include "nfa.h"

// The scanner, parser, etc. have a 'pull' structure. Rather than reading the
// entire input file into memory, turning it into tokens, then parsing the rest
// of the file, the file is tokenised lazily as the tokens are required by the
// parser.

struct token {
	int   type;
	char *string;
	char *filename;
	int   line;
	int   col;
};

const char *alpha = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_";
const char *alnum = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";

struct token_definition {
	char *name;
	struct nfa_graph *pattern;
};

#define NUM_TOKENS 11
void init_tokens(struct token_definition **_tokens)
{
	struct token_definition *tokens = malloc(NUM_TOKENS * (sizeof *tokens));
	tokens[0] = (struct token_definition){"lparen",  nfa_symbol("(")};
	tokens[1] = (struct token_definition){"rparen",  nfa_symbol(")")};
	tokens[2] = (struct token_definition){"lbrace",  nfa_symbol("{")};
	tokens[3] = (struct token_definition){"rbrace",  nfa_symbol("}")};
	tokens[4] = (struct token_definition){"lbrack",  nfa_symbol("[")};
	tokens[5] = (struct token_definition){"rbrack",  nfa_symbol("]")};
	tokens[6] = (struct token_definition){"newline", nfa_symbol("\n")};
	tokens[7] = (struct token_definition){"if",      nfa_string("if")};
	tokens[8] = (struct token_definition){"else",    nfa_string("else")};
	tokens[9] = (struct token_definition){"return",  nfa_string("return")};
	tokens[10] = (struct token_definition){"ident",  nfa_concatenation(nfa_symbol(alpha), nfa_kleene_star(nfa_symbol(alnum)))};

	*_tokens = tokens;
}

static char *
onechar(char c)
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

static char *
escape(char c)
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
	int i = 0;
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
		for (i = 0; i < current->num_states; i++) {
			struct nfa_state *s = current->states[i];
			nfa_statelist_pushmatching(next, s, (char)c);
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

int main(int argc, char **argv)
{
	int i, n;
	FILE *f;
	struct token_definition *tokens;

	init_tokens(&tokens);

	f = fopen("input.txt", "rb");
	if (f == NULL) {
		fprintf(stderr, "No such file\n");
		return -1;
	}


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

	while (!feof(f) && !ferror(f)) {
		long int off;
		char c = fgetc(f);
		if (c == EOF) {
			break;
		}
		ungetc(c, f);
		for (i = 0; i < NUM_TOKENS; i++) {
			fprintf(stderr, "Trying %s.\n", tokens[i].name);
			n = simulate(tokens[i].pattern, f);
			if (n > 0) {
				fprintf(stderr, "Matched [%ld:%ld) to token %s.\n", ftell(f) - n, ftell(f), tokens[i].name);
				goto loopend;
			}
			fseek(f, n, SEEK_CUR);
		}
		off = ftell(f);
		c = fgetc(f);
		if (c == EOF)
			break;
		else
			fprintf(stderr, "Cannot match %c at %ld to any token.\n", c, off);
		return -1;
loopend:	;
	}

	return 0;
}
