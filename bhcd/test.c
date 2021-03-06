#include <glib.h>
#include <gsl/gsl_sf_log.h>
#include <gsl/gsl_sf_exp.h>
#include <gsl/gsl_sf_gamma.h>
#include <gsl/gsl_sf_pow_int.h>
#include "bhcd.h"


void init_test_toy3(Tree **laa, Tree **lbb, Tree **lcc) {
	Dataset *dataset;
	Params *params;
	gpointer aa, bb, cc;

	dataset = dataset_gen_toy3();
	/*dataset_println(dataset, "");*/
	aa = dataset_label_lookup(dataset, "aa");
	for (guint ii = 0; ii < 64; ii++) {
		gchar *str = num_to_string(ii);
		dataset_label_create(dataset, str);
		g_free(str);
	}
	bb = dataset_label_lookup(dataset, "bb");
	cc = dataset_label_lookup(dataset, "cc");
	params = params_default(dataset);

	/* leaves */
	*laa = leaf_new(params, aa);
	*lbb = leaf_new(params, bb);
	*lcc = leaf_new(params, cc);

	params_unref(params);
	dataset_unref(dataset);
}

void init_test_toy4(Tree **laa, Tree **lbb, Tree **lcc, Tree **ldd) {
	Dataset *dataset;
	Params *params;
	gpointer aa, bb, cc, dd;

	dataset = dataset_gen_toy4(FALSE);
	/* dataset_println(dataset, ""); */
	aa = dataset_label_lookup(dataset, "aa");
	bb = dataset_label_lookup(dataset, "bb");
	cc = dataset_label_lookup(dataset, "cc");
	dd = dataset_label_lookup(dataset, "dd");
	params = params_default(dataset);

	/* leaves */
	*laa = leaf_new(params, aa);
	*lbb = leaf_new(params, bb);
	*lcc = leaf_new(params, cc);
	*ldd = leaf_new(params, dd);

	params_unref(params);
	dataset_unref(dataset);
}

