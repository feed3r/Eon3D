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
    /* TODO */
}
END_TEST


/*************************************************************************/

TCase *eon3d_testCaseNone(void)
{
    TCase *tcNone = tcase_create("eon3d.core.none");
    tcase_add_test(tcNone, test_none);

    return tcNone;
}

static Suite *eon3d_suiteNone(void)
{
    TCase *tc = eon3d_testCaseNone();
    Suite *s = suite_create("eon3d.core.none");
    suite_add_tcase(s, tc);
    return s;
}

/*************************************************************************/

int main(int argc, char *argv[])
{
    int number_failed = 0;

    Suite *s = eon3d_suiteNone();
    SRunner *sr = srunner_create(s);

    srunner_run_all(sr, CK_ENV);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

/*************************************************************************/

/* vim: set ts=4 sw=4 et */
/* EOF */

