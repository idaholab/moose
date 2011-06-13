from options import *

test_1 = { INPUT : 'restart_diffusion_test_steady.i',
           EXODIFF : ['steady_out.e'] }

test_2 = { INPUT : 'restart_diffusion_test_transient.i',
           EXODIFF : ['out.e'],
           PREREQ : 'test_1' }

