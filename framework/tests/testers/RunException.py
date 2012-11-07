from options import *
from RunApp import RunApp

class RunException(RunApp):

  def getValidParams():
    params = RunApp.getValidParams()

    # Valgrind
    params.addParam('expect_err', "A regular expression that must occur in the ouput. (Test may terminiate unexpectedly and be considered passing)")
    params.addParam('expect_assert', "DEBUG MODE ONLY: A regular expression that must occur in the ouput. (Test may terminiate unexpectedly and be considered passing)")
    params.addParam('should_crash', True, "Inidicates that the test is expected to crash or otherwise terminate early")

    return params
  getValidParams = staticmethod(getValidParams)

  def __init__(self, klass, specs):
    RunApp.__init__(self, klass, specs)

  def processResults(self, moose_dir, retcode, options, output):
    reason = ''
    specs = self.specs

    # Expected errors and assertions might do a lot of things including crash so we
    # will handle them seperately
    if specs[EXPECT_ERR] != None:
      if not self.checkOutputForPattern(output, specs[EXPECT_ERR]):
        reason = 'NO EXPECTED ERR'
    elif specs[EXPECT_ASSERT] != None:
      if options.method == 'dbg':  # Only check asserts in debug mode
        if not self.checkOutputForPattern(output, specs[EXPECT_ASSERT]):
          reason = 'NO EXPECTED ASSERT'
    else:
      # Check the general error message and program crash possibilities
      if specs[EXPECT_ERR] != None and specs[EXPECT_ERR] not in output:
        reason = 'NO EXPECTED ERR'

    if reason != '':
      (reason, output) = RunApp.processResults(self, moose_dir, retcode, options, output)

    return (reason, output)