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

/* yes, this has to be BEFORE the header... yet */
typedef struct eon_array_ eon_array;

#include "eon3d_private.h"


/*************************************************************************/

START_TEST(test_arrayNewZeroSizes)
{
    eon_array *array = eon_arrayNew(0, 0);
    fail_unless(array == NULL, "created with zero size");
    /* yes, there is a memleak if the test fails*/
}
END_TEST

START_TEST(test_arrayNewZeroItemSize)
{
    eon_array *array = eon_arrayNew(23, 0);
    fail_if(array == NULL, "NOT created with zero item size");
    eon_arrayFree(array);
    /* yes, there is a memleak if the test fails*/
}
END_TEST

START_TEST(test_arrayEmptyLength)
{
    int len = 0;
    eon_array *array = eon_arrayNew(23, 0);
    EON_Status ret = eon_arrayLength(array, &len);
    eon_arrayFree(array);

    fail_unless(ret == EON_OK, "failed while getting length");
    fail_unless(len == 0, "newly created array has non-zero length=%i", len);
}
END_TEST

START_TEST(test_arrayAppendOne)
{
    eon_array *array = eon_arrayNew(23, sizeof(int));
    EON_Status ret = EON_ERROR;
    int value = 42;

    ret = eon_arrayAppend(array, &value);
    fail_unless(ret == EON_OK, "array append(%i) failed", value);
    eon_arrayFree(array);
}
END_TEST


START_TEST(test_arrayLengthAfterAppendOne)
{
    eon_array *array = eon_arrayNew(23, sizeof(int));
    EON_Status ret = EON_ERROR;
    int value = 42, len = 0;

    ret = eon_arrayAppend(array, &value);
    ret = eon_arrayLength(array, &len);
    fail_unless(len == 1, "array after append has inconsistent length=%i", len);
    eon_arrayFree(array);
}
END_TEST

START_TEST(test_arrayGetAfterAppendOne)
{
    eon_array *array = eon_arrayNew(23, sizeof(int));
    EON_Status ret = EON_ERROR;
    int value = 42;
    int *elem = NULL;

    ret = eon_arrayAppend(array, &value);
    elem = eon_arrayGet(array, 0);
    fail_unless(elem != NULL, "array get(%i) failed", 1);
    fail_unless(*elem == value, "got=%i expected=%i", *elem, value);
    eon_arrayFree(array);
}
END_TEST

START_TEST(test_arrayGetPastLength)
{
    eon_array *array = eon_arrayNew(23, sizeof(int));
    EON_Status ret = EON_ERROR;
    int value = 42;
    int *elem = NULL;

    ret = eon_arrayAppend(array, &value);
    elem = eon_arrayGet(array, 1);
    fail_unless(elem == NULL, "array get past length succeded");
    eon_arrayFree(array);
}
END_TEST

START_TEST(test_arrayGetFromEmpty)
{
    eon_array *array = eon_arrayNew(23, sizeof(int));
    int *elem = NULL;

    elem = eon_arrayGet(array, 0);
    fail_unless(elem == NULL, "array get from empty succeded");
    eon_arrayFree(array);
}
END_TEST

START_TEST(test_arrayAppendN)
{
    eon_array *array = eon_arrayNew(23, sizeof(int));
    EON_Status ret = EON_ERROR;
    int j = 0, len = 0,  LEN = 11;

    for (j = 0; j < LEN; j++) {
        ret = eon_arrayAppend(array, &j);
        fail_unless(ret == EON_OK, "array append(%i) failed", j);
    }
    ret = eon_arrayLength(array, &len);
    fail_unless(len == LEN,
                "array length mismatch obtained=%i expected=%i",
                len, LEN);
    eon_arrayFree(array);
}
END_TEST

