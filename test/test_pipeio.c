#include <glib.h>
#include "tree.h"
#include "tokens.h"
#include "dataset_gml.h"
#include "build.h"

static Tree* test_run(GRand* rng, Dataset* dataset) {
    Params* params;
    Tree* root;
    Build* build;

    params = params_new(dataset,
        0.4,
        1.0, 0.2,
        0.2, 1.0);
    params->binary_only = 0;

    build = build_new(rng, params, 1, 0);
    build_set_verbose(build, 0);
    params_unref(params);
    build_run(build);
    root = build_get_best_tree(build);
    tree_ref(root);
    build_free(build);

    g_assert(tree_num_leaves(root) == dataset_num_labels(dataset));
    return root;
}

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
static void write_result(void)
{
    const char a[] = "graph [\n  node [\n    id 0\n    label \"1\"\n  ]\n  node [\n    id 1\n    label \"2\"\n  ]\n  node [\n    id 2\n    label \"3\"\n  ]\n  edge [\n    source 0\n    target 1\n  ]\n  edge [\n    source 0\n    target 2\n  ]\n]\n";
    Dataset* dataset = dataset_gml_load(a);
    GRand* rng = g_rand_new();
    Tree* root = test_run(rng, dataset);
    gchar* strbuffer = NULL;
    tree_io_save(root, strbuffer);
    tree_unref(root);
    g_rand_free(rng);
    g_free(strbuffer);
}
int
main(int   argc,
    char* argv[])
{
    g_test_init(&argc, &argv, NULL);
    g_test_add_func("/testpipeio/token_read", token_read);
    g_test_add_func("/testpipeio/dataset_load", dataset_load);
    g_test_add_func("/testpipeio/write_result", write_result);
    return g_test_run();
}