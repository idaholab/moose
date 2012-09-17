from options import *

testperiodic = { INPUT : 'periodic_bc_test.i',
                 EXODIFF : ['out.e'],
                 MAX_THREADS : 1,
                 MAX_PARALLEL: 1,
                 GROUP : 'periodic'}

testwedge = { INPUT : 'wedge.i',
              EXODIFF : ['out_wedge.e'],
              MAX_THREADS : 1,
              GROUP : 'periodic' }

testwedgesys = { INPUT : 'wedge_sys.i',
                 EXODIFF : ['out_wedge_sys.e'],
                 MAX_THREADS : 1,
                 GROUP : 'periodic' }

testtrapezoid = { INPUT : 'trapezoid.i',
                  EXODIFF : ['out_trapezoid.e'],
                  MAX_THREADS : 1,
                  GROUP : 'periodic'}

testlevel1 = { INPUT : 'periodic_level_1_test.i',
               EXODIFF : ['level1.e', 'level1.e-s010', 'level1.e-s020'],
               MAX_THREADS : 1,
               NO_VALGRIND: True,   # Test takes too long through Valgrind
               GROUP : 'adaptive, periodic'}

subdomain_restricted_vars_test = { INPUT : 'periodic_subdomain_restricted_test.i',
                                   MAX_THREADS : 1,
                                   EXODIFF : ['out_restrict.e'],
                                   GROUP : 'periodic',
                                   SKIP : 'Currently Broken'}

auto_wrap_2d_test = { INPUT : 'auto_periodic_bc_test.i',
                      EXODIFF : ['out_auto.e'],
                      MAX_THREADS : 1,
                      MAX_PARALLEL: 1,
                      GROUP : 'periodic'}

auto_wrap_3d_test = { INPUT : 'auto_periodic_bc_test_3d.i',
                      EXODIFF : ['out_auto_3d.e'],
                      MAX_THREADS : 1,
                      MAX_PARALLEL: 1,
                      NO_VALGRIND: True,   # Test takes too long through Valgrind
                      GROUP : 'periodic'}

all_periodic_trans_test = { INPUT : 'all_periodic_trans.i',
                            EXODIFF : ['all_periodic_trans_out.e'],
                            MAX_THREADS : 1,
                            MAX_PARALLEL: 1,
                            GROUP : 'periodic'}

orthogonal_pbc_on_square_test = { INPUT : 'orthogonal_pbc_on_square.i',
                                  EXODIFF : ['orthogonal_pbc_on_square_out.e'],
                                  MAX_THREADS : 1,
                                  MAX_PARALLEL: 1,
                                  GROUP : 'periodic'}

parallel_pbc_using_trans_test = { INPUT : 'parallel_pbc_using_trans.i',
                                  EXODIFF : ['parallel_pbc_using_trans_out.e'],
                                  MAX_THREADS : 1,
                                  MAX_PARALLEL: 1,
                                  GROUP : 'periodic'}
