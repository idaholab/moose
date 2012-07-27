from options import *

test = { INPUT : 'constant_ic_test.i',
         EXODIFF : ['out.e'],
         SCALE_REFINE : 5
         }

subdomain_test = {
    INPUT : 'subdomain_constant_ic_test.i',
    EXODIFF : ['subdomain_constant_ic_test_out.e'],
    MAX_THREADS : 1,   # Why does this only work with one thread?
    SCALE_REFINE : 5
}

