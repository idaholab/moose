#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import math
from deepdiff.operator import BaseOperator
class CompareDiff(BaseOperator):
    """ A derived DeepDiff class to allow custom comparison operations """
    def __init__(self, rel_err, abs_err, abs_zero, types, regex_paths=None):
        """ Initialization routine """
        self.relative_tolerance = rel_err
        self.absolute_tolerance = abs_err
        self.floor = abs_zero

        # While not used, next two members are necessary for deepdiff constructor to work
        self.types = types
        self.regex_paths = regex_paths

    def give_up_diffing(self, level, diff_instance):
        """
        Derived method called by deepdiff to allow more flexibility when making comparisons.
        This method returns at the first opportunity of a diff.
        """
        level_t1 = []
        level_t2 = []
        # Split for XML diffing purposes
        if isinstance(level.t1, str) and isinstance(level.t2, str):
            level_t1.extend(level.t1.split(' '))
            level_t2.extend(level.t2.split(' '))
            # But if the lengths afterwards are different, we can't continue
            if len(level_t1) != len(level_t2):
                diff_instance.custom_report_result('diff', level, ('field length difference '
                                                                   f'{len(level_t1)} != '
                                                                   f'{len(level_t2)}'))
                return True
        else:
            level_t1.append(level.t1)
            level_t2.append(level.t2)

        custom_report = ''
        for i_index, value in enumerate(level_t1):
            result_goldfile = level_t1[i_index]
            result_computed = level_t2[i_index]

            # Convert to floats, else let it go. We will catch gold vs computed type mismatchs or
            # a str != str difference later.
            try:
                result_goldfile = float(result_goldfile)
                result_computed = float(result_computed)
            except ValueError:
                pass

            # Identical match
            if result_goldfile == result_computed:
                continue

            # Results not of the same type
            if not isinstance(result_computed, type(result_goldfile)):
                diff_instance.custom_report_result('diff', level, ('result type difference: '
                                                                   f'{type(result_computed)} != '
                                                                   f'{type(result_goldfile)}'))
                return True

            # Gold result is a string, but did not matched earlier. Meaning they are different
            if isinstance(result_goldfile, str):
                diff_instance.custom_report_result('diff', level, (f'{result_computed} != '
                                                                   f'{result_goldfile}'))
                return True

            # Corner cases have been discovered above. The following routines will detect minor
            # differences based on supplied floating point tolerances/floors

            # handle NaN and divide by zero errors by setting results to absolute zero (the floor)
            if math.isnan(result_goldfile) or result_goldfile == 0:
                result_goldfile = self.floor
            if math.isnan(result_computed) or result_computed == 0:
                result_computed = self.floor

            # both reference and result are below or at the floor (pass)
            if (max(abs(result_goldfile), self.floor) == self.floor and
                max(abs(result_computed), self.floor) == self.floor):
                continue

            # compute absolute difference
            absolute_diff = abs(result_goldfile - result_computed)

            # compute relative difference
            relative_diff = abs(absolute_diff / result_goldfile)

            # test for absolute difference
            if (self.relative_tolerance == 0 and
                max(absolute_diff, self.absolute_tolerance) == self.absolute_tolerance):
                continue
            elif self.relative_tolerance == 0:
                custom_report = f'absolute diff: {absolute_diff:.8e}'
                break

            # test for relative difference
            if max(relative_diff, self.relative_tolerance) == self.relative_tolerance:
                continue
            else:
                custom_report = f'relative diff: {relative_diff:.8e}'
                break

        if custom_report:
            diff_instance.custom_report_result('diff', level, custom_report)

        return True
