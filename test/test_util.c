#include <glib.h>
#include "winpatch.h"

static void
test_winpatch(void)
{
    guint64 mask_value = 4294967296;
    gint a = g_bit_nth_lsf_impl_win(mask_value, -1);
    g_assert(a == 32);
}
int
main(int   argc,
    char *argv[])
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/testutil/winpatch", test_winpatch);

    return g_test_run();
}