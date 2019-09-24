#include <glib.h>
#if _WIN32
#include "winpatch.h"
#endif
#if _WIN32
static void
test_winpatch(void)
{
    guint64 mask_value = 4294967296;
    gint a = g_bit_nth_lsf_impl_win(mask_value, -1);
    g_assert(a == 32);
}
#endif
int
main(int   argc,
    char *argv[])
{
    g_test_init(&argc, &argv, NULL);
#if _WIN32
    g_test_add_func("/testutil/winpatch", test_winpatch);
#endif
    return g_test_run();
}