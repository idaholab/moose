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

        params.addParam('expect_err', "A regular expression or literal string that must occur in the output (see match_literal). (Test may terminate unexpectedly and be considered passing)")
        params.addParam('expect_assert', "DEBUG MODE ONLY: A regular expression that must occur in the output. (Test may terminate unexpectedly and be considered passing)")
        params.addParam('should_crash', True, "Indicates that the test is expected to crash or otherwise terminate early")

        # RunException tests executed in parallel need to have their output redirected to a file, and examined individually
        params['redirect_output'] = True

        return params

    def __init__(self, name, params):
        RunApp.__init__(self, name, params)
        if (params.isValid("expect_err") == False and params.isValid("expect_assert") == False):
            raise RuntimeError('Either "expect_err" or "expect_assert" must be supplied in RunException')

    def checkRunnable(self, options):
        if options.enable_recover:
            self.addCaveats('type=RunException')
            self.setStatus(self.skip)
            return False
        return RunApp.checkRunnable(self, options)

    def prepare(self, options):
        if self.getProcs(options) > 1:
            file_paths = []
            for processor_id in range(self.getProcs(options)):
                file_paths.append(self.name() + '.processor.{}'.format(processor_id))
            util.deleteFilesAndFolders(self.getTestDir(), file_paths, False)

    def processResults(self, moose_dir, options, output):
        # Exceptions are written to stderr, which can be interleaved so we normally redirect these
        # separate files. Here we must gather those file outputs before processing
        if self.hasRedirectedOutput(options):
            redirected_output = util.getOutputFromFiles(self, options)
            output += redirected_output

        output += self.testFileOutput(moose_dir, options, output)
        self.testExitCodes(moose_dir, options, output)

        return output
