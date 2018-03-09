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
		case TOKEN_EOF:        return "eof";
		case TOKEN_LPAREN:     return "lparen";
		case TOKEN_RPAREN:     return "rparen";
		case TOKEN_LBRACE:     return "lbrace";
		case TOKEN_RBRACE:     return "rbrace";
		case TOKEN_LBRACK:     return "lbrack";
		case TOKEN_RBRACK:     return "rbrack";
		case TOKEN_LANGLE:     return "langle";
		case TOKEN_RANGLE:     return "rangle";
		case TOKEN_ASTERISK:   return "asterisk";
		case TOKEN_PLUS:       return "plus";
		case TOKEN_MINUS:      return "minus";
		case TOKEN_TILDE:      return "tilde";
		case TOKEN_FWDSLASH:   return "fwdslash";
		case TOKEN_BACKSLASH:  return "backslash";
		case TOKEN_PERCENT:    return "percent";
		case TOKEN_HAT:        return "hat";
		case TOKEN_PIPE:       return "pipe";
		case TOKEN_AMPERSAND:  return "ampersand";
		case TOKEN_EXCLAIM:    return "exclaim";
		case TOKEN_SEMI:       return "semi";
		case TOKEN_COLON:      return "colon";
		case TOKEN_COMMA:      return "comma";
		case TOKEN_DOT:        return "dot";
		case TOKEN_EQUAL:      return "equal";
		case TOKEN_QUESTION:   return "question";
		case TOKEN_ARROW:      return "arrow";
		case TOKEN_INCR:       return "incr";
		case TOKEN_DECR:       return "decr";
		case TOKEN_LSHIFT:     return "lshift";
		case TOKEN_RSHIFT:     return "rshift";
		case TOKEN_LEQUAL:     return "lequal";
		case TOKEN_REQUAL:     return "requal";
		case TOKEN_EQUALS:     return "equals";
		case TOKEN_NEQUAL:     return "nequal";
		case TOKEN_AND:        return "and";
		case TOKEN_OR:         return "or";
		case TOKEN_ELLIPSIS:   return "ellipsis";
		case TOKEN_MULEQUAL:   return "mulequal";
		case TOKEN_DIVEQUAL:   return "divequal";
		case TOKEN_MODEQUAL:   return "modequal";
		case TOKEN_ADDEQUAL:   return "addequal";
		case TOKEN_SUBEQUAL:   return "subequal";
		case TOKEN_LSHEQUAL:   return "lshequal";
		case TOKEN_RSHEQUAL:   return "rshequal";
		case TOKEN_BANDEQUAL:  return "bandequal";
		case TOKEN_BXOREQUAL:  return "bxorequal";
		case TOKEN_BOREQUAL:   return "borequal";
		case TOKEN_NEWLINE:    return "newline";
		case TOKEN_BREAK:      return "break";
		case TOKEN_CASE:       return "case";
		case TOKEN_CONTINUE:   return "continue";
		case TOKEN_DEFAULT:    return "default";
		case TOKEN_CHAR:       return "char";
		case TOKEN_DO:         return "do";
		case TOKEN_ELSE:       return "else";
		case TOKEN_ENUM:       return "enum";
		case TOKEN_EXTERN:     return "extern";
		case TOKEN_FLOAT:      return "float";
		case TOKEN_FOR:        return "for";
		case TOKEN_GOTO:       return "goto";
		case TOKEN_IF:         return "if";
		case TOKEN_INT:        return "int";
		case TOKEN_LONG:       return "long";
		case TOKEN_OPEN:       return "open";
		case TOKEN_CLOSED:     return "closed";
		case TOKEN_RETURN:     return "return";
		case TOKEN_SHORT:      return "short";
		case TOKEN_SIGNED:     return "signed";
		case TOKEN_SIZEOF:     return "sizeof";
		case TOKEN_STATIC:     return "static";
		case TOKEN_STRUCT:     return "struct";
		case TOKEN_SWITCH:     return "switch";
		case TOKEN_UNION:      return "union";
		case TOKEN_UNSIGNED:   return "unsigned";
		case TOKEN_VOID:       return "void";
		case TOKEN_VOLATILE:   return "volatile";
		case TOKEN_WHILE:      return "while";
		case TOKEN_WS:         return "ws";
		case TOKEN_IDENT:      return "ident";
		case TOKEN_INTEGER:    return "integer";
		case TOKEN_STRING:     return "string";
		case TOKEN_CHARACTER:  return "character";
		default:
			fprintf(stderr, "No such token: %d\n", type);
			abort();
	}
}

static int variable_content_tokens[] = {
	TOKEN_IDENT,
	TOKEN_INT,
	TOKEN_STRING,
	TOKEN_CHAR,
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
