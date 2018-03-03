#define trace()        tracef("")
#define tracef(...)    fprintf(stderr, "trace: %s:%d", __FILE__, __LINE__); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n") 
#define emalloc(n)     ({ void *p = malloc(n); if (p == NULL) { fprintf(stderr, "%s:%d: Could not allocate %lu bytes\n", __FILE__, __LINE__, (size_t)(n)); abort(); } p; })
#define erealloc(p, n) ({ void *q = realloc((p), (n)); if (q == NULL) { fprintf(stderr, "%s:%d: Could not reallocate %p to %lu bytes\n", __FILE__, __LINE__, (p), (size_t)(n)); abort(); } q; })
