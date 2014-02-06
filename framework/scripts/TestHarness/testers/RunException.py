from RunApp import RunApp

class RunException(RunApp):

  def getValidParams():
    params = RunApp.getValidParams()

    params.addParam('expect_err', "A regular expression that must occur in the ouput. (Test may terminiate unexpectedly and be considered passing)")
    params.addParam('expect_assert', "DEBUG MODE ONLY: A regular expression that must occur in the ouput. (Test may terminiate unexpectedly and be considered passing)")
    params.addParam('should_crash', True, "Inidicates that the test is expected to crash or otherwise terminate early")

    return params
  getValidParams = staticmethod(getValidParams)

  def __init__(self, name, params):
    RunApp.__init__(self, name, params)

  def checkRunnable(self, options):
    if options.enable_recover:
      reason = 'skipped (RunException RECOVER)'
      return (False, reason)
    return (True, '')

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
      (reason, output) = RunApp.processResults(self, moose_dir, retcode, options, output)

    return (reason, output)
