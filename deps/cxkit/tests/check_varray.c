/*
 * check_varray.c - unit tests for varray.
 * (C) 2010 Francesco Romani - fromani at gmail dot com. ZLIB licensed.
 */

#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>

#include <check.h>

#include "varray.h"


/*************************************************************************/

START_TEST(test_core_init_null)
{
    int ret;
    VArray *va = varray_new(4, 0);
    
    fail_if(va == NULL, "varray_new(4, 0) failed");

    ret = varray_del(va);
    
    fail_if(ret != VARRAY_OK, "varray_del() failed");
}
END_TEST


/*************************************************************************/

static Suite *varray_suite(void)
{
    Suite *s = suite_create("varray");

    TCase *tc_core = tcase_create("core");
    tcase_add_test(tc_core, test_core_init_null);

    suite_add_tcase(s, tc_core);

    return s;
}

/*************************************************************************/

int main(int argc, char *argv[])
{
    int number_failed = 0;

    Suite *s = varray_suite();
    SRunner *sr = srunner_create(s);

    srunner_run_all(sr, CK_ENV);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

/*************************************************************************/

/* vim: et sw=4 ts=4: */

