struct nfa_graph {
	struct nfa_state *initial_state;
	struct nfa_state *final_state;
	const char *name; /* lifetime should be managed separately from this object */
};
struct nfa_trans {
	const char *valid;
	struct nfa_state *endpoint;
};
struct nfa_state {
	struct nfa_trans trans1;
	struct nfa_trans trans2;
	const char *name; /* lifetime should be managed separately from this object */
};
extern struct nfa_graph *nfa_never(void);
extern struct nfa_graph *nfa_anybut(const char *invalid);
extern struct nfa_graph *nfa_symbol(const char *valid);
extern struct nfa_graph *nfa_string(const char *string);
extern struct nfa_graph *nfa_epsilon(void);
extern struct nfa_graph *nfa_union(struct nfa_graph *s, struct nfa_graph *t);
extern struct nfa_graph *nfa_concat(struct nfa_graph *s, struct nfa_graph *t);
extern struct nfa_graph *nfa_kleene_star(struct nfa_graph *g);
struct nfa_statelist {
	struct nfa_state **states;
	ptrdiff_t num_states;
	ptrdiff_t capacity;
};
extern struct nfa_statelist *nfa_statelist_new(void);
extern void nfa_statelist_push(struct nfa_statelist *, struct nfa_state *);
extern void nfa_statelist_pushclosure(struct nfa_statelist *, struct nfa_state *);
extern void nfa_statelist_pushmatching(struct nfa_statelist *, struct nfa_state *, char c);
extern int nfa_statelist_contains(struct nfa_statelist *, struct nfa_state *);
extern void nfa_statelist_expand(struct nfa_statelist *);
extern void nfa_statelist_clear(struct nfa_statelist *);
#define trace_statelist_abbrev(code, list)\
	do {\
		struct nfa_statelist *_l = (list);\
		if (_l->num_states)\
			tracef(" - " code "ns:%lu last:{%s}", _l->num_states, _l->states[_l->num_states - 1]->name);\
		else\
			tracef(" - " code "ns:%lu", _l->num_states);\
	} while (0)
#define trace_statelist(code, list)\
	do {\
		struct nfa_statelist *_l = (list);\
		int _i;\
		tracefx(" - " code "ns:%lu last:", _l->num_states);\
		for (_i = 0; _i < _l->num_states; _i++) {\
			fprintf(stderr, "{%s} ", _l->states[_i]->name);\
		}\
		fprintf(stderr, "\n");\
	} while (0)
