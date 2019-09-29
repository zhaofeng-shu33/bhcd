#include "tree_io.h"
#include "util.h"
#include "version.h"
#if _WIN32
#include <io.h>
#include <fcntl.h>
#endif

Tree * tree_io_load(const gchar *fname) {
	return NULL;
}

void tree_io_save(Tree *tree, gchar *fname_or_strbuffer) {
	io_writefile(fname_or_strbuffer, (IOFunc)tree_io_save_io, tree);
}

void tree_io_save_string(Tree* tree, gchar** strbufferptr) {
    GIOChannel* io;
    GError* error = NULL;
    int fd[2];
#if _WIN32
        guint label_num = labelset_count(tree->labels);
        if (-1 == _pipe(fd, label_num * 1024, _O_TEXT))
            g_error("open write pipe failed");
        io = g_io_channel_win32_new_fd(fd[1]);
#else
        pipe(fd);
        io = g_io_channel_unix_new(fd[1]);
#endif
    tree_io_save_io(tree, io);
    g_io_channel_shutdown(io, TRUE, &error);
    if (error != NULL) {
        g_error("shutdown: %s", error->message);
    }
    
    // read from the pipe
    gsize io_size;
#if _WIN32
    GIOChannel* io_read = g_io_channel_win32_new_fd(fd[0]);
#else
    GIOChannel* io_read = g_io_channel_unix_new(fd[0]);
#endif
    gchar* str_pointer;
    g_io_channel_read_to_end(io_read, &str_pointer, &io_size, &error);
    *strbufferptr = str_pointer;
    if (error != NULL) {
        g_error("read error with message : %s", error->message);
    }
    g_io_channel_shutdown(io_read, TRUE, &error);
    if (error != NULL) {
        g_error("shutdown error with message: %s", error->message);
    }
    g_io_channel_unref(io_read);
    g_io_channel_unref(io);
}

void tree_io_save_io(Tree *root, GIOChannel *io) {
	GQueue * qq;
	Dataset * dataset;
	Params * params;
	gint next_index;

	params = tree_get_params(root);
	dataset = params->dataset;

	io_printf(io, "{\n");
	io_printf(io, "\t\"data\": { \"file\": \"%s\" },\n", dataset_get_filename(dataset));
	/* IEEE754 numbers have at most 17 dp of precision. */
	io_printf(io, "\t\"params\": { \"gamma\": %1.17e, \"alpha\": %1.17e, \"beta\": %1.17e, \"delta\": %1.17e, \"lambda\": %1.17e },\n",
			params->gamma,
			params->alpha, params->beta,
			params->delta, params->lambda
			);
	io_printf(io, "\t\"fit\": { \"logprob\": %1.17e, \"method\": \"%s\" },\n",
			tree_get_logprob(root),
			BHCD_NAME_VERSION);
	io_printf(io, "\t\"tree\": [\n");

	qq = g_queue_new();
	next_index = -1;
	g_queue_push_head(qq, pair_new(GINT_TO_POINTER(next_index), root));
	next_index++;

	while (!g_queue_is_empty(qq)) {
		Pair *cur;
		Tree *tree;
		gint parent_index;

		cur = g_queue_pop_head(qq);
		parent_index = GPOINTER_TO_INT(cur->fst);
		tree = cur->snd;

		if (tree_is_leaf(tree)) {
			io_printf(io, "\t\t, {\"leaf\": { \"logprob\": %1.17e, \"logresp\": %1.17e, \"parent\": %d, \"label\": \"%s\" }}\n",
					tree_get_logprob(tree),
					tree_get_logresponse(tree),
					parent_index,
					dataset_label_to_string(dataset, leaf_get_label(tree))
				);
		} else {
			GList * children;
			guint my_index;

			my_index = next_index;
			next_index++;

			if (parent_index == -1) {
				io_printf(io, "\t\t  {\"root\": { \"logprob\": %1.17e, \"logresp\": %1.17e, \"id\": %d }}\n",
						tree_get_logprob(tree),
						tree_get_logresponse(tree),
						my_index
					);
			} else {
				io_printf(io, "\t\t, {\"stem\": { \"logprob\": %1.17e, \"logresp\": %1.17e, \"parent\": %d, \"child\": %d }}\n",
						tree_get_logprob(tree),
						tree_get_logresponse(tree),
						parent_index,
						my_index
					);
			}
			children = g_list_sort(g_list_copy(branch_get_children(tree)), tree_cmp_label);
			for (GList * child = children; child != NULL; child = g_list_next(child)) {
				g_queue_push_tail(qq, pair_new(GINT_TO_POINTER(my_index), child->data));
			}
			g_list_free(children);
		}
		pair_free(cur);
	}
	g_queue_free(qq);
	io_printf(io, "\t]\n}\n");
}