void test_tree_logprob3(void) {
	Params * params;
	Params * new_params;
	Tree *laa, *lbb, *lcc;
	Tree *tab, *tac, *tbc, *tabc;
	gdouble prec;
	gdouble correct_tab, correct_tac, correct_tabc;

	prec = 1e-4;
	init_test_toy3(&laa, &lbb, &lcc);

	g_assert(tree_get_params(laa) == tree_get_params(lbb));
	g_assert(tree_get_params(lcc) == tree_get_params(lbb));
	params = tree_get_params(laa);
	params_ref(params);

	assert_eqfloat(tree_get_logprob(laa), 0.0, prec);
	assert_eqfloat(tree_get_logprob(lbb), 0.0, prec);
	assert_eqfloat(tree_get_logprob(lcc), 0.0, prec);

	/* pairs */
	tab = branch_new(params);
	g_assert(tree_num_leaves(tab) == 0);
	g_assert(tree_num_intern(tab) == 1);
	branch_add_child(tab, laa);
	branch_add_child(tab, lbb);
	g_assert(tree_num_leaves(tab) == 2);
	g_assert(tree_num_intern(tab) == 1);

	g_assert(labelset_count(tree_get_merge_left(tab)) == 1);
	g_assert(labelset_count(tree_get_merge_right(tab)) == 1);
	g_assert(labelset_count(tree_get_merge_left(laa)) == 1);
	g_assert(labelset_count(tree_get_merge_right(laa)) == 0);

	correct_tab =
		log_add_exp(gsl_sf_log(0.4) + gsl_sf_lnbeta(1.0+1, 0.2+0) - gsl_sf_lnbeta(1.0, 0.2)
			   ,gsl_sf_log(1.0 - 0.4) + gsl_sf_lnbeta(0.2+1, 1.0+0) - gsl_sf_lnbeta(0.2, 1.0)
			   );
	assert_eqfloat(tree_get_logprob(tab), correct_tab, prec);

	tac = branch_new(params);
	branch_add_child(tac, laa);
	branch_add_child(tac, lcc);

	correct_tac =
		log_add_exp(gsl_sf_log(0.4) + gsl_sf_lnbeta(1.0+0, 0.2+1) - gsl_sf_lnbeta(1.0, 0.2)
			   ,gsl_sf_log(1.0 - 0.4) + gsl_sf_lnbeta(0.2+0, 1.0+1) - gsl_sf_lnbeta(0.2, 1.0)
			   );
	assert_eqfloat(tree_get_logprob(tac), correct_tac, prec);

	tbc = branch_new(params);
	branch_add_child(tbc, lbb);
	branch_add_child(tbc, lcc);
	assert_eqfloat(tree_get_logprob(tac), correct_tac, prec);

	/* 3-trees */
	tabc = branch_new(params);
	branch_add_child(tabc, tab);
	branch_add_child(tabc, lcc);
	g_assert(tree_num_intern(tabc) == 2);
	g_assert(tree_num_leaves(tabc) == 3);

	correct_tabc =
		log_add_exp(gsl_sf_log(0.4) + gsl_sf_lnbeta(1.0+1, 0.2+2) - gsl_sf_lnbeta(1.0, 0.2)
			   ,gsl_sf_log(1.0 - 0.4) + correct_tab + 0.0 + gsl_sf_lnbeta(0.2, 1.0+2) - gsl_sf_lnbeta(0.2, 1.0)
			   );
	assert_eqfloat(tree_get_logprob(tabc), correct_tabc, prec);

	/* params do not propagate */
	new_params = params_new(params->dataset, 0.2, 0.3, 0.1, 0.5, 0.6);
	tree_set_params(tabc, new_params, FALSE);
	correct_tabc =
		log_add_exp(gsl_sf_log(0.2) + gsl_sf_lnbeta(0.3+1, 0.1+2) - gsl_sf_lnbeta(0.3, 0.1)
			   ,gsl_sf_log(1.0 - 0.2) + correct_tab + 0.0 + gsl_sf_lnbeta(0.5, 0.6+2) - gsl_sf_lnbeta(0.5, 0.6)
			   );
	assert_eqfloat(tree_get_logprob(tabc), correct_tabc, prec);
	/* params DO propagate */
	tree_set_params(tabc, new_params, TRUE);
	correct_tab =
		log_add_exp(gsl_sf_log(0.2) + gsl_sf_lnbeta(0.3+1, 0.1+0) - gsl_sf_lnbeta(0.3, 0.1)
			   ,gsl_sf_log(1.0 - 0.2) + gsl_sf_lnbeta(0.5+1, 0.6+0) - gsl_sf_lnbeta(0.5, 0.6)
			   );
	correct_tabc =
		log_add_exp(gsl_sf_log(0.2) + gsl_sf_lnbeta(0.3+1, 0.1+2) - gsl_sf_lnbeta(0.3, 0.1)
			   ,gsl_sf_log(1.0 - 0.2) + correct_tab + 0.0 + gsl_sf_lnbeta(0.5, 0.6+2) - gsl_sf_lnbeta(0.5, 0.6)
			   );
	assert_eqfloat(tree_get_logprob(tabc), correct_tabc, prec);

	params_unref(new_params);
	tree_unref(tabc);

	tabc = branch_new(params);
	branch_add_child(tabc, laa);
	branch_add_child(tabc, lcc);
	branch_add_child(tabc, lbb);

	correct_tabc =
		log_add_exp(gsl_sf_log(1.-(1.-0.4)*(1.-0.4)) + gsl_sf_lnbeta(1.0+1, 0.2+2) - gsl_sf_lnbeta(1.0, 0.2)
			   ,2*gsl_sf_log(1.0 - 0.4) + gsl_sf_lnbeta(0.2+1, 1.0+2) - gsl_sf_lnbeta(0.2, 1.0)
			   );
	assert_eqfloat(tree_get_logprob(tabc), correct_tabc, prec);

	new_params = params_new(params->dataset, 0.2, 0.3, 0.1, 0.5, 0.6);

	tree_set_params(tabc, new_params, FALSE);
	correct_tabc =
		log_add_exp(gsl_sf_log(1.-(1.-0.2)*(1.-0.2)) + gsl_sf_lnbeta(0.3+1, 0.1+2) - gsl_sf_lnbeta(0.3, 0.1)
			   ,2*gsl_sf_log(1.0 - 0.2) + gsl_sf_lnbeta(0.5+1, 0.6+2) - gsl_sf_lnbeta(0.5, 0.6)
			   );
	assert_eqfloat(tree_get_logprob(tabc), correct_tabc, prec);

	params_unref(new_params);

	/* deallocate */
	tree_unref(laa);
	tree_unref(lbb);
	tree_unref(lcc);
	tree_unref(tab);
	tree_unref(tac);
	tree_unref(tbc);
	tree_unref(tabc);
	params_unref(params);
}


