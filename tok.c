#define _GNU_SOURCE
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "util.h"
#include "tok.h"

const char *token_name(int type)
{
	switch (type) {
		case TOKEN_EOF:
			return "eof";
		case TOKEN_LPAREN:
			return "lparen";
		case TOKEN_RPAREN:
			return "rparen";
		case TOKEN_LBRACE:
			return "lbrace";
		case TOKEN_RBRACE:
			return "rbrace";
		case TOKEN_LBRACK:
			return "lbrack";
		case TOKEN_RBRACK:
			return "rbrack";
		case TOKEN_NEWLINE:
			return "newline";
		case TOKEN_IF:
			return "if";
		case TOKEN_ELSE:
			return "else";
		case TOKEN_RETURN:
			return "return";
		case TOKEN_IDENT:
			return "ident";
		default:
			fprintf(stderr, "No such token: %d\n", type);
			abort();
	}
}

static int variable_content_tokens[] = {
	TOKEN_IDENT,
};
static size_t num_variable_content_tokens = sizeof(variable_content_tokens) / sizeof(variable_content_tokens[0]);
static int is_variable_content_token(int type)
{
	for (size_t i = 0; i < num_variable_content_tokens; i++)
		if (variable_content_tokens[i] == type)
			return 1;
	return 0;
}

char *token_stringify(struct token *t)
{
	char *result;
	const char *name = token_name(t->type);
	if (is_variable_content_token(t->type))
		asprintf(&result, "%d:%d:%s:%s", t->line, t->col, name, t->string);
	else
		asprintf(&result, "%d:%d:%s", t->line, t->col, name);
	return result;
}

struct tok_tokenlist *tok_tokenlist_new(void)
{
	struct tok_tokenlist *list = emalloc(sizeof *list);

	list->tokens = emalloc(sizeof(struct token));
	list->num_tokens = 0;
	list->capacity = 1;

	return list;
}

void tok_tokenlist_push(struct tok_tokenlist *list, struct token *t)
{
	if (list->num_tokens >= list->capacity)
		tok_tokenlist_expand(list);

	list->tokens[list->num_tokens++] = t;
}

int tok_tokenlist_contains(struct tok_tokenlist *list, struct token *t)
{
	for (ptrdiff_t i = 0; i < list->num_tokens; i++)
		if (list->tokens[i] == t)
			return 1;
	return 0;
}

void tok_tokenlist_expand(struct tok_tokenlist *list)
{
	list->tokens = erealloc(list->tokens, 2 * list->capacity * sizeof(struct token *));
	list->capacity *= 2;
}

void tok_tokenlist_clear(struct tok_tokenlist *list)
{
	list->num_tokens = 0;
}
