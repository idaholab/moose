#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import os
import argparse
import json
import collections
import logging
from deepdiff import DeepDiff
from textwrap import indent

def parse_args():
    parser = argparse.ArgumentParser(description='Tool for comparing two JSON files')
    parser.add_argument('files', nargs=2)
    parser.add_argument('--rel_err', type=float, default=0.0, help="Relative error value used in exodiff comparisons.")
    parser.add_argument('--abs_zero', type=float, default=None, help='Value representing an absolute zero (default: 0)')
    parser.add_argument('--skip_keys', default=[], nargs='+', type=str, help="A list of keys to skip in the JSON comparison.")
    return parser.parse_args()

class MooseDeepDiff(DeepDiff):
    def __init__(self, *args, relative_error=None, absolute_error=None, absolute_zero=None, **kwargs):
        self.rel_err = relative_error
        self.abs_err = absolute_error
        self.abs_zero = absolute_zero
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

    def _diff_numbers(self, level, **kwargs):
        """Diff Numbers"""
        t1_type = "number" if self.ignore_numeric_type_changes else level.t1.__class__.__name__
        t2_type = "number" if self.ignore_numeric_type_changes else level.t2.__class__.__name__

        x = level.t1
        y = level.t2
        if self.abs_zero is not None:
            x = 0 if abs(x) < self.abs_zero else x
            y = 0 if abs(y) < self.abs_zero else y

        if self.rel_err is not None:
            if self.relative_error(x, y, self.rel_err):
                self._report_result('values_changed', level)
        elif self.abs_err is not None:
            if self.absolute_error(x, y, self.abs_err):
                self._report_result('values_changed', level)
        else:
            DeepDiff._diff_numbers(self, level, **kwargs)

class JSONDiffer(object):
    """Basic class for performing diff between JSON files/data"""

    def __init__(self, input0, input1, **kwargs):
        kwargs.setdefault('sort_keys', True)
        self._indent = kwargs.pop('indent', 4) * ' '
        color = kwargs.pop('color', True)
        self._skip_keys = kwargs.pop('skip_keys', [])

        self._data0 = self._load(input0)
        self._data1 = self._load(input1)

        self._diff = MooseDeepDiff(self._data0, self._data1, relative_error=kwargs.pop('relative_error', 0.0), absolute_zero=kwargs.pop('absolute_zero', None))

    def fail(self):
        return bool(self._diff)

    def message(self):
        return indent('Files are the same' if not self.fail() else self._diff.pretty(), self._indent)

    def __str__(self):
        return indent('Files are the same' if not self.fail() else self._diff.pretty(), self._indent)

    def _load(self, input0):
        if os.path.isfile(input0):
            with open(input0, 'r', encoding='utf-8') as fid:
                return json.load(fid, object_hook=self._skip)
        elif isinstance(input0, str):
            return json.loads(input0, object_hook=self._skip)
        return input0

    def _skip(self, data):
        return collections.OrderedDict({k:v for k, v in data.items() if k not in self._skip_keys})

if __name__ == '__main__':
    args = parse_args()
    obj = JSONDiffer(*args.files, relative_error=args.rel_err, absolute_zero=args.abs_zero, skip_keys=args.skip_keys)
    print(obj)