void test_tree_logprob4(void) {
	Params * params;
	Merge * merge;
	GRand *rng;
	Tree *laa, *lbb, *lcc, *ldd;
	Tree *tab, *tac, *tcd, *tbalance;
	Tree *tflat, *tcascade, *tcascade_intern;
	gdouble correct_laa, correct_lcc, correct_tab, correct_tac, correct_tcd;
	gdouble correct_tbalance, correct_tbalance_children;
	gdouble correct_tflat, correct_tcascade, correct_tcascade_intern;
	gdouble prec;

	prec = 1e-4;
	rng = g_rand_new();
	init_test_toy4(&laa, &lbb, &lcc, &ldd);

	g_assert(tree_get_params(laa) == tree_get_params(lbb));
	g_assert(tree_get_params(lcc) == tree_get_params(lbb));
	g_assert(tree_get_params(ldd) == tree_get_params(lbb));
	params = tree_get_params(laa);
	params_ref(params);

	/* leaves */
	correct_laa = gsl_sf_lnbeta(1.0+1, 0.2) - gsl_sf_lnbeta(1.0, 0.2);
	correct_lcc = gsl_sf_lnbeta(1.0, 0.2+1) - gsl_sf_lnbeta(1.0, 0.2);
	assert_eqfloat(tree_get_logprob(laa), correct_laa, prec);
	assert_eqfloat(tree_get_logprob(lbb), 0.0, prec);
	assert_eqfloat(tree_get_logprob(lcc), correct_lcc, prec);
	assert_eqfloat(tree_get_logprob(ldd), 0.0, prec);

	/* pair */
	tab = branch_new_full(params, laa, lbb);
	correct_tab =
		log_add_exp(gsl_sf_log(0.4) + gsl_sf_lnbeta(1.0+1, 0.2+2) - gsl_sf_lnbeta(1.0, 0.2)
			   ,gsl_sf_log(1.0 - 0.4) + gsl_sf_lnbeta(0.2, 1.0+2) - gsl_sf_lnbeta(0.2, 1.0)
				  + correct_laa
			   );
	assert_eqfloat(tree_get_logprob(tab), correct_tab, prec);

	tac = branch_new_full(params, laa, lcc);
	correct_tac =
		log_add_exp(gsl_sf_log(0.4) + gsl_sf_lnbeta(1.0+2, 0.2+2) - gsl_sf_lnbeta(1.0, 0.2)
			   ,gsl_sf_log(1.0 - 0.4) + gsl_sf_lnbeta(0.2+1, 1.0+1) - gsl_sf_lnbeta(0.2, 1.0)
				  + correct_laa + correct_lcc
			   );
	assert_eqfloat(tree_get_logprob(tac), correct_tac, prec);

	tcd = branch_new_full(params, lcc, ldd);
	correct_tcd =
		log_add_exp(gsl_sf_log(0.4) + gsl_sf_lnbeta(1.0+1, 0.2+2) - gsl_sf_lnbeta(1.0, 0.2)
			   ,gsl_sf_log(1.0 - 0.4) + gsl_sf_lnbeta(0.2+1, 1.0+1) - gsl_sf_lnbeta(0.2, 1.0)
				 + correct_lcc + 0.0
			   );
	assert_eqfloat(tree_get_logprob(tcd), correct_tcd, prec);

	/* 3-trees: useful stepping stones for 4-trees */

	/* NOTE: these affect the sscache */
	tree_unref(branch_new_full(params, lbb, lcc));
	tree_unref(branch_new_full(params, lbb, ldd));
	tree_unref(branch_new_full(params, laa, ldd));
	tree_unref(branch_new_full(params, tab, lcc));
	tree_unref(branch_new_full(params, tab, ldd));

	/* 4-balance */
	correct_tbalance_children = gsl_sf_lnbeta(0.2+4, 1.0+3) - gsl_sf_lnbeta(0.2, 1.0) + correct_tab + correct_tcd;
	correct_tbalance =
		log_add_exp(gsl_sf_log(0.4) + gsl_sf_lnbeta(1.0+6, 0.2+7) - gsl_sf_lnbeta(1.0, 0.2)
			   ,gsl_sf_log(1.0 - 0.4) + correct_tbalance_children
			   );
	tbalance = branch_new_full(params, tab, tcd);
	g_assert(tree_num_leaves(tbalance) == 4);
	g_assert(tree_num_intern(tbalance) == 3);
	assert_eqfloat(tree_get_logprob(tbalance), correct_tbalance, prec);

	/* 4-flat */
	correct_tflat =
		log_add_exp(gsl_sf_log(1.0 - gsl_sf_pow_int(1-0.4, 3)) + gsl_sf_lnbeta(1.0+6, 0.2+7) - gsl_sf_lnbeta(1.0, 0.2)
			   ,3*gsl_sf_log(1.0 - 0.4) + gsl_sf_lnbeta(0.2+5, 1.0+6) - gsl_sf_lnbeta(0.2, 1.0)
				+ gsl_sf_lnbeta(1.0+1, 0.2) - gsl_sf_lnbeta(1.0, 0.2)
				+ gsl_sf_lnbeta(1.0, 0.2+1) - gsl_sf_lnbeta(1.0, 0.2)
			   );
	tflat = branch_new_full(params, laa, lbb, lcc, ldd);
	g_assert(tree_num_leaves(tflat) == 4);
	g_assert(tree_num_intern(tflat) == 1);
	assert_eqfloat(tree_get_logprob(tflat), correct_tflat, prec);

	/*
	merge = merge_collapse(rng, params, 0, tcd, 1, tab);
	assert_eqfloat(tree_get_logprob(merge->tree), correct_tflat, prec);
	merge_free(merge);
	*/

	/* 4-cascade */
	correct_tcascade_intern =
		log_add_exp(
			  gsl_sf_log(0.4) + gsl_sf_lnbeta(1.0+4, 0.2+4) - gsl_sf_lnbeta(1.0, 0.2)
			, gsl_sf_log(1-0.4) + gsl_sf_lnbeta(0.2+3, 1.0+1) - gsl_sf_lnbeta(0.2, 1.0)
				+ gsl_sf_lnbeta(1.0, 0.2+1) - gsl_sf_lnbeta(1.0, 0.2)
				+ correct_tab);
	correct_tcascade =
		log_add_exp(gsl_sf_log(0.4) + gsl_sf_lnbeta(1.0+6, 0.2+7) - gsl_sf_lnbeta(1.0, 0.2)
			, gsl_sf_log(1-0.4) + gsl_sf_lnbeta(0.2+2, 1.0+3) - gsl_sf_lnbeta(0.2, 1.0)
				+ correct_tcascade_intern);
	tcascade_intern = branch_new_full(params, tab, lcc);
	g_assert(tree_num_leaves(tcascade_intern) == 3);
	g_assert(tree_num_intern(tcascade_intern) == 2);
	assert_eqfloat(tree_get_logprob(tcascade_intern), correct_tcascade_intern, prec);
	tcascade = branch_new(params);
	branch_add_child(tcascade, tcascade_intern);
	branch_add_child(tcascade, ldd);
	g_assert(tree_num_leaves(tcascade) == 4);
	g_assert(tree_num_intern(tcascade) == 3);
	assert_eqfloat(tree_get_logprob(tcascade), correct_tcascade, prec);

	merge = merge_join(rng, NULL, params, 0, tcascade_intern, 1, ldd);
	assert_eqfloat(tree_get_logprob(merge->tree), correct_tcascade, prec);
	merge_free(merge);

	/* deallocate */
	tree_unref(laa);
	tree_unref(lbb);
	tree_unref(lcc);
	tree_unref(ldd);
	tree_unref(tab);
	tree_unref(tac);
	tree_unref(tcd);
	tree_unref(tbalance);
	tree_unref(tflat);
	tree_unref(tcascade);
	tree_unref(tcascade_intern);
	params_unref(params);
	g_rand_free(rng);
}


