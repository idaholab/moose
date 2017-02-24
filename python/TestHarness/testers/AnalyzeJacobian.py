import re, os, sys
import util
from Tester import Tester
from RunParallel import RunParallel # For TIMEOUT value

class AnalyzeJacobian(Tester):

    @staticmethod
    def validParams():
        params = Tester.validParams()
        params.addRequiredParam('input',  "The input file to use for this test.")
        params.addParam('test_name',      "The name of the test - populated automatically")
        params.addParam('expect_out',     "A regular expression that must occur in the input in order for the test to be considered passing.")
        params.addParam('resize_mesh', False, "Resize the input mesh")
        params.addParam('off_diagonal', True, "Also test the off-diagonal Jacobian entries")
        params.addParam('mesh_size',   1, "Resize the input mesh")

        return params


    def __init__(self, name, params):
        Tester.__init__(self, name, params)


    # Check if numpy is available
    def checkRunnable(self, options):
        try:
            import numpy
            return (True, '')
        except Exception as e:
            return (False, 'skipped (no numpy)')


    def getCommand(self, options):
        specs = self.specs
        # Create the command line string to run
        command = os.path.join(specs['moose_dir'], 'python', 'jacobiandebug', 'analyzejacobian.py')

        # Check for built application
        if not options.dry_run and not os.path.exists(command):
            print 'Application not found: ' + str(specs['executable'])
            sys.exit(1)

        mesh_options = ' -m %s' % options.method
        if specs['resize_mesh'] :
            mesh_options += ' -r -s %d' % specs['mesh_size']

        if not specs['off_diagonal'] :
            mesh_options += ' -D'

        command += mesh_options + ' ' + specs['input'] + ' -e ' + specs['executable'] + ' ' + ' '.join(specs['cli_args'])

        return command


    def processResults(self, moose_dir, retcode, options, output):
        reason = ''
        specs = self.specs
        if specs.isValid('expect_out'):
            out_ok = util.checkOutputForPattern(output, specs['expect_out'])
            if (out_ok and retcode != 0):
                reason = 'OUT FOUND BUT CRASH'
            elif (not out_ok):
                reason = 'NO EXPECTED OUT'
        if reason == '':
            if retcode == RunParallel.TIMEOUT:
                reason = 'TIMEOUT'
            elif retcode != 0 :
                reason = 'CRASH'

        # populate status bucket
        if reason != '':
            self.setStatus(reason, self.bucket_fail)
        else:
            self.setStatus(self.success_message, self.bucket_success)

        return output
