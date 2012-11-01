from options import *

test2dMesh = { INPUT : '2_d_mms_sine_test.i',
               EXODIFF : ['2_d_out.e'] }

test3dMesh = { INPUT : '3_d_mms_sine_test.i',
               EXODIFF : ['3_d_out.e'] }

test2dCSV = { INPUT : '2_d_mms_sine_postprocessor_test.i',
              CSVDIFF : ['2_d_postprocessor_out.csv'],
              TYPE : 'CSVDiff' }

test3dCSV = { INPUT : '3_d_mms_sine_postprocessor_test.i',
              CSVDIFF : ['3_d_postprocessor_out.csv'],
              TYPE : 'CSVDiff' }