gdouble test_build_logpred4_sparse(gboolean sparse) {
	Tree * root;
	Dataset * dataset;
	Params * params;
	GRand * rng;
	gdouble logpred_true, logpred_false;
	gpointer bb, dd;
	Build * build;
	gdouble total;
	guint count;
	DatasetPairIter pairs;
	gpointer src, dst;

	rng = g_rand_new_with_seed(2);
	dataset = dataset_gen_toy4(sparse);
	params = params_default(dataset);
	build = build_new(rng, params, 10, FALSE);
	build_run(build);
	root = build_get_best_tree(build);
	tree_ref(root);
	build_free(build);

	total = 0;
	count = 0;
	/* test on the training points */
#define	pred_ok(src, dst) \
	do {									\
		logpred_true = tree_logpredict(root, src, dst, TRUE);		\
		logpred_false = tree_logpredict(root, src, dst, FALSE);		\
		assert_lefloat(logpred_true, 0.0, EQFLOAT_DEFAULT_PREC);	\
		assert_lefloat(logpred_false, 0.0, EQFLOAT_DEFAULT_PREC);	\
		assert_eqfloat(log_add_exp(logpred_true, logpred_false), 0.0,	\
				EQFLOAT_DEFAULT_PREC);				\
	} while (0)

	/*dataset_println(dataset, sparse? "sparse": "dense");*/
	dataset_label_pairs_iter_init(dataset, &pairs);
	while (dataset_label_pairs_iter_next(&pairs, &src, &dst)) {
		/*
		g_print("%s %s\n",
				dataset_label_to_string(dataset, src),
				dataset_label_to_string(dataset, dst));
		*/
		pred_ok(src, dst);
		total += logpred_true;
		count++;
	}
	g_assert_cmpint(count, ==, 16-3);
	bb = dataset_label_lookup(dataset, "bb");
	dd = dataset_label_lookup(dataset, "dd");
	pred_ok(bb, bb);
	pred_ok(bb, dd);
	pred_ok(dd, dd);
#undef pred_ok
	dataset_unref(dataset);
	params_unref(params);
	tree_unref(root);
	g_rand_free(rng);
	return total;
}

void test_build_logpred4(void) {
	gdouble total_dense;
	gdouble total_sparse;

	total_dense = test_build_logpred4_sparse(FALSE);
	total_sparse = test_build_logpred4_sparse(TRUE);
	assert_eqfloat(total_dense, total_sparse, EQFLOAT_DEFAULT_PREC);
}

