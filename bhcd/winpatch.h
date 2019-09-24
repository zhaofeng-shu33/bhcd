#include <glib.h>
static inline gint
g_bit_nth_lsf_impl_win(guint64 mask,
    gint   nth_bit)
{
    if (G_UNLIKELY(nth_bit < -1))
        nth_bit = -1;
    while (nth_bit < ((GLIB_SIZEOF_LONG * 8) - 1))
    {
        nth_bit++;
        if (mask & (1UL << nth_bit))
            return nth_bit;
    }
    return -1;
}