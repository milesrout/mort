#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "token.h"

#define trace() tracef("")
#define tracef(...)\
	fprintf(stderr, "trace: %s:%d", __FILE__, __LINE__);\
	fprintf(stderr, __VA_ARGS__);\
	fprintf(stderr, "\n")

// The scanner, parser, etc. have a 'pull' structure. Rather than reading the
// entire input file into memory, turning it into tokens, then parsing the rest
// of the file, the file is tokenised lazily as the tokens are required by the
// parser.

struct token_definition {
	char *name;
	char *regexp;
};

char *keywords[] = {
	"if",
	"else",
	"return",
};

struct token_definition nonkeywords[] = {
	{"lparen", "\\("},
	{"rparen", "\\)"},
	{"lbrace", "\\{"},
	{"rbrace", "\\}"},
};

size_t tokens_total(struct token_definition *tokens, size_t n)
{
	size_t total = 0;
	int i;

	for (i = 0; i < n; i++) {
		total += strlen(tokens[i].name) + 1;
		total += strlen(tokens[i].regexp) + 1;
		total += sizeof(struct token_definition);
	}

	return total;
}

size_t keywords_total(char **keywords, size_t n)
{
	size_t total;
	int i;

	for (i = 0; i < n; i++) {
		size_t length = strlen(keywords[i]);
		total += length + 1;
		total += length + 5;
		total += 2 * sizeof(char *);
	}

	return total;
}

int colocated_token_definitions(struct token_definition **_tokens, size_t *_n)
{
	int i;
	char *p;
	size_t tn = sizeof(nonkeywords) / sizeof(nonkeywords[0]);
	size_t kn = sizeof(keywords) / sizeof(keywords[0]);
	size_t tt = tokens_total(nonkeywords, tn);
	size_t kt = keywords_total(keywords, kn);
	struct token_definition *tokens_original;
	struct token_definition *tokens;
	char *after_tokens;
	
	tokens_original = tokens = malloc(tt + kt);
	if (tokens == NULL) {
		*_tokens = NULL;
		*_n = 0;
		return -1;
	}

	after_tokens = (char*)(tokens + tn + kn);

	for (i = 0; i < kn; i++) {
		struct token_definition token;
		size_t n = strlen(keywords[i]);

		token.name = after_tokens;
		memcpy(after_tokens, keywords[i], n + 1);
		after_tokens += n + 1;

		token.regexp = after_tokens;
		*after_tokens++ = '\\';
		*after_tokens++ = 'b';
		memcpy(after_tokens, keywords[i], n);
		after_tokens += n;
		*after_tokens++ = '\\';
		*after_tokens++ = 'b';
		*after_tokens++ = '\0';

		*tokens++ = token;
	}
	for (i = 0; i < tn; i++) {
		struct token_definition token;
		size_t n = strlen(nonkeywords[i].name) + 1;
		size_t m = strlen(nonkeywords[i].regexp) + 1;

		token.name = after_tokens;
		memcpy(after_tokens, nonkeywords[i].name, n);
		after_tokens += n;

		token.regexp = after_tokens;
		memcpy(after_tokens, nonkeywords[i].regexp, m);
		after_tokens += m;

		*tokens++ = token;
	}

	*_tokens = tokens_original;
	*_n = tn + kn;
	return 0;
}

int main(int argc, char **argv)
{
	struct token_definition *tokens;
	size_t n;
	int err, i;
	
	if (0 != (err = colocated_token_definitions(&tokens, &n))) {
		fprintf(stderr, "[%d]: Cannot consolidate token definitions\n", err);
		return -1;
	}
	
	tracef(" - t:%p n:%lu", tokens, n);

	for (i = 0; i < n; i++) {
		//tracef(" %d", i);
		printf("%10s := %s\n", tokens[i].name, tokens[i].regexp);
	}

	return 0;
}
