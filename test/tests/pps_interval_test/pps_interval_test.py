from options import *

test_interval = { INPUT : 'pps_out_interval.i',
                  EXODIFF : ['pps_out_interval.e'] }

# An eleventh item (Initial condition + 10 time steps) PPS table should be in the output
test_interval_verify = { INPUT : 'pps_out_interval.i',
                         EXPECT_OUT : '(?:^\|\s+\d\.\d.*\n){11}',
                         PREREQ : ['test_interval'] }

test_bad_interval = { INPUT : 'pps_bad_interval.i',
                      EXPECT_ERR : '\"screen_interval \(\d+\)\" must evenly divide \"interval' }

test_bad_interval2 = { INPUT : 'pps_bad_interval2.i',
                       EXPECT_ERR : '\"screen_interval \(\d+\)\" must be less than or equal to \"interval' }

test_bad_interval3 = { INPUT : 'pps_bad_interval3.i',
                       EXPECT_ERR : '\"output_initial\" is set to false' }

