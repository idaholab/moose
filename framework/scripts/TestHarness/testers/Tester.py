from options import *
from util import *
from InputParameters import InputParameters

class Tester(object):
  # Static Method
  def getValidParams():
    params = InputParameters()

    # Common Options
    params.addRequiredParam('type', "The type of test of Tester to create for this test.")
    params.addParam('max_time',   300, "The maximum in seconds that the test will be allowed to run.")
    params.addParam('skip',     False, "If supplied will skip the test and print the reason given for doing so.")
    params.addParam('deleted',         "Tests that only show up when using the '-e' option (Permanently skipped or not implemented).")

    params.addParam('heavy',    False, "Set to True if this test should only be run when the '--heavy' option is used.")
    params.addParam('group',       [], "A list of groups for which this test belongs.")
    params.addParam('prereq',      [], "A list of prereq tests that need to run successfully before launching this test.")

    # Test Filters
    params.addParam('platform',      ['ALL'], "A list of platforms for which this test will run on. ('ALL', 'DARWIN', 'LINUX', 'SL', 'LION', 'ML')")
    params.addParam('compiler',      ['ALL'], "A list of compilers for which this test is valid on. ('ALL', 'GCC', 'INTEL', 'CLANG')")
    params.addParam('petsc_version', ['ALL'], "A list of petsc versions for which this test will run on, supports normal comparison operators ('<', '>', etc...)")
    params.addParam('mesh_mode',     ['ALL'], "A list of mesh modes for which this test will run ('PARALLEL', 'SERIAL')")
    params.addParam('method',        ['ALL'], "A test that runs under certain executable configurations ('ALL', 'OPT', 'DBG', 'DEVEL', 'OPROF', 'PRO')")
    params.addParam('library_mode',  ['ALL'], "A test that only runs when libraries are built under certain configurations ('ALL', 'STATIC', 'DYNAMIC')")
    params.addParam('dtk',           ['ALL'], "A test that runs only if DTK is detected ('TRUE', 'FALSE')")

    return params
  getValidParams = staticmethod(getValidParams)

  def __init__(self, name, params):
    self.specs = params


  # Override this method to tell the harness whether or not this test should run.
  # This function should return a tuple (Boolean, reason)
  # If a reason is provided it'll be printed and counted as skipped.  If the reason
  # is left blank, the test will not be printed at all nor counted in the test totals.
  def checkRunnable(self, options):
    return (True, '')


  # This method is called prior to running the test.  It can be used to cleanup files
  # or do other preparations before the tester is run
  def prepare(self):
    return


  # This method should return the executable command that will be executed by the tester
  def getCommand(self, options):
    return


  # This method will be called to process the results of running the test.  Any post-test
  # processing should happen in this method
  def processResults(self, moose_dir, retcode, options, output):
    return


  # This is the base level runnable check common to all Testers.  DO NOT override
  # this method in any of your derived classes.  Instead see "checkRunnable"
  def checkRunnableBase(self, options, checks):
    reason = ''

    # Are we running only tests in a specific group?
    if options.group <> 'ALL' and options.group not in self.specs[GROUP]:
      return (False, reason)
    if options.not_group <> '' and options.not_group in self.specs[GROUP]:
      return (False, reason)

    # Store regexp for matching tests if --re is used
    if options.reg_exp:
      match_regexp = re.compile(options.reg_exp)

    # If --re then only test matching regexp. Needs to run before other SKIP methods
    if options.reg_exp and not match_regexp.search(self.specs[TEST_NAME]):
      return (False, reason)

    # Check for deleted tests
    if self.specs.isValid(DELETED):
      if options.extra_info:
        # We might want to trim the string so it formats nicely
        if len(self.specs[DELETED]) >= TERM_COLS - (len(self.specs[TEST_NAME])+21):
          test_reason = (self.specs[DELETED])[:(TERM_COLS - (len(self.specs[TEST_NAME])+24))] + '...'
        else:
          test_reason = self.specs[DELETED]
        reason = 'deleted (' + test_reason + ')'
      return (False, reason)

    # Check for skipped tests
    if self.specs.type(SKIP) is bool and self.specs[SKIP]:
      # Backwards compatible (no reason)
      return (False, 'skipped')
    elif self.specs.type(SKIP) is not bool and self.specs.isValid(SKIP):
      skip_message = self.specs[SKIP]
      # We might want to trim the string so it formats nicely
      if len(skip_message) >= TERM_COLS - (len(self.specs[TEST_NAME])+21):
        test_reason = (skip_message)[:(TERM_COLS - (len(self.specs[TEST_NAME])+24))] + '...'
      else:
        test_reason = skip_message
      reason = 'skipped (' + test_reason + ')'
      return (False, reason)
    # If were testing for SCALE_REFINE, then only run tests with a SCALE_REFINE set
    elif options.store_time and self.specs[SCALE_REFINE] == 0:
      return (False, reason)
    # If we're testing with valgrind, then skip tests that require parallel or threads
    elif options.enable_valgrind and (self.specs[MIN_PARALLEL] > 1 or self.specs[MIN_THREADS] > 1):
      reason = 'skipped (Valgrind requires serial)'
      return (False, reason)
    elif options.enable_valgrind and self.specs[NO_VALGRIND]:
      reason = 'skipped (NO VALGRIND)'
      return (False, reason)

    # Check for PETSc versions
    (petsc_status, logic_reason, petsc_version) = checkPetscVersion(checks, self.specs)
    if not petsc_status:
      reason = 'skipped (using PETSc ' + str(checks[PETSC_VERSION]) + ' REQ: ' + logic_reason + ' ' + petsc_version + ')'
      return (False, reason)

    # PETSc is being explicitly checked above
    local_checks = [PLATFORM, COMPILER, MESH_MODE, METHOD, LIBRARY_MODE, DTK]
    for check in local_checks:
      test_platforms = set()
      for x in self.specs[check]:
        test_platforms.add(x.upper())
      if not len(test_platforms.intersection(checks[check])):
        reason = 'skipped (' + re.sub(r'\[|\]', '', check).upper() + '!=' + ', '.join(self.specs[check]) + ')'
        return (False, reason)

    # Check for heavy tests
    if options.all_tests or options.heavy_tests:
      if not self.specs[HEAVY] and options.heavy_tests:
        reason = 'skipped (NOT HEAVY)'
        return (False, reason)
    elif self.specs[HEAVY]:
      reason = 'skipped (HEAVY)'
      return (False, reason)

    # Check for positive scale refine values when using store timing options
    if self.specs[SCALE_REFINE] == 0 and options.store_time:
      return (False, reason)

    # Check the return values of the derived classes
    return self.checkRunnable(options)
