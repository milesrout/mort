struct nfa_graph {
	struct nfa_state *initial_state;
	struct nfa_state *final_state;
};

struct nfa_trans {
	// NULL = epsilon-transition
	// ""   = no valid characters
	const char *valid;

	int (*condition)(const char *input, size_t index, size_t length);

	struct nfa_state *endpoint;
};

struct nfa_state {
	struct nfa_trans trans1;
	struct nfa_trans trans2;
};

struct nfa_graph *nfa_symbol(const char *valid);
struct nfa_graph *nfa_string(const char *string);
struct nfa_graph *nfa_keyword(const char *string);
struct nfa_graph *nfa_word_boundary();
struct nfa_graph *nfa_epsilon(void);
struct nfa_graph *nfa_union(struct nfa_graph *s, struct nfa_graph *t);
struct nfa_graph *nfa_concatenation(struct nfa_graph *s, struct nfa_graph *t);
struct nfa_graph *nfa_kleene_star(struct nfa_graph *g);
