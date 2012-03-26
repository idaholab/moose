from options import *

testDirichletbc = { INPUT : 'forcing_function_test.i',
                    EXODIFF : ['out.e'] }

testNeumannbc = { INPUT : 'forcing_function_neumannbc_test.i',
                  EXODIFF : ['neumannbc_out.e'] }

testParseCheck = { INPUT : 'forcing_function_error_check.i',
               EXPECT_ERR : 'The value in ParsedFunction ".*?" contains quotes' }
