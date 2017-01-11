from RunApp import RunApp

class RunException(RunApp):

  @staticmethod
  def validParams():
    params = RunApp.validParams()

    params.addParam('expect_err', "A regular expression that must occur in the ouput. (Test may terminiate unexpectedly and be considered passing)")
    params.addParam('expect_assert', "DEBUG MODE ONLY: A regular expression that must occur in the ouput. (Test may terminiate unexpectedly and be considered passing)")
    params.addParam('should_crash', True, "Inidicates that the test is expected to crash or otherwise terminate early")

    # Printing errors in parallel often intertwine when multiple processors receive the same error.  We will set max_parallel = 1 by default, but it can be overridden
    params['max_parallel'] = 1

    return params

  def __init__(self, name, params):
    RunApp.__init__(self, name, params)

  def checkRunnable(self, options):
    if options.enable_recover:
      reason = 'skipped (RunException RECOVER)'
      self.setStatus(reason, 'SKIP')
      return False
    return RunApp.checkRunnable(self, options)

  def processResults(self, moose_dir, retcode, options, output):
    reason = ''
    specs = self.specs

    # Expected errors and assertions might do a lot of things including crash so we
    # will handle them seperately
    if specs.isValid('expect_err'):
      if not self.checkOutputForPattern(output, specs['expect_err']):
        reason = 'NO EXPECTED ERR'
    elif specs.isValid('expect_assert'):
      if options.method == 'dbg':  # Only check asserts in debug mode
        if not self.checkOutputForPattern(output, specs['expect_assert']):
          reason = 'NO EXPECTED ASSERT'

    if reason == '':
      output = RunApp.processResults(self, moose_dir, retcode, options, output)

    if reason != '':
      self.setStatus(reason, 'FAIL')

    return output
