/**************************************************************************
 * check_rgb: RGB functions unit test (yes, is that simple)               *
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

START_TEST(test_rgb_create_black)
{
    EON_RGB rgb;
    EON_RGBSet(&rgb, EON_RGB_BLACK, EON_RGB_BLACK, EON_RGB_BLACK);
    fail_unless(rgb.R == EON_RGB_BLACK, "failed to set the R component");
    fail_unless(rgb.G == EON_RGB_BLACK, "failed to set the G component");
    fail_unless(rgb.B == EON_RGB_BLACK, "failed to set the B component");
   /* who cares about alpha? */
}
END_TEST

START_TEST(test_rgb_create_zero)
{
    EON_RGB rgb;
    EON_RGBSet(&rgb, 0, 0, 0);
    fail_unless(rgb.R == 0, "failed to set the R component");
    fail_unless(rgb.G == 0, "failed to set the G component");
    fail_unless(rgb.B == 0, "failed to set the B component");
   /* who cares about alpha? */
}
END_TEST

START_TEST(test_rgb_create_rand)
{
    int R = 42, G = 63, B = 101;
    EON_RGB rgb;
    EON_RGBSet(&rgb, R, G, B);
    fail_unless(rgb.R == R, "failed to set the R component");
    fail_unless(rgb.G == G, "failed to set the G component");
    fail_unless(rgb.B == B, "failed to set the B component");
   /* who cares about alpha? */
}
END_TEST

START_TEST(test_rgb_pack_unpack)
{
    int R = 42, G = 63, B = 101;
    EON_RGB rgb1, rgb2;
    EON_UInt32 color = 0;

    EON_RGBSet(&rgb1, R, G, B);
    color = EON_RGBPack(&rgb1);
    EON_RGBUnpack(&rgb2, color);

    fail_if(rgb1.R != rgb2.R, "failed to restore the R component (%i vs %i)",
                              rgb1.R, rgb2.R);
    fail_if(rgb1.G != rgb2.G, "failed to restore the G component (%i vs %i)",
                              rgb1.G, rgb2.G);
    fail_if(rgb1.B != rgb2.B, "failed to restore the B component (%i vs %i)",
                              rgb1.B, rgb2.B);
   /* who cares about alpha? */
}
END_TEST



/*************************************************************************/

static Suite *eon3d_suite_rgb(void)
{
    Suite *s = suite_create("eon3d_core_rgb");

    TCase *tc_rgb = tcase_create("rgb");
    tcase_add_test(tc_rgb, test_rgb_create_black);
    tcase_add_test(tc_rgb, test_rgb_create_zero);
    tcase_add_test(tc_rgb, test_rgb_create_rand);
    tcase_add_test(tc_rgb, test_rgb_pack_unpack);

    suite_add_tcase(s, tc_rgb);

    return s;
}

/*************************************************************************/

int main(int argc, char *argv[])
{
    int number_failed = 0;

    Suite *s = eon3d_suite_rgb();
    SRunner *sr = srunner_create(s);

    srunner_run_all(sr, CK_ENV);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

/*************************************************************************/

/* vim: set ts=4 sw=4 et */
/* EOF */

