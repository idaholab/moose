#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from TestHarness import util
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
        if (params.isValid("expect_err") == False and params.isValid("expect_assert") == False):
            raise RuntimeError('Either "expect_err" or "expect_assert" must be supplied in RunException')

    def checkRunnable(self, options):
        if options.enable_recover:
            self.addCaveats('RunException RECOVER')
            self.setStatus(self.bucket_skip.status, self.bucket_skip)
            return False
        return RunApp.checkRunnable(self, options)

    def prepare(self, options):
        if self.getProcs(options) > 1:
            file_paths = []
            for processor_id in xrange(self.getProcs(options)):
                file_paths.append(self.name() + '.processor.{}'.format(processor_id))
            util.deleteFilesAndFolders(self.specs['test_dir'], file_paths, False)

    def processResults(self, moose_dir, options, output):
        reason = ''
        specs = self.specs

        if self.hasRedirectedOutput(options):
            redirected_output = util.getOutputFromFiles(self, options)
            output += redirected_output

        # Expected errors and assertions might do a lot of things including crash so we
        # will handle them seperately
        if specs.isValid('expect_err'):
            if not util.checkOutputForPattern(output, specs['expect_err']):
                reason = 'EXPECTED ERROR MISSING'
        elif specs.isValid('expect_assert'):
            if options.method in ['dbg', 'devel']:  # Only check asserts in debug and devel modes
                if not util.checkOutputForPattern(output, specs['expect_assert']):
                    reason = 'EXPECTED ASSERT MISSING'

        # If we've set a reason right here, we should report the pattern that we were unable to match.
        if reason != '':
            output += "#"*80 + "\n\nUnable to match the following pattern against the program's output:\n\n" + specs['expect_err'] + "\n"

        if reason == '':
            RunApp.testFileOutput(self, moose_dir, options, output)

        if reason != '':
            self.setStatus(reason, self.bucket_fail)
        else:
            self.setStatus(self.success_message, self.bucket_success)

        return output