void test_merge_score3(void) {
	GRand * rng;
	Params * params;
	Tree *laa, *lbb, *lcc;
	Tree *tab, *tabc;
	Merge *merge_ab;
	Merge *merge_abc;
	gdouble prec;
	gdouble score_tabc, score_tab, correct_tab, correct_tabc;
	gpointer global_suffstats;

	prec = 1e-4;
	rng = g_rand_new();
	init_test_toy3(&laa, &lbb, &lcc);
	params = tree_get_params(laa);

	tab = branch_new(params);
	branch_add_child(tab, laa);
	branch_add_child(tab, lbb);
	merge_ab = merge_new(rng, NULL, params, 0, laa, 1, lbb, tab);
	global_suffstats = counts_new(1, 3);
	merge_notify_pair(merge_ab, global_suffstats);
	suffstats_unref(global_suffstats);

	correct_tab =
		log_add_exp(gsl_sf_log(0.4) + gsl_sf_lnbeta(1.0+1, 0.2+0) - gsl_sf_lnbeta(1.0, 0.2)
			   ,gsl_sf_log(1.0 - 0.4) + gsl_sf_lnbeta(0.2+1, 1.0+0) - gsl_sf_lnbeta(0.2, 1.0)
			   );
	assert_eqfloat(tree_get_logprob(tab), correct_tab, prec);
	assert_eqfloat(tree_get_logprob(lcc), 0.0, prec);

	score_tab = correct_tab + gsl_sf_lnbeta(0.2, 1.0+2) - gsl_sf_lnbeta(0.2+1, 1.0+2);
	assert_eqfloat(merge_ab->score, score_tab, prec);

	/* need to update cache with these merges first */
	tree_unref(branch_new_full(params, laa, lcc));
	tree_unref(branch_new_full(params, lbb, lcc));

	tabc = branch_new(params);
	branch_add_child(tabc, tab);
	branch_add_child(tabc, lcc);

	merge_abc = merge_new(rng, merge_ab, params, 0, tab, 1, lcc, tabc);

	correct_tabc =
		log_add_exp(gsl_sf_log(0.4) + gsl_sf_lnbeta(1.0+1, 0.2+2) - gsl_sf_lnbeta(1.0, 0.2)
			   ,gsl_sf_log(1.0 - 0.4) + correct_tab + 0.0 + gsl_sf_lnbeta(0.2, 1.0+2) - gsl_sf_lnbeta(0.2, 1.0)
			   );
	assert_eqfloat(tree_get_logprob(tabc), correct_tabc, prec);
	assert_eqfloat(merge_abc->tree_score, correct_tabc - correct_tab - 0.0, prec);
	//score_tabc = correct_tabc - (correct_tab + 0.0 + gsl_sf_lnbeta(0.2, 1.0+2) - gsl_sf_lnbeta(0.2, 1.0));
	score_tabc = correct_tabc - correct_tab - gsl_sf_lnbeta(0.2, 1.0+2) + gsl_sf_lnbeta(0.2, 1.0);
	assert_eqfloat(merge_abc->score, score_tabc, prec);


	merge_free(merge_abc);
	merge_free(merge_ab);
	tree_unref(tab);
	tree_unref(tabc);
	tree_unref(laa);
	tree_unref(lbb);
	tree_unref(lcc);
	g_rand_free(rng);
}


void test_sscache_stats(void) {
	Tree *laa;
	Tree *lbb;
	Tree *lcc;
	Tree *ldd;
	gconstpointer aa;
	gconstpointer bb;
	gconstpointer cc;
	gconstpointer dd;
	SSCache *cache;
	Dataset * dataset;
	Counts * counts;
	Labelset * src_left;
	Labelset * src_right;
	Labelset * dst_left;
	Labelset * dst_right;

	init_test_toy4(&laa, &lbb, &lcc, &ldd);
	cache = sscache_new(tree_get_params(laa)->dataset, FALSE);
	dataset = tree_get_params(laa)->dataset;

	counts = sscache_get_label(cache, aa = leaf_get_label(laa));
	g_assert(counts->num_ones == 1);
	g_assert(counts->num_total == 1);

	counts = sscache_get_label(cache, bb = leaf_get_label(lbb));
	g_assert(counts->num_ones == 0);
	g_assert(counts->num_total == 0);

	counts = sscache_get_label(cache, cc = leaf_get_label(lcc));
	g_assert(counts->num_ones == 0);
	g_assert(counts->num_total == 1);

	counts = sscache_get_label(cache, dd = leaf_get_label(ldd));
	g_assert(counts->num_ones == 0);
	g_assert(counts->num_total == 0);

	counts = sscache_get_offblock_full(cache, aa, bb);
	g_assert(counts != NULL);
	g_assert(counts->num_ones == 0);
	g_assert(counts->num_total == 2);

	counts = sscache_get_offblock_full(cache, aa, cc);
	g_assert(counts != NULL);
	g_assert(counts->num_ones == 1);
	g_assert(counts->num_total == 2);

	counts = sscache_get_offblock_full(cache, aa, dd);
	g_assert(counts != NULL);
	g_assert(counts->num_ones == 1);
	g_assert(counts->num_total == 2);

	counts = sscache_get_offblock_full(cache, cc, bb);
	g_assert(counts != NULL);
	g_assert(counts->num_ones == 2);
	g_assert(counts->num_total == 2);

	counts = sscache_get_offblock_full(cache, dd, bb);
	g_assert(counts != NULL);
	g_assert(counts->num_ones == 0);
	g_assert(counts->num_total == 1);

	counts = sscache_get_offblock_full(cache, dd, cc);
	g_assert(counts != NULL);
	g_assert(counts->num_ones == 1);
	g_assert(counts->num_total == 2);


	/* act as if {a,b} is a node */
	src_left = labelset_new(dataset, aa);
	src_right = labelset_new(dataset, bb);
	dst_left = labelset_new(dataset, cc);
	dst_right = labelset_new(dataset);
	counts = sscache_get_offblock(cache, src_left, src_right, dst_left, dst_right);
	g_assert(counts != NULL);
	g_assert(counts->num_ones == 3);
	g_assert(counts->num_total == 4);
	labelset_unref(src_left);
	labelset_unref(src_right);
	labelset_unref(dst_left);
	labelset_unref(dst_right);

	src_left = labelset_new(dataset, aa);
	src_right = labelset_new(dataset, bb);
	dst_left = labelset_new(dataset);
	dst_right = labelset_new(dataset, dd);
	counts = sscache_get_offblock(cache, src_left, src_right, dst_left, dst_right);
	g_assert(counts != NULL);
	g_assert(counts->num_ones == 1);
	g_assert(counts->num_total == 3);
	labelset_unref(src_left);
	labelset_unref(src_right);
	labelset_unref(dst_left);
	labelset_unref(dst_right);

	/* then suppose {c,d} is a node */

	/* now test */
	src_left = labelset_new(dataset, aa);
	src_right = labelset_new(dataset, bb);
	dst_left = labelset_new(dataset, cc);
	dst_right = labelset_new(dataset, dd);
	counts = sscache_get_offblock(cache, src_left, src_right, dst_left, dst_right);
	g_assert(counts != NULL);
	g_assert(counts->num_ones == 4);
	g_assert(counts->num_total == 7);
	labelset_unref(src_left);
	labelset_unref(src_right);
	labelset_unref(dst_left);
	labelset_unref(dst_right);

	tree_unref(laa);
	tree_unref(lbb);
	tree_unref(lcc);
	tree_unref(ldd);
	sscache_unref(cache);
}


