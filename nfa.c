#define _GNU_SOURCE
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "nfa.h"

// chemotherapy
#define asprintf(...) asprintf((char**) __VA_ARGS__)

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

static char *
_(const char *string)
{
	int i;
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
			if (strchr("[]{}()*+?/\\", string[i]) != NULL) {
				*p++ = '\\';
			}
			*p++ = string[i];
		}
	}
	*p++ = '\0';

	return erealloc(new, strlen(new) + 1);
}

struct nfa_graph *
nfa_phantom(void)
{
	struct nfa_state *f, *q;
	struct nfa_graph *g;

	f = emalloc(sizeof *f);
	asprintf(&f->name, "(nil-f)");
	f->trans1.endpoint = NULL;
	f->trans2.endpoint = NULL;

	q = emalloc(sizeof *q);
	asprintf(&q->name, "(nil-q)");
	q->trans1.endpoint = NULL;
	q->trans2.endpoint = NULL;

	g = emalloc(sizeof *g);
	asprintf(&g->name, "(nil-g)");
	g->initial_state = q;
	g->final_state = f;

	return g;
}

struct nfa_graph *
nfa_symbol(const char *valid)
{
	struct nfa_state *f, *q;
	struct nfa_graph *graph;

	f = emalloc(sizeof *f);
	asprintf(&f->name, "nfa_symbol /[%s]/ final", _(valid));
	f->trans1.endpoint = NULL;
	f->trans2.endpoint = NULL;

	q = emalloc(sizeof *q);
	asprintf(&q->name, "nfa_symbol /[%s]/ initial", _(valid));
	q->trans1.endpoint = f;
	q->trans1.valid = valid;
	q->trans2.endpoint = NULL;

	graph = emalloc(sizeof *graph);
	asprintf(&graph->name, "[%s]", _(valid));
	graph->initial_state = q;
	graph->final_state = f;

	return graph;
}

struct nfa_graph *
nfa_string(const char *string)
{
	struct nfa_state *state;
	struct nfa_graph *graph;
	char c;
	int i, n;

	i = 0;
	n = strlen(string);

	state = emalloc(sizeof *state);
	asprintf(&state->name, "nfa_string /%s/ initial", _(string));
	state->trans1.endpoint = NULL;
	state->trans2.endpoint = NULL;

	graph = emalloc(sizeof *graph);
	asprintf(&graph->name, "%s", _(string));
	graph->initial_state = state;

	while ((c = *string++) != '\0') {
		struct nfa_state *new = emalloc(sizeof *new);
		asprintf(&new->name, "nfa_string /%s/ %d of %d", _(string), i, n);
		new->trans1.endpoint = NULL;
		new->trans2.endpoint = NULL;

		state->trans1.endpoint = new;
		state->trans1.valid = onechar(c);

		state = new;
		i++;
	}

	graph->final_state = state;

	return graph;
}

struct nfa_graph *
nfa_word_boundary()
{
	struct nfa_graph *graph;
	struct nfa_state *f, *q;

	f = emalloc(sizeof *f);
	f->name = "nfa_word_boundary final";
	f->trans1.endpoint = NULL;
	f->trans2.endpoint = NULL;

	q = emalloc(sizeof *q);
	q->name = "nfa_word_boundary initial";
	q->trans1.endpoint = f;
	q->trans1.valid = "";
	q->trans2.endpoint = NULL;

	graph = emalloc(sizeof *graph);
	asprintf(&graph->name, "\\b");
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
	f->name = "nfa_union final";
	f->trans1.endpoint = NULL;
	f->trans2.endpoint = NULL;

	q = emalloc(sizeof *q);
	q->name = "nfa_union initial";
	q->trans1.endpoint = s->initial_state;
	q->trans1.valid = NULL;
	q->trans2.endpoint = t->initial_state;
	q->trans2.valid = NULL;

	s->final_state->trans1.endpoint = f;
	s->final_state->trans1.valid = NULL;

	t->final_state->trans1.endpoint = f;
	t->final_state->trans1.valid = NULL;

	graph = emalloc(sizeof *graph);
	asprintf(&graph->name, "(%s|%s)", s->name, t->name);
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

	graph = emalloc(sizeof *graph);
	asprintf(&graph->name, "%s%s", s->name, t->name);
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
	f->name = "nfa_kleene_star final";
	f->trans1.endpoint = NULL;
	f->trans2.endpoint = NULL;

	g->final_state->trans1.endpoint = g->initial_state;
	g->final_state->trans1.valid = NULL;
	g->final_state->trans2.endpoint = f;
	g->final_state->trans2.valid = NULL;

	q = emalloc(sizeof *q);
	q->name = "nfa_kleene_star initial";
	q->trans1.endpoint = g->initial_state;
	q->trans1.valid = NULL;
	q->trans2.endpoint = f;
	q->trans2.valid = NULL;

	graph = emalloc(sizeof *graph);
	asprintf(&graph->name, "(%s)*", g->name);
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

void
nfa_statelist_expand(struct nfa_statelist *list)
{
	list->states = erealloc(list->states, 2 * list->capacity * sizeof(struct nfa_state *));
	list->capacity *= 2;
}

int
nfa_statelist_contains(struct nfa_statelist *list, struct nfa_state *s)
{
	for (ptrdiff_t i = 0; i < list->num_states; i++)
		if (list->states[i] == s)
			return 1;
	return 0;
}

void
nfa_statelist_push(struct nfa_statelist *list, struct nfa_state *s)
{
	if (list->num_states >= list->capacity) {
		nfa_statelist_expand(list);
	}

	list->states[list->num_states++] = s;
}

void
nfa_statelist_pushclosure(struct nfa_statelist *list, struct nfa_state *s)
{
	nfa_statelist_push(list, s);
	if (s->trans1.endpoint != NULL && s->trans1.valid == NULL)
		if (!nfa_statelist_contains(list, s->trans1.endpoint))
			nfa_statelist_pushclosure(list, s->trans1.endpoint);
	if (s->trans2.endpoint != NULL && s->trans2.valid == NULL)
		if (!nfa_statelist_contains(list, s->trans2.endpoint))
			nfa_statelist_pushclosure(list, s->trans2.endpoint);
}

void
nfa_statelist_pushmatching(struct nfa_statelist *list, struct nfa_state *s, char c)
{
	if (s->trans1.endpoint != NULL && s->trans1.valid != NULL)
		if (strchr(s->trans1.valid, c) != NULL)
			if (!nfa_statelist_contains(list, s->trans1.endpoint))
				nfa_statelist_pushclosure(list, s->trans1.endpoint);

	if (s->trans2.endpoint != NULL && s->trans2.valid != NULL)
		if (strchr(s->trans2.valid, c) != NULL)
			if (!nfa_statelist_contains(list, s->trans2.endpoint))
				nfa_statelist_pushclosure(list, s->trans2.endpoint);
}

struct nfa_statelist *
nfa_statelist_new(void)
{
	struct nfa_statelist *list = emalloc(sizeof *list);

	list->states = emalloc(sizeof(struct nfa_state *));
	list->num_states = 0;
	list->capacity = 1;

	return list;
}

void
nfa_statelist_clear(struct nfa_statelist *list)
{
	list->num_states = 0;
}
