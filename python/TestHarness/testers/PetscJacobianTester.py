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
from TestHarness import util
import os

class PetscJacobianTester(RunApp):
    @staticmethod
    def validParams():
        params = RunApp.validParams()
        params.addParam('ratio_tol', 1e-8, "Relative tolerance to compare the ration against.")
        params.addParam('difference_tol', 1e-6, "Relative tolerance to compare the difference against.")
        params.addParam('state', 'user', "The state for which we want to compare against the "
                                         "finite-differenced Jacobian ('user', 'const_positive', or "
                                         "'const_negative'.")
        params.addParam('run_sim', False, "Whether to actually run the simulation, testing the Jacobian "
                                          "at every non-linear iteration of every time step. This is only "
                                          "relevant for petsc versions >= 3.9.")
        params.addParam('turn_off_exodus_output', True, "Whether to set exodus=false in Outputs")
        params.addParam('only_final_jacobian', False, "Check only final Jacobian comparison.")

        # override default values
        params.valid['valgrind'] = 'NONE'
        params.valid['petsc_version'] = ['>=3.9.4']
        params.valid['method'] = ['OPT']

        return params

    def checkRunnable(self, options):
        if options.enable_recover:
            self.addCaveats('PetscJacTester RECOVER')
            self.setStatus(self.skip)
            return False
        return RunApp.checkRunnable(self, options)

    def __init__(self, name, params):
        RunApp.__init__(self, name, params)

        self.moose_dir = os.environ.get('MOOSE_DIR',
                                        os.path.abspath(os.path.join(os.path.dirname(os.path.realpath(__file__)),
                                                                     '..')))

        if os.environ.get("LIBMESH_DIR"):
            self.libmesh_dir = os.environ['LIBMESH_DIR']
        else:
            self.libmesh_dir = os.path.join(self.moose_dir, 'libmesh', 'installed')

        if self.specs['turn_off_exodus_output']:
            self.specs['cli_args'][:0] = ['Outputs/exodus=false']

        if list(map(int, util.getPetscVersion(self.libmesh_dir).split("."))) < [3, 9]:
            self.old_petsc = True
            self.specs['cli_args'].extend(['-snes_type test', '-snes_mf_operator 0'])
        else:
            self.old_petsc = False
            self.specs['cli_args'].extend(['-snes_test_jacobian', '-snes_force_iteration'])
            if not self.specs['run_sim']:
                self.specs['cli_args'].extend(['-snes_type', 'ksponly',
                                  '-ksp_type', 'preonly', '-pc_type', 'none', '-snes_convergence_test', 'skip'])

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
        if self.old_petsc:
            if self.specs['state'].lower() == 'user':
                m = re.search("Norm of matrix ratio (\S+?),? difference (\S+) \(user-defined state\)",
                              output, re.MULTILINE | re.DOTALL);
            elif self.specs['state'].lower() == 'const_positive':
                m = re.search("Norm of matrix ratio (\S+?),? difference (\S+) \(constant state 1\.0\)",
                              output, re.MULTILINE | re.DOTALL);
            elif self.specs['state'].lower() == 'const_negative':
                m = re.search("Norm of matrix ratio (\S+?),? difference (\S+) \(constant state -1\.0\)",
                              output, re.MULTILINE | re.DOTALL);
            else:
                self.setStatus("state must be either 'user', const_positive', or 'const_negative'",
                               self.bucket_fail)
                return output

            if m:
                if self.__compare(self.__strToFloat(m.group(1)), self.specs['ratio_tol']) and \
                   self.__compare(self.__strToFloat(m.group(2)), self.specs['difference_tol']):
                    reason = ''
                else:
                    reason = 'INCORRECT JACOBIAN'
            else:
                reason = 'EXPECTED OUTPUT NOT FOUND'

        else:
            matches = re.finditer("\|\|J - Jfd\|\|_F/\|\|J\|\|_F\s?=?\s?(\S+), \|\|J - Jfd\|\|_F\s?=?\s?(\S+)",
                  output, re.MULTILINE | re.DOTALL)

            reason = 'EXPECTED OUTPUT NOT FOUND'
            for match in matches:
                if self.__compare(self.__strToFloat(match.group(1)), self.specs['ratio_tol']) and \
                   self.__compare(self.__strToFloat(match.group(2)), self.specs['difference_tol']):
                    reason = ''
                else:
                    reason = 'INCORRECT JACOBIAN'
                    if str(self.specs['only_final_jacobian']).lower()=="false":
                        break

        if reason != '':
            self.setStatus(self.fail, reason)

        return output
