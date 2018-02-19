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
import math

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

    def __strToFloat(self, str):
        """ Convert string to float """
        # PETSc returns 'nan.' and 'inf.', so in order to convert it properly, we need to strip the trailing '.'
        if str == 'nan.' or str == 'inf.':
            return float(str[:-1])
        # And '-nan.' is also a possible result from PETSc
        elif str == '-nan.':
            return float('nan')
        else:
            return float(str)

    def __compare(self, value, threshold):
        """
        return True if:
          1. `value` and `threshold` are both Inf
          2. `value` and `threshold` are both NaN
          3. `value` is less then `threshold`
        otherwise False
        """
        if (math.isnan(value) and math.isnan(float(threshold))):
            return True
        elif (math.isinf(value) and math.isinf(float(threshold))):
            return True
        elif value < float(threshold):
            return True
        else:
            return False

    def processResults(self, moose_dir, options, output):
        m = re.search("Norm of matrix ratio (\S+?),? difference (\S+) \(user-defined state\)", output, re.MULTILINE | re.DOTALL);
        if m:
            if self.__compare(self.__strToFloat(m.group(1)), self.specs['ratio_tol']) and \
               self.__compare(self.__strToFloat(m.group(2)), self.specs['difference_tol']):
                reason = ''
            else:
                reason = 'INCORRECT JACOBIAN'
        else:
            reason = 'EXPECTED OUTPUT NOT FOUND'

        # populate status bucket
        if reason != '':
            self.setStatus(reason, self.bucket_fail)
        else:
            self.setStatus(self.success_message, self.bucket_success)

        return output
