# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

from RunApp import RunApp
import re
import math
from TestHarness import util
import os

class PetscJacobianTester(RunApp):
    @staticmethod
    def validParams():
        params = RunApp.validParams()
        params.addParam('ratio_tol', 1e-7, "Relative tolerance to compare the ration against.")
        params.addParam('difference_tol', 1e0, "Relative tolerance to compare the difference against.")
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
        params['recover'] = False
        params['restep'] = False
        return params

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

    def processResults(self, moose_dir, options, exit_code, runner_output):
        output = ''

        matches = re.finditer(r"\|\|J - Jfd\|\|_F/\|\|J\|\|_F\s?=?\s?(\S+), \|\|J - Jfd\|\|_F\s?=?\s?(\S+)",
                runner_output, re.MULTILINE | re.DOTALL)

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
