# This file holds all the options that tests can use declared as global variables
# so typos will crash instead of just producing weird output

# General options
INPUT         = 'input'          # The input.i file to use
SKIP	      = 'skip'           # Give a reason to skip the test
MAX_TIME      = 'max_time'       # Test will fail if it exceeds this time in seconds (accuracy ~1s)
TEST_NAME     = 'test_name'      # The name of the test as it appears in output, this is
                                 # set to module_name.dict_name by default - DO NOT OVERRIDE!
RELATIVE_PATH = 'relative_path'  # The relative directory where the test resides, this is populated automatically - DO NOT OVERRIDE!
EXECUTABLE    = 'executable'     # The name of the executable, this is populated automatically - DO NOT OVERRIDE!
HOSTNAME      = 'hostname'       # The name of machine that the harness is executing on, populated automatically - DO NOT OVERRIDE!
TYPE          = 'type'           # The type of tester to create
DELETED       = 'deleted'        # Tests that only show up when using the "-e" option (Permanently skipped or not implemented)

################################
##### LIST TYPE PARAMETERS #####
################################
# All parameters in this section must be supplied in list format
PLATFORM      = 'platform'       # A list of platforms set to 'ALL', 'DARWIN', 'LINUX', 'SL', and/or 'LION'
COMPILER      = 'compiler'       # A test that runs only on a given compiler set to 'ALL', 'GCC', 'INTEL' and/or 'CLANG'
PETSC_VERSION = 'petsc_version'  # A test that runs only on the matching PETSC version
MESH_MODE     = 'mesh_mode'      # A test that only runs on a given set of mesh modes 'ALL', 'SERIAL', and/or 'PARALLEL'
METHOD        = 'method'         # A test that only runs under an executable built by the specified method 'ALL', 'OPT', 'DBG', 'DEV'
LIBRARY_MODE  = 'library_mode'   # A test that only runs when libraries are built in a certain mode 'ALL', 'STATIC', 'DYNAMIC'

EXODIFF       = 'exodiff'        # A list of files to exodiff
EXODIFF_OPTS  = 'exodiff_opts'   # A place where the user can put in what ever else needs to be sent out.
CSVDIFF       = 'csvdiff'        # A list of files to CSV diff
CHECK_FILES   = 'check_files'    # A list of files to check for existence

CLI_ARGS      = 'cli_args'       # Additional argument vector to pass to test

# Test Ordering
PREREQ        = 'prereq'         # The list of prereq tests that need to be complete before launching this test

# Groups of tests
GROUP         = 'group'          # Specify a single group or a list of groups that this test belongs to

ERRORS        = 'errors'         # list[strings] - The test will fail if any string occurs in the output
################################
### END LIST TYPE PARAMETERS ###
################################

GOLD_DIR      = 'gold_dir'       # The directory where the "golden standard" files resides relative to TEST_DIR
TEST_DIR      = 'test_dir'       # The directory where the test resides, this is populated automatically
SHOULD_CRASH  = 'should_crash'   # Set to true if this test should crash and we don't really care what the error says
EXPECT_ERR    = 'expect_err'     # string - This string must occur in the output for the test to pass
                                 #          The program may crash if an error is expected and still be considered OK
EXPECT_OUT    = 'expect_out'     # Similar to EXPECT_ERR except the program is expected not to crash
EXPECT_ASSERT = 'expect_assert'  # string - This string must occur in DEBUG mode builds to pass

HEAVY         = 'heavy'          # Boolean: Set to true if this test should only be run when the --heavy option is used

# Parallel testing
MAX_PARALLEL  = 'max_parallel'   # Maximum number of MPI processes this test can be run with      (Default: 1000)
MIN_PARALLEL  = 'min_parallel'   # Minimum number of MPI processes that this test can be run with (Default: 1)
MAX_THREADS   = 'max_thread'     # Max number of threads (Default: 16)
MIN_THREADS   = 'min_thread'     # Min number of threads (Default: 1)
SCALE_REFINE  = 'scale_refine'      # The number of refinements to do when scaling

# EXODIFF/CSV options
ABS_ZERO      = 'abs_zero'       # Absolute zero value passed to the exodiff tool
REL_ERR       = 'rel_err'        # Relative error value passed to the exodiff tool
# EXODIFF options
CUSTOM_CMP    = 'custom_cmp'     # Custom comparison file
USE_OLD_FLOOR = 'use_old_floor'  # Boolean: Use Exodiff old floor option

# Valgrind Options
NO_VALGRIND   = 'no_valgrind'    # Set to True to skip test when testing with --valgrind

# Default test options: these are use if an option is not specified
DEFAULTS = { EXODIFF : [],
             EXODIFF_OPTS : [],
             CSVDIFF : [],
             CHECK_FILES : [],
             CLI_ARGS : [],
             ERRORS : ['ERROR', 'command not found', 'erminate called after throwing an instance of'],
             PLATFORM : ['ALL'],
             COMPILER : ['ALL'],
             PETSC_VERSION : ['ALL'],
             MESH_MODE : ['ALL'],
             METHOD : ['ALL'],
             LIBRARY_MODE : ['ALL'],
             SHOULD_CRASH : False,
             EXPECT_ERR : None,
             EXPECT_OUT : None,
             EXPECT_ASSERT : None,
             GROUP : [],
             HEAVY : False,
             SCALE_REFINE : 0,
             MIN_PARALLEL : 1,
             MAX_PARALLEL : 1000,
             MIN_THREADS: 1,
             MAX_THREADS: 16,
             MAX_TIME : 300,
             SKIP : None,
	     DELETED : None,
             ABS_ZERO : 1e-10,      # Exodiff option
             REL_ERR : 5.5e-6,      # Exodiff option
             CUSTOM_CMP : None,     # Exodiff option
             USE_OLD_FLOOR : False, # Exodiff option
             GOLD_DIR : 'gold',
             PREREQ : [],
             NO_VALGRIND : False
             # TEST_DIR is automatically populated to the location of the py file
             # TEST_NAME is automatically populated to module_name.dict_name
}
