#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import re
import math

from moosetools.moosetest.base import Differ


class PETScJacobianDiffer(Differ):
    """
    Analyzes output from MOOSE application that includes PETSc Jacobian debugging information.

    This `Differ` object is designed to operate with the legacy `PetscJacobianTester`.
    """
    @staticmethod
    def validParams():
        params = Differ.validParams()
        params.add('ratio_tol', vtype=float, default=1e-8, doc="Relative tolerance to compare the ration against.")
        params.add('difference_tol', vtype=float, default=1e-8, doc="Relative tolerance to compare the difference against.")
        params.add('only_final_jacobian', vtype=bool, default=False,
                   doc="Only check the final Jacobian comparison.")


        # TODO: These are probably going to be Controller based options, thus then need to be
        #       overridden by an object. The override can't happen here, it will need to be in the
        #       constructor because the Controller params are added after calling validParams
        #       (That could be changed if this is cleaner in the end, which is looking better to me))
        #
        #        if 'foo' in params:
        #            params.setValue('foo', 'valgrind', None)
        #
        #        -or-
        #
        #        def __init__(...):
        #           self.setControllerValue(...)
        #
        # override default values
        #params.valid['valgrind'] = 'NONE'
        #params.valid['petsc_version'] = ['>=3.9.4']
        #params.valid['method'] = ['OPT']

        return params

    def execute(self, rcode, text):

        ratio_tol = self.getParam('ratio_tol')
        diff_tol = self.getParam('difference_tol')
        only_final = self.getParam('only_final_jacobian')

        matches = re.finditer(r"\|\|J - Jfd\|\|_F/\|\|J\|\|_F\s?=?\s?(\S+), \|\|J - Jfd\|\|_F\s?=?\s?(\S+)",
                              text, re.MULTILINE | re.DOTALL)

        if not matches:
            self.error("Expected PETSc output for Jacobian comparison not located in:\n{}", text)

        for match in matches:
            self.__compare(self.__strToFloat(match.group(1)), ratio_tol)
            self.__compare(self.__strToFloat(match.group(2)), diff_tol)
            if only_final:
                break

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
        Error if all of the following conditions are not :
          1. `value` and `threshold` are both Inf
          2. `value` and `threshold` are both NaN
          3. `value` is less then `threshold`
        otherwise False
        """
        if (math.isnan(value) and math.isnan(threshold)):
            pass
        elif (math.isinf(value) and math.isinf(threshold)):
            pass
        elif value < threshold:
            pass
        else:
            self.error("The Jacobian entry is incorrect: {} >= {}", value, threshold)
