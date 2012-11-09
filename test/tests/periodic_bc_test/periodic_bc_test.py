from options import *

testperiodic = { INPUT : 'periodic_bc_test.i',
                 EXODIFF : ['out.e'],
                 MAX_THREADS : 1,
                 MAX_PARALLEL: 1,
                 MESH_MODE : ['SERIAL'],
                 GROUP : 'periodic'}

testwedge = { INPUT : 'wedge.i',
              EXODIFF : ['out_wedge.e'],
              MAX_THREADS : 1,
              MESH_MODE : ['SERIAL'],
              GROUP : 'periodic' }

testwedgesys = { INPUT : 'wedge_sys.i',
                 EXODIFF : ['out_wedge_sys.e'],
                 MAX_THREADS : 1,
                 MESH_MODE : ['SERIAL'],
                 GROUP : 'periodic' }

testtrapezoid = { INPUT : 'trapezoid.i',
                  EXODIFF : ['out_trapezoid.e'],
                  MAX_THREADS : 1,
                  MESH_MODE : ['SERIAL'],
                  GROUP : 'periodic'}

testlevel1 = { INPUT : 'periodic_level_1_test.i',
               EXODIFF : ['level1.e', 'level1.e-s010', 'level1.e-s020'],
               MAX_THREADS : 1,
               NO_VALGRIND: True,   # Test takes too long through Valgrind
               MESH_MODE : ['SERIAL'],
               GROUP : 'adaptive, periodic'}

subdomain_restricted_vars_test = { INPUT : 'periodic_subdomain_restricted_test.i',
                                   MAX_THREADS : 1,
                                   EXODIFF : ['out_restrict.e'],
                                   GROUP : 'periodic',
                                   MESH_MODE : ['SERIAL'],
                                   DELETED : 'Libmesh Bug #1410'}

auto_wrap_2d_test = { INPUT : 'auto_periodic_bc_test.i',
                      EXODIFF : ['out_auto.e'],
                      MAX_THREADS : 1,
                      MAX_PARALLEL: 1,
                      MESH_MODE : ['SERIAL'],
                      GROUP : 'periodic'}

auto_wrap_2d_test_error_check = { INPUT : 'auto_periodic_bc_test.i',
			          CLI_ARGS : ['AuxKernels/periodic_dist/point="0 99999 0"'],
			          EXPECT_ERR : '"point" is outside of the domain',
                                  GROUP : 'periodic',
				  PREREQ : ['auto_wrap_2d_test'] }

auto_wrap_3d_test = { INPUT : 'auto_periodic_bc_test_3d.i',
                      EXODIFF : ['out_auto_3d.e'],
                      MAX_THREADS : 1,
                      MAX_PARALLEL: 1,
                      NO_VALGRIND: True,   # Test takes too long through Valgrind
                      MESH_MODE : ['SERIAL'],
                      GROUP : 'periodic'}

all_periodic_trans_test = { INPUT : 'all_periodic_trans.i',
                            EXODIFF : ['all_periodic_trans_out.e'],
                            MAX_THREADS : 1,
                            MAX_PARALLEL: 1,
                            MESH_MODE : ['SERIAL'],
                            GROUP : 'periodic'}

orthogonal_pbc_on_square_test = { INPUT : 'orthogonal_pbc_on_square.i',
                                  EXODIFF : ['orthogonal_pbc_on_square_out.e'],
                                  MAX_THREADS : 1,
                                  MAX_PARALLEL: 1,
                                  MESH_MODE : ['SERIAL'],
                                  GROUP : 'periodic'}

parallel_pbc_using_trans_test = { INPUT : 'parallel_pbc_using_trans.i',
                                  EXODIFF : ['parallel_pbc_using_trans_out.e'],
                                  MAX_THREADS : 1,
                                  MAX_PARALLEL: 1,
                                  MESH_MODE : ['SERIAL'],
                                  GROUP : 'periodic'}
