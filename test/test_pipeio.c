#include <glib.h>
#if _WIN32
#include "tokens.h"
#endif
#if _WIN32
static void
token_read(void)
{
    const char a[] = "graph [\n  node [\n    id 0\n    label \"1\"\n  ]\n  node [\n    id 1\n    label \"2\"\n  ]\n  node [\n    id 2\n    label \"3\"\n  ]\n  edge [\n    source 0\n    target 1\n  ]\n  edge [\n    source 0\n    target 2\n  ]\n]\n";
    Tokens* toks = tokens_open_from_pipe_string(a, strlen(a));
    char* next;
    while (tokens_has_next(toks)) {
        next = tokens_next(toks);
        g_free(next);
    }
    tokens_close(toks);
}
#endif
int
main(int   argc,
    char* argv[])
{
    g_test_init(&argc, &argv, NULL);
#if _WIN32
    g_test_add_func("/testpipeio/token_read", token_read);
#endif
    return g_test_run();
}