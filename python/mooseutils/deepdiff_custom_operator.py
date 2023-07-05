#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

""" A derived DeepDiff class to allow custom comparison operations """

import math
from deepdiff.operator import BaseOperator
class CompareDiff(BaseOperator):
    """ A derived DeepDiff class to allow custom comparison operations """
    #pylint: disable=too-many-arguments,super-init-not-called
    def __init__(self, rel_err, abs_err, abs_zero, types, regex_paths=None):
        """ Initialization routine """
        self.relative_tolerance = rel_err
        self.absolute_tolerance = abs_err
        self.floor = abs_zero
        self.types = types

        # While not used, is necessary for deepdiff constructor to work
        self.regex_paths = regex_paths

    def match(self, level) -> bool:
        """ Return bool if CompareDiff is allowed to provide comparisons """
        if type(level.t1) in self.types:
            return True
        return False

    @staticmethod
    def do_type(reference, result) -> bool:
        """ Return bool if both reference and result are the same type """
        return isinstance(reference, type(result))

    @staticmethod
    def do_floor(reference, result, floor) -> bool:
        """ Return bool if both reference and result are below floor """
        return (max(abs(reference), floor) == floor and max(abs(result), floor) == floor)

    @staticmethod
    def do_absolute(reference, result, tolerance) -> bool:
        """ Return bool if both reference and result differences are below absolute tolerances """
        return max(abs(reference - result), tolerance) == tolerance

    @staticmethod
    def do_relative(reference, result, tolerance) -> bool:
        """ Return bool if both reference and result differences are below relative tolerances """
        return max(abs(abs(reference - result) / reference), tolerance) == tolerance

    def give_up_diffing(self, level, diff_instance) -> bool:
        """
        Derived method called by deepdiff to allow more flexibility when making comparisons.

        To use, set up a test (see do_* methods above), and if your test passes return True with
        without applying a diff_instance. If your test failes, apply a diff_instance (see
        examples below, and also return True).

        Returning True from this mehtod tells deepdiff to stop trying to determin a diff.
        """
        # Split for XML diffing purposes
        if isinstance(level.t1, str) and isinstance(level.t2, str):
            # But if the lengths afterwards are different, we can't continue
            if len(level.t1.split(' ')) != len(level.t2.split(' ')):
                diff_instance.custom_report_result('diff', level, ('field length difference '
                                                                   f'{len(level.t1)} != '
                                                                   f'{len(level.t2)}'))
                return True

        result_goldfile = level.t1
        result_computed = level.t2

        # Convert to floats, else let it go. We will catch gold vs computed type mismatchs or
        # a str != str difference later.
        try:
            result_goldfile = float(result_goldfile)
            result_computed = float(result_computed)
        except ValueError:
            pass

        # Identical match
        if result_goldfile == result_computed:
            return True

        # Results not of the same type
        if not self.do_type(result_goldfile, result_computed):
            diff_instance.custom_report_result('diff', level, ('gold != result type difference: '
                                                               f'{type(result_computed)} != '
                                                               f'{type(result_goldfile)}'))
            return True

        # Gold result is a string, but did not match earlier. Meaning they are different
        if isinstance(result_goldfile, str):
            diff_instance.custom_report_result('diff', level, (f'{result_computed} != '
                                                                f'{result_goldfile}'))
            return True

        # Corner cases have been discovered. The following routines will detect minor differences
        # based on supplied floating point tolerances/floors

        # handle NaN and divide by zero errors by setting results to absolute zero (the floor)
        if math.isnan(result_goldfile) or result_goldfile == 0:
            result_goldfile = self.floor
        if math.isnan(result_computed) or result_computed == 0:
            result_computed = self.floor

        # both reference and result are below or at the floor
        if self.do_floor(result_goldfile, result_computed, self.floor):
            return True

        # test for absolute difference (relative tolerances need to be zero to enable test)
        if self.relative_tolerance == 0:
            if not self.do_absolute(result_goldfile, result_computed, self.absolute_tolerance):
                custom_report = f'absolute diff: {abs(result_goldfile - result_computed):.8e}'
                diff_instance.custom_report_result('diff', level, custom_report)

        # test for relative difference
        elif not self.do_relative(result_goldfile, result_computed, self.relative_tolerance):
            relative = abs(abs(result_goldfile - result_computed) / result_goldfile)
            custom_report = f'relative diff: {relative:.8e}'
            diff_instance.custom_report_result('diff', level, custom_report)

        return True
