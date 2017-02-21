import util
from RunApp import RunApp

class RunException(RunApp):

    @staticmethod
    def validParams():
        params = RunApp.validParams()

        params.addParam('expect_err', "A regular expression that must occur in the ouput. (Test may terminiate unexpectedly and be considered passing)")
        params.addParam('expect_assert', "DEBUG MODE ONLY: A regular expression that must occur in the ouput. (Test may terminiate unexpectedly and be considered passing)")
        params.addParam('should_crash', True, "Inidicates that the test is expected to crash or otherwise terminate early")

        # RunException tests executed in parallel need to have their output redirected to a file, and examined individually
        params['redirect_output'] = True

        return params

    def __init__(self, name, params):
        RunApp.__init__(self, name, params)

    def checkRunnable(self, options):
        if options.enable_recover:
            self.setStatus('RunException RECOVER', self.bucket_skip)
            return False
        return RunApp.checkRunnable(self, options)

    def prepare(self, options):
        if self.getProcs(options) > 1:
            file_paths = []
            for processor_id in xrange(self.getProcs(options)):
                file_paths.append(self.name() + '.processor.{}'.format(processor_id))
            util.deleteFilesAndFolders(self.specs['test_dir'], file_paths, False)

    def processResults(self, moose_dir, retcode, options, output):
        reason = ''
        specs = self.specs

        # Expected errors and assertions might do a lot of things including crash so we
        # will handle them seperately
        if specs.isValid('expect_err'):
            if not util.checkOutputForPattern(output, specs['expect_err']):
                reason = 'NO EXPECTED ERR'
        elif specs.isValid('expect_assert'):
            if options.method == 'dbg':  # Only check asserts in debug mode
                if not util.checkOutputForPattern(output, specs['expect_assert']):
                    reason = 'NO EXPECTED ASSERT'

        if reason == '':
            output = RunApp.processResults(self, moose_dir, retcode, options, output)

        if reason != '':
            self.setStatus(reason, self.bucket_fail)
        else:
            self.setStatus(self.success_message, self.bucket_success)

        return output
