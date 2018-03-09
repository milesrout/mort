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
#include "tok_scanner.h"

// The scanner, parser, etc. have a 'pull' structure. Rather than reading the
// entire input file into memory, turning it into tokens, then parsing the rest
// of the file, the file is tokenised lazily as the tokens are required by the
// parser.

static struct token *get_token_nows(struct tok_scanner *s)
{
	struct token *t;
label:
	if ((t = get_token(s))) {
		switch (t->type) {
			case TOKEN_NEWLINE:
			case TOKEN_WS:
				goto label;
		}
	}
	return t;
}

int main(int argc, char **argv)
{
	struct tok_scanner s;
	struct tok_tokenlist *list = tok_tokenlist_new();
	struct token *t;
	char *procpath;
	char filename[1024] = {0};

	init_tokens(&s.tokens);

	s.f = fopen("input.txt", "rb");
	if (s.f == NULL) {
		fprintf(stderr, "No such file\n");
		return -1;
	}

	asprintf(&procpath, "/proc/self/fd/%d", fileno(s.f));
	memset(filename, 0, sizeof(filename));
	readlink(procpath, filename, sizeof(filename) - 1);
	free(procpath);
	s.filename = &filename[0];

	while (!feof(s.f) && !ferror(s.f) && (t = get_token_nows(&s))) {
		tok_tokenlist_push(list, t);
		trace_tokenlist("t", list);
	}

	return 0;
}
