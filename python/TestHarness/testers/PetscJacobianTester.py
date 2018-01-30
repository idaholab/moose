#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from RunApp import RunApp
import re

class PetscJacobianTester(RunApp):

    @staticmethod
    def validParams():
        params = RunApp.validParams()
        params.addParam('ratio_tol', 1e-8, "Relative tolerance to compare the ration against.")
        params.addParam('difference_tol', 1e-8, "Relative tolerance to compare the difference against.")

        return params

    def checkRunnable(self, options):
        if options.enable_recover:
            self.addCaveats('PetscJacTester RECOVER')
            self.setStatus(self.bucket_skip.status, self.bucket_skip)
            return False
        return RunApp.checkRunnable(self, options)

    def __init__(self, name, params):
        RunApp.__init__(self, name, params)
        self.specs['cli_args'].append('-snes_type test')

    def processResults(self, moose_dir, options, output):
        m = re.search("Norm of matrix ratio (\S+?),? difference (\S+) \(user-defined state\)", output, re.MULTILINE | re.DOTALL);
        if m:
            if float(m.group(1)) < float(self.specs['ratio_tol']) and float(m.group(2)) < float(self.specs['difference_tol']):
                reason = ''
            else:
                reason = 'INCORRECT JACOBIAN'
        else:
            reason = 'EXPECTED OUTPUT NOT FOUND';

        # populate status bucket
        if reason != '':
            self.setStatus(reason, self.bucket_fail)
        else:
            self.setStatus(self.success_message, self.bucket_success)

        return output
