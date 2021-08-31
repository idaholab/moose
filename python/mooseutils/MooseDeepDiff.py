#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import json
import logging
from deepdiff import DeepDiff


class MooseDeepDiff(DeepDiff):
    def __init__(self, *args, relative_error=None, absolute_error=None, **kwargs):
        self.rel_err = relative_error
        self.abs_err = absolute_error
        super().__init__(*args, **kwargs)

    def relative_error(self, x, y, max_relative_error):
        """ Determines whether a number has changed by calculating the relative error
            max_relative_error: the maximum relative tolerance that will be detected as a changed value
            Return True if the computed relative error is larger than the maximum relative error
            Return False if the computed relative error is smaller than the maximum relative error"""
        log = logging.getLogger(__name__)
        if y == 0.0:
            log.warning(
                'Division by zero: Using absolute error for single instance where x={0}, y={1}'.
                format(x, y))
            self.absolute_error(x, y, max_relative_error)
        else:
            relative_error = abs((x - y) / y)
            return relative_error > max_relative_error

    def absolute_error(self, x, y, max_absolute_error):
        """ Determines whether a number has changed by calculating the absolute error
            max_absolute_error: the maximum absolute tolerance that will be detected as a changed value
            Return True if the computed absolute error is larger than the maximum absolute error
            Return False if the computed absolute error is smaller than the maximum absolute error"""
        absolute_error = abs(x - y)
        return absolute_error > max_absolute_error

    def _diff_numbers(self, level):
        """Diff Numbers"""
        t1_type = "number" if self.ignore_numeric_type_changes else level.t1.__class__.__name__
        t2_type = "number" if self.ignore_numeric_type_changes else level.t2.__class__.__name__

        if self.rel_err is not None:
            if self.relative_error(level.t1, level.t2, self.rel_err):
                self._report_result('values_changed', level)
        elif self.abs_err is not None:
            if self.absolute_error(level.t1, level.t2, self.abs_err):
                self._report_result('values_changed', level)
        else:
            DeepDiff._diff_numbers(self, level)