void test_bitset(void) {
	Bitset * aa;
	Bitset * bb;
	Bitset * cc;
	const guint size = 1300;
	guint ii;

	aa = bitset_new(size);
	for (ii = 0; ii < size; ii++) {
		g_assert(!bitset_contains(aa, ii));
		g_assert(bitset_count(aa) == 0);
		bitset_set(aa, ii);
		g_assert(bitset_contains(aa, ii));
		g_assert(bitset_count(aa) == 1);
		bitset_clear(aa, ii);
		g_assert(!bitset_contains(aa, ii));
		g_assert(bitset_count(aa) == 0);
	}
	for (ii = 0; ii < size; ii++) {
		g_assert(!bitset_contains(aa, ii));
		g_assert(bitset_count(aa) == ii);
		bitset_set(aa, ii);
		g_assert(bitset_contains(aa, ii));
		g_assert(bitset_count(aa) == ii+1);
	}
	for (ii = 0; ii < size; ii++) {
		bitset_clear(aa, ii);
	}
	bitset_set(aa, 23);
	g_assert(bitset_contains(aa, 23));
	for (ii = 0; ii < size; ii++) {
		g_assert(bitset_contains(aa, ii) == (ii == 23));
	}
	bitset_clear(aa, 23);
	bitset_set(aa, 123);
	g_assert(bitset_contains(aa, 123));
	for (ii = 0; ii < size; ii++) {
		g_assert(bitset_contains(aa, ii) == (ii == 123));
	}
	bitset_ref(aa);
	bitset_unref(aa);

	bb = bitset_new(size);
	g_assert(bitset_disjoint(aa, bb));

	bitset_union(bb, aa);
	g_assert(!bitset_disjoint(aa, bb));
	g_assert(bitset_equal(aa, bb));
	g_assert(bitset_hash(aa) == bitset_hash(bb));

	cc = bitset_new(size);
	g_assert(!bitset_equal(aa, cc));
	g_assert(bitset_hash(cc) != bitset_hash(aa));

	bitset_clear(aa, 123);
	g_assert(!bitset_contains(aa, 123));
	g_assert(bitset_equal(aa, cc));
	g_assert(!bitset_equal(aa, bb));

	for (ii = 0; ii < size; ii += 3) {
		bitset_set(aa, ii);
	}
	for (ii = 0; ii < size; ii++) {
		g_assert(bitset_contains(aa, ii) == ((ii % 3) == 0));
	}
	for (ii = 0; ii < size; ii += 6) {
		bitset_set(bb, ii);
	}
	for (ii = 0; ii < size; ii++) {
		g_assert(bitset_contains(bb, ii) == ((ii % 6) == 0) || ii == 123);
	}
	g_assert(!bitset_disjoint(aa, bb));

	bitset_foreach(aa, (BitsetFunc)bitset_set, cc);
	g_assert(bitset_equal(aa, cc));

	bitset_set(bb, 29);
	for (ii = 0; ii < size; ii += 6) {
		bitset_clear(bb, ii);
	}
	g_assert(bitset_count(bb) == 2);

	{
		GString *out = g_string_new("");
		bitset_tostring(bb, out);
		g_assert_cmpstr(out->str, ==, "29 123 ");
		g_string_free(out, TRUE);
	}

	bitset_unref(aa);
	bitset_unref(bb);
	bitset_unref(cc);
}

