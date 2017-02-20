import util
from RunPythonApp import RunPythonApp

class RunPythonException(RunPythonApp):

    @staticmethod
    def validParams():
        params = RunPythonApp.validParams()
        params.addParam('expect_err', "Search for the supplied string in the output.")
        return params

    def __init__(self, name, params):
        RunPythonApp.__init__(self, name, params)

    def processResults(self, moose_dir, retcode, options, output):

        specs = self.specs

        # Initialize reason output
        reason = 'NO EXPECTED ERROR'

        if specs.isValid('expect_err'):
            if specs['match_literal']:
                out_ok = util.checkOutputForLiteral(output, specs['expect_err'])
            else:
                out_ok = util.checkOutputForPattern(output, specs['expect_err'])

            # Process out_ok
            if out_ok:
                reason = ''

        # Populate the bucket
        if reason != '':
            self.setStatus(reason, self.buck_fail)
        else:
            self.setStatus(self.success_message, self.buck_success)
        return output
