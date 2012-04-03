from options import *

test = { INPUT : 'constant_ic_test.i',
         EXODIFF : ['out.e'],
         SCALE_REFINE : 5
         }

subdomain_test = {
    INPUT : 'subdomain_constant_ic_test.i',
    EXODIFF : ['subdomain_constant_ic_test_out.e'],
    SCALE_REFINE : 5
}

