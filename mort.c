#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void init_tokens(struct token_definition **_tokens)
{
	struct token_definition *tokens = malloc(8 * (sizeof *tokens));
	tokens[0] = (struct token_definition){"lparen", nfa_symbol("(")};
	tokens[1] = (struct token_definition){"rparen", nfa_symbol(")")};
	tokens[2] = (struct token_definition){"lbrace", nfa_symbol("{")};
	tokens[3] = (struct token_definition){"rbrace", nfa_symbol("}")};
	tokens[4] = (struct token_definition){"ident",  nfa_concatenation(nfa_symbol(alpha), nfa_kleene_star(nfa_symbol(alnum)))};
	tokens[5] = (struct token_definition){"if",     nfa_keyword("if")};
	tokens[6] = (struct token_definition){"else",   nfa_keyword("else")};
	tokens[7] = (struct token_definition){"return", nfa_keyword("return")};

	*_tokens = tokens;
}

int main(int argc, char **argv)
{
	struct token_definition *tokens;

	init_tokens(&tokens);

	return 0;
}
