#ifndef PARAMS_H
#define PARAMS_H
#include <glib.h>
#include "dataset.h"
#include "sscache.h"
#include "lnbetacache.h"


typedef struct {
	/* private: */
	guint		ref_count;
	/* public: */
	Dataset *	dataset;
	SSCache *	sscache;
	gboolean	binary_only;
	gboolean	sparse;

	gdouble		gamma;
	gdouble		alpha;
	gdouble		beta;
	gdouble		delta;
	gdouble		lambda;
	/* private */
	/* cached: really log(1-gamma) */
	gdouble		loggamma;
	/* cached: logbeta(alpha, beta) */
	LnBetaCache *	logbeta_alpha_beta;
	/* cached: logbeta(delta, lambda) */
	LnBetaCache *	logbeta_delta_lambda;
} Params;

typedef gdouble (*ParamsProbFunc)(Params *, gpointer);

Params * params_new(Dataset * dataset, gdouble gamma, gdouble alpha, gdouble beta, gdouble delta, gdouble lambda);
Params * params_default(Dataset * dataset);
void params_reset_cache(Params *);
void params_set_sparse(Params *, gboolean);
void params_ref(Params * params);
void params_unref(Params * params);

gdouble params_logprob_off(Params *, gpointer);
gdouble params_logprob_offscore(Params *, gpointer);
gdouble params_logprob_on(Params *, gpointer);
gdouble params_logpred_off(Params *, gpointer, gboolean);
gdouble params_logpred_on(Params *, gpointer, gboolean);

Params * params_sample(GRand * rng, Params * params, ParamsProbFunc func, gpointer user_data);

#endif
