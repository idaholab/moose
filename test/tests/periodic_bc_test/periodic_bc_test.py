from options import *

testperiodic = { INPUT : 'periodic_bc_test.i',
                 EXODIFF : ['out.e'],
                 MAX_THREADS : 1}

testwedge = { INPUT : 'wedge.i',
              EXODIFF : ['out_wedge.e'],
              MAX_THREADS : 1 }

testwedgesys = { INPUT : 'wedge_sys.i',
                 EXODIFF : ['out_wedge_sys.e'],
                 MAX_THREADS : 1 }

testtrapezoid = { INPUT : 'trapezoid.i',
                  EXODIFF : ['out_trapezoid.e'],
                  MAX_THREADS : 1}

testlevel1 = { INPUT : 'periodic_level_1_test.i',
               EXODIFF : ['level1.e', 'level1.e-s010', 'level1.e-s020'],
               MAX_THREADS : 1,
               GROUP : 'adaptive'}

subdomain_restricted_vars_test = { INPUT : 'periodic_subdomain_restricted_test.i',
                                   MAX_THREADS : 1,
                                   EXODIFF : ['out_restrict.e'] }

auto_wrap_2d_test = { INPUT : 'auto_periodic_bc_test.i',
                      EXODIFF : ['out_auto.e'],
                      MAX_THREADS : 1}

auto_wrap_3d_test = { INPUT : 'auto_periodic_bc_test_3d.i',
                      EXODIFF : ['out_auto_3d.e'],
                      MAX_THREADS : 1}
