import os, re
import util
import unittest
from Tester import Tester

class RunPythonApp(Tester):
    @staticmethod
    def validParams():
        params = Tester.validParams()
        params.addRequiredParam('input', "The python input file to use for this test.")
        params.addParam('expect_out', "Search for the supplied string in the output.")
        params.addParam('match_literal', False, "Treat expect_out as a string not a regular expression.")
        params.addParam('unittest', False, "This uses unittest.")
        params.addParam('cli_args', '', "String of additional options to pass to Python executable.")
        return params

    def __init__(self, name, params):
        Tester.__init__(self, name, params)

    def getCommand(self, options):
        """
        Returns the python command that executes unit tests
        """
        return os.path.join(self.specs['test_dir'], self.specs['input']) + ' ' + self.specs['cli_args']

    def processResults(self, moose_dir, retcode, options, output):

        specs = self.specs

        # Initialize reason output
        reason = ''

        if specs.isValid('expect_out'):
            if specs['match_literal']:
                out_ok = util.checkOutputForLiteral(output, specs['expect_out'])
            else:
                out_ok = util.checkOutputForPattern(output, specs['expect_out'])

            # Process out_ok
            if (out_ok and retcode != 0):
                reason = 'OUT FOUND BUT CRASH'
            elif (not out_ok):
                reason = 'NO EXPECTED OUT'
        elif specs['unittest']:
            out_ok = util.checkOutputForPattern(output, "^OK$") or util.checkOutputForPattern(output, "^OK\s\(skipped=\d+\)$")
            if not out_ok:
                reason = 'FAILED'

        # Populate the bucket
        if reason != '':
            self.setStatus(reason, self.bucket_fail)
        else:
            self.setStatus(self.success_message, self.bucket_success)
        return output
