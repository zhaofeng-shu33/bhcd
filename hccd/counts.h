#ifndef COUNTS_H
#define COUNTS_H

#include <glib.h>

typedef struct {
	/* private: */
	guint		ref_count;
	/* public: */
	guint		num_ones;
	guint		num_total;
} Counts;

Counts * counts_new(guint num_ones, guint num_total);
Counts * counts_copy(Counts *);
void counts_add(Counts * dst, Counts * src);
void counts_sub(Counts * dst, Counts * src);
guint counts_num_zeros(Counts *);
guint counts_num_ones(Counts *);
void counts_ref(Counts *);
void counts_unref(Counts *);

#endif
