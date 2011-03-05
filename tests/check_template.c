/**************************************************************************
 * check_template: template of an eon3d unit test                         *
 **************************************************************************/
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include <check.h>

#include "config.h"

#include "eon3d.h" 


/*************************************************************************/

START_TEST(test_none)
{
    int ret = 0;
}
END_TEST


/*************************************************************************/

static Suite *eon3d_suite_none(void)
{
    Suite *s = suite_create("eon3d_core_none");

    TCase *tc_none = tcase_create("none");
    tcase_add_test(tc_none, test_none);

    suite_add_tcase(s, tc_none);

    return s;
}

/*************************************************************************/

int main(int argc, char *argv[])
{
    int number_failed = 0;

    Suite *s = eon3d_suite_none();
    SRunner *sr = srunner_create(s);

    srunner_run_all(sr, CK_ENV);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

/*************************************************************************/

/* vim: set ts=4 sw=4 et */
/* EOF */

