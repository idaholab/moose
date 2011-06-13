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
               EXODIFF : ['level1_0000.e', 'level1_0009.e', 'level1_0019.e'] }

