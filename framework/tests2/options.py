# This file holds all the options that tests can use declared as global variables
# so typos will crash instead of just producing weird output

# General options
INPUT    = 'input'              # The input.i file to use
SKIP     = 'skip'               # Print "skipped" instead of running this test
MAX_TIME = 'max_time'           # Test will fail if it exceeds this time in seconds (accuracy ~1s)
TEST_NAME = 'testname'          # The name of the test as it appears in output, this is
                                # set to module_name.dict_name by default

# Types of tests
EXODIFF      = 'exodiff'        # A list of files to exodiff
CSVDIFF      = 'csvdiff'        # A list of files to CSV diff
GOLD_DIR     = 'gold_dir'       # The directory where the "golden standard" files resides relative to TEST_DIR
TEST_DIR     = 'test_dir'       # The directory where the test resides, this is populated automatically
SHOULD_CRASH = 'should_crash'   # Set to true if this test should crash
EXPECT_ERR   = 'expect_err'     # string - This string must occur in the output for the test to pass
ERRORS       = 'errors'         # list[strings] - The test will fail if any string occurs in the output

# Groups of tests
GROUP = 'group'                 # Specify a single group or a list of groups that this test belongs to
HEAVY = 'heavy'                 # Set to true if this test should only be run when the --heavy option is used

# Scaling options
SCALE    = 'scale'              # Set to True to scale?
DOFS     = 'dofs'               # Default number of DOFS to use when --scale is specified
DOF_GOLD = 'dof_gold'           # A dictionary specifying which gold files to use for higher numbers of dofs
                                # for example: { 1000 : 'out1k.e', 2000 : 'out2k.e' }
PARALLEL = 'parallel'           # Number of processes to use
THREADS  = 'threads'            # Number of threads to use

# Test Timing Options
TIME      = 'time'              # Set to True to time this test
TIME_DOFS = 'time_dofs'         # The number of dofs to run the problem at for timeing purposes
TIME_GOLD = 'time_gold'         # The gold file to exodiff output for timing runs, default no exodiff

# EXODIFF options
EXO_OPTIONS = 'exo_options'     # This variable contains a list of options for the exodiff tool
ABS_ZERO    = 'abs_zero'        # Absolute zero value passed to the exodiff tool
REL_ERR     = 'rel_err'         # Relative error value passed to the exodiff tool

EXO_OPTS = { ABS_ZERO : 1e-11, 
             REL_ERR : 5.5e-6 }

# Default test options: these are use if an option is not specified
DEFAULTS = { EXODIFF : [],
             CSVDIFF : [],
             ERRORS : ['ERROR', 'command not found', 'erminate called after throwing an instance of'],
             SHOULD_CRASH : False,
             EXPECT_ERR : None,
             GROUP : [],
             HEAVY : False,
             DOFS : 0,
             PARALLEL : 0,
             THREADS : 0,
             MAX_TIME : 180,
             SKIP : False,
             TIME : False,
             TIME_DOFS : 4000,
             TIME_GOLD : None,
             EXO_OPTIONS : EXO_OPTS,
             GOLD_DIR : 'gold'
             # TEST_DIR is automatically populated to the location of the py file
             # TEST_NAME is automatically populated to module_name.dict_name
}