void test_labelset(void) {
	Tree *laa, *lbb, *lcc;
	gconstpointer aa, bb, cc;
	Labelset *seta, *setb, *setc;
	Dataset * dataset;

	init_test_toy3(&laa, &lbb, &lcc);
	aa = leaf_get_label(laa);
	bb = leaf_get_label(lbb);
	cc = leaf_get_label(lcc);

	dataset = tree_get_params(laa)->dataset;
	seta = labelset_new(dataset);
	setb = labelset_new(dataset);
	setc = labelset_new(dataset);

	g_assert(labelset_equal(seta, setb));
	g_assert(labelset_equal(seta, seta));

	for (guint ii = 0; ii < 0x80 - 4; ii++) {
		gchar *str = num_to_string(ii);
		gpointer label = dataset_label_create(dataset, str);
		g_assert(!labelset_contains(seta, label));
		g_assert(labelset_count(seta) == ii);
		labelset_add(seta, label);
		g_assert(labelset_contains(seta, label));
		g_assert(labelset_count(seta) == ii+1);
		g_free(str);
	}
	for (guint ii = 0; ii < 0x80 - 4; ii++) {
		gchar *str = num_to_string(ii);
		gpointer label = dataset_label_create(dataset, str);
		g_free(str);
		labelset_del(seta, label);
	}

	labelset_add(seta, aa);
	labelset_add(setb, bb);
	labelset_add(setc, cc);

	g_assert(labelset_count(seta) == 1);
	g_assert(!labelset_equal(seta, setb));
	g_assert(!labelset_equal(setc, setb));

	labelset_union(seta, setb);
	g_assert(labelset_count(seta) == 2);
	g_assert(!labelset_equal(seta, setb));
	labelset_union(setb, seta);
	g_assert(labelset_equal(seta, setb));

	// setaa = {aa, bb}
	labelset_del(seta, aa);
	g_assert(!labelset_equal(seta, setb));
	labelset_add(seta, aa);
	g_assert(labelset_equal(seta, setb));
	labelset_union(setc, seta);
	labelset_add(setb, cc);
	g_assert(labelset_equal(setc, setb));

	labelset_unref(seta);
	seta = labelset_copy(setc);
	labelset_del(seta, aa);

	{
		GString *out = g_string_new("");
		labelset_tostring(setc, out);
		g_assert_cmpstr(out->str, ==, "aa bb cc ");
		g_string_free(out, TRUE);
	}

	labelset_unref(seta);
	labelset_unref(setb);
	labelset_unref(setc);
	tree_unref(laa);
	tree_unref(lbb);
	tree_unref(lcc);
}


void test_bitset_popcount(void) {
	guint64 test[] = {
		0x5555555555555555ULL, 32,
		0x3333333333333333ULL, 32,
		0x123456789abcdef0ULL, 32,
		0x123456789abcdef0ULL, 32,
		0x501ad1e3dc77bf5eULL, 37,
		0x3b3a24293bd8, 23,
		0x19b12e6f, 17,
		0x2009a88, 7,
		0x7, 3,
		0x6, 2,
		0x5, 2,
		0x4, 1,
		0x3, 2,
		0x2, 1,
		0x1, 1,
		0x0, 0
	};
	for (guint ii = 0; ii < sizeof(test)/sizeof(test[0]); ii += 2) {
		g_assert(pop_count(test[ii]) == test[ii+1]);
	}
}


void test_log_add_exp(void) {
	/* same as naive for basic values */
	for (gdouble ii = 0.1; ii < 1.0; ii += 0.01) {
		gdouble log_ii = gsl_sf_log(ii);
		for (gdouble jj = 0.1; jj < 1.0; jj += 0.01) {
			gdouble log_jj = gsl_sf_log(jj);

			gdouble naive = gsl_sf_log(ii+jj);
			gdouble test = log_add_exp(log_ii, log_jj);
			assert_eqfloat(naive, test, EQFLOAT_DEFAULT_PREC);
		}
	}
	for (gdouble log_ii = 0.0; log_ii > -100.0; log_ii -= 0.5) {
		for (gdouble log_jj = 0.0; log_jj > -100.0; log_jj -= 0.5) {
			gdouble naive = gsl_sf_log(gsl_sf_exp(log_ii)+gsl_sf_exp(log_jj));
			gdouble test = log_add_exp(log_ii, log_jj);
			assert_eqfloat(naive, test, EQFLOAT_DEFAULT_PREC);
		}
	}
}


void test_lnbetacache(void) {
	gdouble hypers[] = { 0.01, 0.1, 0.5, 10.0 };
	const guint max_num = 51;

	for (guint h0 = 0; h0 < sizeof(hypers)/sizeof(hypers[0]); h0++) {
		gdouble alpha = hypers[h0];
		for (guint h1 = 0; h1 < sizeof(hypers)/sizeof(hypers[0]); h1++) {
			gdouble beta = hypers[h1];
			LnBetaCache * cache = lnbetacache_new(alpha, beta, max_num);
			for (guint n0 = 0; n0 < 2*max_num; n0++) {
				for (guint n1 = 0; n1 < 2*max_num; n1++) {
					gdouble truth = gsl_sf_lnbeta(alpha+n1, beta+n0);
					gdouble cc = lnbetacache_get(cache, n1, n0);
					assert_eqfloat(truth, cc, EQFLOAT_DEFAULT_PREC);
				}
			}
			g_assert(lnbetacache_get_num_hits(cache) == 0);
			for (guint n0 = 0; n0 < 2*max_num; n0++) {
				for (guint n1 = 0; n1 < 2*max_num; n1++) {
					gdouble truth = gsl_sf_lnbeta(alpha+n1, beta+n0);
					gdouble cc = lnbetacache_get(cache, n1, n0);
					assert_eqfloat(truth, cc, EQFLOAT_DEFAULT_PREC);
				}
			}
			/* note that only values lying < max_num will be cached
			 */
			g_assert(lnbetacache_get_num_hits(cache) == max_num*max_num);
			lnbeta_cache_free(cache);
		}
	}
}

