from options import *

test = { INPUT : 'stateful_prop_test.i',
         EXODIFF : ['out.e'],
         MAX_THREADS : 1,
         MAX_PARALLEL : 1}

# Run again and look at CSV Output
test_csv = { INPUT : 'stateful_prop_test.i',
             CSVDIFF : ['out.csv'],
             MAX_THREADS : 1,
             MAX_PARALLEL : 1,
             TYPE : 'CSVDiff',
             PREREQ : ['test'] }

test_older = { INPUT : 'stateful_prop_test_older.i',
               EXODIFF : ['out_older.e'],
               MAX_THREADS : 1,
               MAX_PARALLEL : 1}

# Run again and look at CSV Output
test_older_csv = { INPUT : 'stateful_prop_test_older.i',
                   CSVDIFF : ['out_older.csv'],
                   MAX_THREADS : 1,
                   MAX_PARALLEL : 1,
                   TYPE : 'CSVDiff',
                   PREREQ : ['test_older'] }

spatial_test = { INPUT : 'stateful_prop_spatial_test.i',
                 EXODIFF : ['out_spatial.e'],
                 MAX_THREADS : 1,
                 MAX_PARALLEL : 1}

computing_initial_residual_test = { INPUT : 'computing_initial_residual_test.i',
                                    EXODIFF : ['computing_initial_residual_test_out.e'],
                                    MAX_THREADS : 1,
                                    MAX_PARALLEL : 1}