START_TEST(test_arrayAppendNAndGet)
{
    eon_array *array = eon_arrayNew(23, sizeof(int));
    EON_Status ret = EON_ERROR;
    int j = 0,  LEN = 11;

    for (j = 0; j < LEN; j++) {
        ret = eon_arrayAppend(array, &j);
    }
    for (j = 0; j < LEN; j++) {
        int *elem = eon_arrayGet(array, j);
        fail_unless(elem != NULL, "lost the %i-th element", j);
        fail_unless(*elem == j, "%i-th element mismatch found=%i", j, *elem);
    }
    eon_arrayFree(array);
}
END_TEST

START_TEST(test_arrayAppendNWithGrow)
{
    eon_array *array = eon_arrayNew(1, sizeof(int));
    EON_Status ret = EON_ERROR;
    int j = 0, len = 0,  LEN = 11;

    for (j = 0; j < LEN; j++) {
        ret = eon_arrayAppend(array, &j);
        fail_unless(ret == EON_OK, "array append(%i) failed", j);
    }
    ret = eon_arrayLength(array, &len);
    fail_unless(len == LEN,
                "array length mismatch obtained=%i expected=%i",
                len, LEN);
    eon_arrayFree(array);
}
END_TEST

START_TEST(test_arrayAppendNWithGrowAndGetEarly)
{
    eon_array *array = eon_arrayNew(1, sizeof(int));
    EON_Status ret = EON_ERROR;
    int j = 0,  LEN = 11;
    int *elem = NULL;

    for (j = 0; j < LEN; j++) {
        ret = eon_arrayAppend(array, &j);
        elem = eon_arrayGet(array, j);
        fail_unless(elem != NULL, "lost the %i-th element", j);
        fail_unless(*elem == j, "%i-th element mismatch found=%i", j, *elem);
    }
    eon_arrayFree(array);
}
END_TEST

START_TEST(test_arrayAppendNWithGrowAndGet)
{
    eon_array *array = eon_arrayNew(1, sizeof(int));
    EON_Status ret = EON_ERROR;
    int j = 0,  LEN = 11;

    for (j = 0; j < LEN; j++) {
        ret = eon_arrayAppend(array, &j);
    }
    for (j = 0; j < LEN; j++) {
        int *elem = eon_arrayGet(array, j);
        fail_unless(elem != NULL, "lost the %i-th element", j);
        fail_unless(*elem == j, "%i-th element mismatch found=%i", j, *elem);
    }
    eon_arrayFree(array);
}
END_TEST


/*************************************************************************/

static Suite *eon3d_suiteArray(void)
{
    Suite *s = suite_create("eon3d.core.array");

    TCase *tcArray = tcase_create("array");
    tcase_add_test(tcArray, test_arrayNewZeroSizes);
    tcase_add_test(tcArray, test_arrayNewZeroItemSize);
    tcase_add_test(tcArray, test_arrayEmptyLength);
    tcase_add_test(tcArray, test_arrayAppendOne);
    tcase_add_test(tcArray, test_arrayLengthAfterAppendOne);
    tcase_add_test(tcArray, test_arrayGetAfterAppendOne);
    tcase_add_test(tcArray, test_arrayGetPastLength);
    tcase_add_test(tcArray, test_arrayGetFromEmpty);
    tcase_add_test(tcArray, test_arrayAppendN);
    tcase_add_test(tcArray, test_arrayAppendNAndGet);
    tcase_add_test(tcArray, test_arrayAppendNWithGrow);
    tcase_add_test(tcArray, test_arrayAppendNWithGrowAndGetEarly);
    tcase_add_test(tcArray, test_arrayAppendNWithGrowAndGet);

    suite_add_tcase(s, tcArray);

    return s;
}

/*************************************************************************/

int main(int argc, char *argv[])
{
    int number_failed = 0;

    Suite *s = eon3d_suiteArray();
    SRunner *sr = srunner_create(s);

    srunner_run_all(sr, CK_ENV);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

/*************************************************************************/

/* vim: set ts=4 sw=4 et */
/* EOF */

