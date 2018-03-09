struct tok_scanner {
	FILE *f;
	struct tok_defn *tokens;
	const char *filename;
};
struct token *get_token(struct tok_scanner *);
void init_tokens(struct tok_defn **_tokens);
