from options import *

testperiodic = { INPUT : 'periodic_bc_test.i',
                 EXODIFF : ['out.e'] }

testwedge = { INPUT : 'wedge.i',
              EXODIFF : ['out_wedge.e'] }

testwedgesys = { INPUT : 'wedge_sys.i',
                 EXODIFF : ['out_wedge_sys.e'] }

testtrapezoid = { INPUT : 'trapezoid.i',
                  EXODIFF : ['out_trapezoid.e'] }

testlevel1 = { INPUT : 'periodic_level_1_test.i',
               EXODIFF : ['level1.e', 'level1.e-s010', 'level1.e-s020'],
               GROUP : 'adaptive'}

subdomain_restricted_vars_test = { INPUT : 'periodic_subdomain_restricted_test.i',
                                   EXODIFF : ['out_restrict.e'] }

auto_wrap_2d_test = { INPUT : 'auto_periodic_bc_test.i',
                 EXODIFF : ['out_auto.e'] }

auto_wrap_3d_test = { INPUT : 'auto_periodic_bc_test_3d.i',
                      EXODIFF : ['out_auto_3d.e'] }
