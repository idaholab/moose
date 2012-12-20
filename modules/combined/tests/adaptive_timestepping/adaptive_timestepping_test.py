from options import *

test_grow_init_dt = { INPUT : 'adapt_tstep_grow_init_dt.i',
                    EXODIFF : ['out_grow_init_dt.e']}

test_shrink_init_dt = { INPUT : 'adapt_tstep_shrink_init_dt.i',
                        EXODIFF : ['out_shrink_init_dt.e']}

test_grow_dtfunc = { INPUT : 'adapt_tstep_grow_dtfunc.i',
                    EXODIFF : ['out_grow_dtfunc.e']}

test_exception_creep = { INPUT : 'exception_creep_iterations.i',
                         EXODIFF : ['out_except_creep.e']}

test_function_change = { INPUT : 'adapt_tstep_function_change.i',
                         EXODIFF : ['out_function_change.e'],
			 REL_ERR : 9e-6}
