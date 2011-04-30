/**************************************************************************
 * check_vector3: vector3 function unit tests                             *
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

TCase *eon3d_testCaseVector3(void)
{
    TCase *tcVector3 = tcase_create("eon3d.core.vector3");
    tcase_add_test(tcVector3, test_none);

    return tcVector3;
}

static Suite *eon3d_suiteVector3(void)
{
    TCase *tc = eon3d_testCaseVector3();
    Suite *s = suite_create("eon3d.core.vector3");
    suite_add_tcase(s, tc);
    return s;
}

/*************************************************************************/

int main(int argc, char *argv[])
{
    int number_failed = 0;

    Suite *s = eon3d_suiteVector3();
    SRunner *sr = srunner_create(s);

    srunner_run_all(sr, CK_ENV);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

/*************************************************************************/

/* vim: set ts=4 sw=4 et */
/* EOF */

