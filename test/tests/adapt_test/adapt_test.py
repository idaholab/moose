from options import *

test = { INPUT : 'adapt_test.i',
         EXODIFF : ['out_0001.e'] }

test_time   = { INPUT : 'adapt_time_test.i',
                EXODIFF : ['out_time_0006.e'] }

displaced_test = {
	INPUT : 'displaced_adapt_test.i',
	EXODIFF : ['out_displaced_0002.e' , 'displaced_adapt_test_out_0001.e']
}