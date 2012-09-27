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

# Note that for these tests
# we are supplying one of the parameters via CLI
gnuplot_ps_out_test = { INPUT : 'output_test_gnuplot.i',
                        CLI_ARGS : ['Output/gnuplot_format=ps'],
                        CHECK_FILES : ['out.gp', 'out.dat'] }

gnuplot_png_out_test = { INPUT : 'output_test_gnuplot.i',
                         CLI_ARGS : ['Output/gnuplot_format=png'],
                         CHECK_FILES : ['out.gp', 'out.dat'],
                         PREREQ : ['gnuplot_ps_out_test'] }

gnuplot_gif_out_test = { INPUT : 'output_test_gnuplot.i',
                         CLI_ARGS : ['Output/gnuplot_format=gif'],
                         CHECK_FILES : ['out.gp', 'out.dat'],
                         PREREQ : ['gnuplot_png_out_test'] }

gnuplot_bad_out_test = { INPUT : 'output_test_gnuplot.i',
                         CLI_ARGS : ['Output/gnuplot_format=magic'],
                         EXPECT_ERR : 'gnuplot format .*? is not supported' }

# Test that YAML dumps are working
yaml_dump_test = { INPUT : 'IGNORED',
                   CLI_ARGS : ['--yaml'],
                   EXPECT_OUT : 'START YAML DATA.*END YAML DATA'}

# Test the --dump is working
dump_test = { INPUT : 'IGNORED',
              CLI_ARGS : ['--dump'],
              EXPECT_OUT : 'block\s*=\s*ANY_BLOCK_ID'}  # Look for one of the default parameters

# Test the --show-input is working
show_input_test = { INPUT : 'output_test_gmv.i',
                    CLI_ARGS : ['--show-input'],
                    PREREQ : ['gmv_out_test'],
                    EXPECT_OUT : 'type\s*=\s*GeneratedMesh'}  # Look for one of the supplied parameters from the input file
