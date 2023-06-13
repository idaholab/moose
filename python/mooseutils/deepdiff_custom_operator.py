#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from deepdiff.operator import BaseOperator
class CompareDiff(BaseOperator):
    """ A derived DeepDiff class to allow custom comparison operations """
    def __init__(self, rel_err, abs_zero, types, regex_paths=None):
        """ Initialization routine """
        self.rel_err = rel_err
        self.abs_zero = abs_zero

        # While not used, next two members are necessary for deepdiff constructor to work
        self.types = types
        self.regex_paths = regex_paths

    def give_up_diffing(self, level, diff_instance):
        """
        Derived method called by deepdiff to allow more flexibility when making comparisons.
        This method returns False at the first opportunity of a diff.
        """
        level_t1 = []
        level_t2 = []
        # Split for XML diffing purposes
        if isinstance(level.t1, str) and isinstance(level.t2, str):
            level_t1.extend(level.t1.split(' '))
            level_t2.extend(level.t2.split(' '))
            # But if the lengths afterwards are different, we can't continue
            if len(level_t1) != len(level_t2):
                return False
        else:
            level_t1.append(level.t1)
            level_t2.append(level.t2)

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
                diff_instance.custom_report_result('diff', level, ('type difference '
                                                                   f'{type(result_computed)} != '
                                                                   f'{type(result_goldfile)}'))
                return False

            # Gold result is a string, but did not matched earlier. Meaning they are different
            # (the rest of this method's comparisons assume we are dealing with floats)
            if isinstance(result_goldfile, str):
                diff_instance.custom_report_result('diff', level, (f'{result_computed} != '
                                                                   f'{result_goldfile}'))
                return False

            # Convert results to floats
            result_goldfile = float(result_goldfile)
            result_computed = float(result_computed)

            if result_goldfile < self.abs_zero and result_computed < self.abs_zero:
                continue

            # difference between both results
            result_difference = max(result_goldfile,
                                    result_computed) - min(result_goldfile,
                                                           result_computed)

            # the biggest value of the two results
            max_ofresults = abs(max(result_goldfile, result_computed))

            if self.rel_err == 0 or result_computed == 0:
                if result_difference > self.rel_err:
                    exp_results = f'{result_difference} > {self.rel_err}'
                    diff_instance.custom_report_result('diff', level, exp_results)
                    return False

                elif abs(result_difference / max_ofresults) > self.rel_err:
                    exp_results = f'{abs(result_difference/max_ofresults)} > {self.rel_err}'
                    diff_instance.custom_report_result('diff', level, exp_results)
                    return False
        return True
