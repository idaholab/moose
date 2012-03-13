from options import *

gmv_out_test = { INPUT : 'output_test_gmv.i',
                 CHECK_FILES : ['out_0000.gmv'] }

tecplot_out_test = { INPUT : 'output_test_tecplot.i',
                     CHECK_FILES : ['out_0000.plt'] }

sln_out_test = { INPUT : 'output_test_sln.i',
                 CHECK_FILES : ['out.slh'] }

nemesis_out_test = { INPUT : 'output_test_nemesis.i',
                     MESH_MODE : ['PARALLEL'],
                     CHECK_FILES : ['out.e.1.0'] }

nemesis_out_check_test = { INPUT : 'output_test_nemesis.i',
                           EXPECT_ERR : 'Nemesis not supported when compiled without --enable-parmesh',
                           MESH_MODE : ['SERIAL'] }

