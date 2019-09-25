#include <glib.h>
#include "tokens.h"
#include "dataset_gml.h"
static void
token_read(void)
{
    const char a[] = "graph [\n  node [\n    id 0\n    label \"1\"\n  ]\n  node [\n    id 1\n    label \"2\"\n  ]\n  node [\n    id 2\n    label \"3\"\n  ]\n  edge [\n    source 0\n    target 1\n  ]\n  edge [\n    source 0\n    target 2\n  ]\n]\n";
    Tokens* toks = tokens_open_from_pipe_string(a);
    char* next;
    while (tokens_has_next(toks)) {
        next = tokens_next(toks);
        g_free(next);
    }
    tokens_close(toks);
}
static void dataset_load(void)
{
    const char a[] = "graph [\n  node [\n    id 0\n    label \"1\"\n  ]\n  node [\n    id 1\n    label \"2\"\n  ]\n  node [\n    id 2\n    label \"3\"\n  ]\n  edge [\n    source 0\n    target 1\n  ]\n  edge [\n    source 0\n    target 2\n  ]\n]\n";
    Dataset* dd = dataset_gml_load(a);
    dataset_unref(dd);
}
int
main(int   argc,
    char* argv[])
{
    g_test_init(&argc, &argv, NULL);
    g_test_add_func("/testpipeio/token_read", token_read);
    g_test_add_func("/testpipeio/dataset_load", dataset_load);
    return g_test_run();
}