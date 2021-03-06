#ifndef	SSCACHE_H
#define	SSCACHE_H

#include <glib.h>
#include "dataset.h"
#include "labelset.h"

struct SSCache_t;
typedef struct SSCache_t SSCache;

SSCache * sscache_new(Dataset *, gboolean);
gpointer sscache_get_label(SSCache *cache, gconstpointer label);
gpointer sscache_get_offblock(SSCache *cache, Labelset * xx_left, Labelset * xx_right, Labelset * yy_left, Labelset * yy_right);
gpointer sscache_get_offblock_full(SSCache *cache, gconstpointer ii, gconstpointer jj);
void sscache_println(SSCache * cache, const gchar * prefix);
void sscache_unref(SSCache *cache);

gpointer suffstats_new_empty(void);
gpointer suffstats_copy(gpointer src);
void suffstats_ref(gpointer ss);
void suffstats_unref(gpointer ss);
void suffstats_add(gpointer pdst, gpointer psrc);
void suffstats_sub(gpointer pdst, gpointer psrc);
void suffstats_print(gpointer ss);

#endif /*SSCACHE_H*/