gint intp_cmp(gconstpointer aa, gconstpointer bb) {
	return GPOINTER_TO_INT(aa) - GPOINTER_TO_INT(bb);
}

gpointer intp(gint aa) {
	return GINT_TO_POINTER(aa);
}

void test_minheap(void) {
	MinHeap * heap;

	heap = minheap_new(0, intp_cmp, minheap_elem_no_free);
	g_assert(minheap_size(heap) == 0);

	minheap_enq(heap, intp(0xfeed));
	g_assert(minheap_size(heap) == 1);
	g_assert(minheap_deq(heap) == intp(0xfeed));
	g_assert(minheap_size(heap) == 0);

	minheap_enq(heap, intp(14));
	minheap_enq(heap, intp(13));
	minheap_enq(heap, intp(12));
	g_assert(minheap_size(heap) == 3);
	g_assert(minheap_deq(heap) == intp(12));
	g_assert(minheap_deq(heap) == intp(13));
	g_assert(minheap_deq(heap) == intp(14));
	g_assert(minheap_size(heap) == 0);

	minheap_enq(heap, intp(5));
	minheap_enq(heap, intp(4));
	minheap_enq(heap, intp(3));
	minheap_enq(heap, intp(2));
	minheap_enq(heap, intp(1));
	g_assert(minheap_size(heap) == 5);
	g_assert(minheap_deq(heap) == intp(1));
	g_assert(minheap_deq(heap) == intp(2));
	g_assert(minheap_deq(heap) == intp(3));
	g_assert(minheap_deq(heap) == intp(4));
	g_assert(minheap_deq(heap) == intp(5));
	g_assert(minheap_size(heap) == 0);

	minheap_enq(heap, intp(3));
	minheap_enq(heap, intp(5));
	minheap_enq(heap, intp(1));
	g_assert(minheap_size(heap) == 3);
	g_assert(minheap_deq(heap) == intp(1));
	g_assert(minheap_deq(heap) == intp(3));
	minheap_enq(heap, intp(2));
	minheap_enq(heap, intp(4));
	g_assert(minheap_size(heap) == 3);
	{
		gboolean seen2, seen4, seen5;
		MinHeapIter iter;
		gpointer elem;
		guint num_seen;

		seen2 = seen4 = seen5 = FALSE;
		num_seen = 0;
		for (minheap_iter_init(heap, &iter); minheap_iter_next(&iter, &elem);) {
			seen2 |= (elem == intp(2));
			seen4 |= (elem == intp(4));
			seen5 |= (elem == intp(5));
			num_seen++;
		}
		g_assert(seen2 && seen4 && seen5);
		g_assert(num_seen == 3);
	}
	g_assert(minheap_deq(heap) == intp(2));
	g_assert(minheap_deq(heap) == intp(4));
	g_assert(minheap_deq(heap) == intp(5));
	g_assert(minheap_size(heap) == 0);
	minheap_free(heap);

	heap = minheap_new(0, intp_cmp, minheap_elem_no_free);
	{
		GRand *rng;
		gpointer elem;
		guint ii;
		gint prev, cur;

		rng = g_rand_new_with_seed(2);
		for (ii = 0; ii < 8; ii++) {
			cur = g_rand_int_range(rng, -1000,1000);
			minheap_enq(heap, intp(cur));
		}

		elem = minheap_deq(heap);
		prev = GPOINTER_TO_INT(elem);
		while (minheap_size(heap) > 0) {
			elem = minheap_deq(heap);
			cur = GPOINTER_TO_INT(elem);
			g_assert(cur >= prev);
			prev = cur;
		}
		g_rand_free(rng);
	}
	minheap_free(heap);
}

int main(int argc, char *argv[]) {
	g_test_init(&argc, &argv, NULL);
	g_test_add_func("/tree/logprob3", test_tree_logprob3);
	g_test_add_func("/tree/logprob4", test_tree_logprob4);
	g_test_add_func("/tree/logpred4", test_build_logpred4);
	g_test_add_func("/merge/score3", test_merge_score3);
	g_test_add_func("/bitset", test_bitset);
	g_test_add_func("/bitset/popcount", test_bitset_popcount);
	g_test_add_func("/labelset", test_labelset);
	g_test_add_func("/sscache/stats", test_sscache_stats);
	g_test_add_func("/util/log_add_exp", test_log_add_exp);
	g_test_add_func("/util/lnbetacache", test_lnbetacache);
	g_test_add_func("/util/minheap", test_minheap);
	g_test_run();
	return 0;
}
