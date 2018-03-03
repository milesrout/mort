#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define trace()        tracef("")
#define tracef(...)    fprintf(stderr, "trace: %s:%d", __FILE__, __LINE__); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n") 
#define emalloc(n)     ({ void *p = malloc(n); if (p == NULL) { fprintf(stderr, "%s:%d: Could not allocate %lu bytes\n", __FILE__, __LINE__, (size_t)(n)); abort(); } p; })
#define erealloc(p, n) ({ void *q = realloc((p), (n)); if (q == NULL) { fprintf(stderr, "%s:%d: Could not reallocate %p to %lu bytes\n", __FILE__, __LINE__, (p), (size_t)(n)); abort(); } q; })

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

struct nfa_graph *nfa_symbol(const char *);
struct nfa_graph *nfa_keyword(const char *);
struct nfa_graph *nfa_concatenation(struct nfa_graph *, struct nfa_graph *);
struct nfa_graph *nfa_kleene_star(struct nfa_graph *);

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

// represents the temporary state necessary for running an NFA over a string
struct nfa_parser;

struct nfa_graph;
struct nfa_trans;
struct nfa_graph;

struct nfa_forest {
	size_t num_graphs;
	struct nfa_graph *graphs[];
};
#define NFA_FOREST_SIZE(num_graphs) (sizeof(struct nfa_forest) + (num_graphs) * sizeof(struct nfa_graph *))

struct nfa_graph {
	struct nfa_state *initial_state;
	struct nfa_state *final_state;
};

// represents an edge in an NFA graph
struct nfa_trans {
	// NULL = epsilon-transition
	// ""   = no valid characters
	const char *valid;

	int (*condition)(const char *input, size_t index, size_t length);

	struct nfa_state *endpoint;
};

// represents a state in an NFA graph
struct nfa_state {
	struct nfa_trans trans1;
	struct nfa_trans trans2;
};

struct nfa_graph *
nfa_symbol(const char *valid)
{
	struct nfa_state *f, *q;
	struct nfa_graph *graph;

	f = emalloc(sizeof *f);
	f->trans1.endpoint = NULL;
	f->trans2.endpoint = NULL;

	q = emalloc(sizeof *q);
	q->trans1.endpoint = f;
	q->trans1.valid = valid;
	q->trans1.condition = NULL;
	q->trans2.endpoint = NULL;

	graph = emalloc(sizeof *graph);
	graph->initial_state = q;
	graph->final_state = f;

	return graph;
}

char *onechar(char c)
{
	char *s = emalloc(2);
	s[0] = c;
	s[1] = '\0';
	return s;
}

struct nfa_graph *
nfa_string(const char *string)
{
	struct nfa_state *state;
	struct nfa_graph *graph;
	char c;

	state = emalloc(sizeof *state);
	state->trans1.endpoint = NULL;
	state->trans2.endpoint = NULL;

	graph = emalloc(sizeof *graph);
	graph->initial_state = state;

	while ((c = *string++) != '\0') {
		struct nfa_state *new = emalloc(sizeof *new);
		new->trans1.endpoint = NULL;
		new->trans2.endpoint = NULL;

		state->trans1.endpoint = new;
		state->trans1.valid = onechar(c);
		state->trans1.condition = NULL;

		state = new;
	}

	graph->final_state = state;

	return graph;
}

struct nfa_parser {
	struct nfa_graph *graph;
	const char *input;
	size_t length;
	size_t index;
};

int isword(int ch)
{
	return ch == '_' || isalnum(ch);
}

int is_word_boundary(const char *input, size_t index, size_t length)
{
	if (index > length) {
		fprintf(stderr, "Invalid index: %lu, should be in [0,%lu)\n", index, length);
		abort();
	}

	if (index == length) {
		return isword(input[index - 1]);
	}

	if (index == 0 || index == length - 1)
		if (isword(input[index]))
			return 1;

	if (index < length - 1 && isword(input[index]) != isword(input[index + 1]))
		return 1;

	if (index > 0 && isword(input[index]) != isword(input[index - 1]))
		return 1;

	return 0;
}

struct nfa_graph *
nfa_word_boundary()
{
	struct nfa_graph *graph;
	struct nfa_state *f, *q;

	f = emalloc(sizeof *f);
	f->trans1.endpoint = NULL;
	f->trans2.endpoint = NULL;

	q = emalloc(sizeof *q);
	q->trans1.endpoint = f;
	q->trans1.valid = "";
	q->trans1.condition = is_word_boundary;
	q->trans2.endpoint = NULL;

	graph = emalloc(sizeof *graph);
	graph->initial_state = q;
	graph->final_state = f;

	return graph;
}

struct nfa_graph *
nfa_epsilon(void)
{
	return nfa_symbol(NULL);
}

struct nfa_graph *
nfa_union(struct nfa_graph *s, struct nfa_graph *t)
{
	struct nfa_state *f, *q;
	struct nfa_graph *graph;

	f = emalloc(sizeof *f);
	f->trans1.endpoint = NULL;
	f->trans2.endpoint = NULL;

	q = emalloc(sizeof *q);
	q->trans1.endpoint = s->initial_state;
	q->trans1.valid = NULL;
	q->trans1.condition = NULL;
	q->trans2.endpoint = t->initial_state;
	q->trans2.valid = NULL;
	q->trans2.condition = NULL;

	s->final_state->trans1.endpoint = f;
	s->final_state->trans1.valid = NULL;
	s->final_state->trans1.condition = NULL;
	t->final_state->trans1.endpoint = f;
	t->final_state->trans1.valid = NULL;
	t->final_state->trans1.condition = NULL;

	graph = emalloc(sizeof *graph);
	graph->initial_state = q;
	graph->final_state = f;

	return graph;
}

// Could instead record a list of state equations and set s_f = t_i
// then use that information when converting to a DFA
struct nfa_graph *
nfa_concatenation(struct nfa_graph *s, struct nfa_graph *t)
{
	struct nfa_graph *graph;

	s->final_state->trans1.endpoint = t->initial_state;
	s->final_state->trans1.valid = NULL;
	s->final_state->trans1.condition = NULL;

	graph = emalloc(sizeof *graph);
	graph->initial_state = s->initial_state;
	graph->final_state = t->final_state;

	return graph;
}

struct nfa_graph *
nfa_kleene_star(struct nfa_graph *g)
{
	struct nfa_state *f, *q;
	struct nfa_graph *graph;

	f = emalloc(sizeof *f);
	f->trans1.endpoint = NULL;
	f->trans2.endpoint = NULL;

	g->final_state->trans1.endpoint = g->initial_state;
	g->final_state->trans1.valid = NULL;
	g->final_state->trans1.condition = NULL;
	g->final_state->trans2.endpoint = f;
	g->final_state->trans2.valid = NULL;
	g->final_state->trans2.condition = NULL;

	q = emalloc(sizeof *q);
	q->trans1.endpoint = g->initial_state;
	q->trans1.valid = NULL;
	q->trans1.condition = NULL;
	q->trans2.endpoint = f;
	q->trans2.valid = NULL;
	q->trans2.condition = NULL;

	graph = emalloc(sizeof *graph);
	graph->initial_state = q;
	graph->final_state = f;

	return graph;
}

struct nfa_graph *
nfa_keyword(const char *string)
{
	return nfa_concatenation(nfa_word_boundary(),
	           nfa_concatenation(nfa_string(string), nfa_word_boundary()));
}

int main(int argc, char **argv)
{
	struct token_definition *tokens;

	init_tokens(&tokens);

	return 0;
}
