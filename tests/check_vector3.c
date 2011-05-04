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
#include "eon3d_private.h"

/*************************************************************************/

START_TEST(test_vector3Set)
{
    EON_Float X = 1.0, Y = -2.0, Z = 3.0;
    EON_Vector3 V;

    eon_Vector3Set(&V, X, Y, Z);

    fail_unless(eon_floatAreEquals(V.X, X), "X component mis-set");
    fail_unless(eon_floatAreEquals(V.Y, Y), "Y component mis-set");
    fail_unless(eon_floatAreEquals(V.Z, Z), "Z component mis-set");
}
END_TEST

START_TEST(test_vector3SetFromVector)
{
    EON_Float X = 1.0, Y = -2.0, Z = 3.0;
    EON_Vector3 V, W;

    eon_Vector3Set(&V, X, Y, Z);
    eon_Vector3Set(&W, V.X, V.Y, V.Z);

    fail_unless(eon_floatAreEquals(V.X, W.X), "X component mis-set");
    fail_unless(eon_floatAreEquals(V.Y, W.Y), "Y component mis-set");
    fail_unless(eon_floatAreEquals(V.Z, W.Z), "Z component mis-set");
}
END_TEST

START_TEST(test_vector3AreEqualsComp)
{
    EON_Float X = 1.0, Y = -2.0, Z = 3.0;
    EON_Vector3 V, W;

    eon_Vector3Set(&V, X, Y, Z);
    eon_Vector3Set(&W, X, Y, Z);

    fail_unless(eon_floatAreEquals(V.X, W.X), "X component mis-set");
    fail_unless(eon_floatAreEquals(V.Y, W.Y), "Y component mis-set");
    fail_unless(eon_floatAreEquals(V.Z, W.Z), "Z component mis-set");
}
END_TEST

START_TEST(test_vector3AreEquals)
{
    EON_Float X = 1.0, Y = -2.0, Z = 3.0;
    EON_Vector3 V, W;

    eon_Vector3Set(&V, X, Y, Z);
    eon_Vector3Set(&W, X, Y, Z);

    fail_unless(eon_Vector3IsEqual(&V, &W), "vectors are different");
}
END_TEST


START_TEST(test_vector3AddVSRAdd)
{
    EON_Float X = 1.0, Y = -2.0, Z = 3.0;
    EON_Vector3 V, W, Q;

    eon_Vector3Set(&V, X, Y, Z);
    eon_Vector3Set(&W, X, Y, Z);

    eon_Vector3Add(&V, &W, &Q);
    eon_Vector3RAdd(&V, &W);

    fail_unless(eon_Vector3IsEqual(&V, &Q), "vectors are different");
}
END_TEST


/*************************************************************************/

TCase *eon3d_testCaseVector3(void)
{
    TCase *tcVector3 = tcase_create("eon3d.core.vector3");
    tcase_add_test(tcVector3, test_vector3Set);
    tcase_add_test(tcVector3, test_vector3SetFromVector);
    tcase_add_test(tcVector3, test_vector3AreEquals);
    tcase_add_test(tcVector3, test_vector3AreEqualsComp);
    tcase_add_test(tcVector3, test_vector3AddVSRAdd);

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

