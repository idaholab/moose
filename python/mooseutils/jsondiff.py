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
import mooseutils

def parse_args():
    parser = argparse.ArgumentParser(description='Tool for comparing two JSON files')
    parser.add_argument('files', nargs=2, type=argparse.FileType('r'))
    return parser.parse_args()

class JSONDiffer(object):
    """Basic class for performing diff between JSON files/data"""

    def __init__(self, input0, input1, **kwargs):
        kwargs.setdefault('sort_keys', True)
        kwargs.setdefault('indent', 4)
        skip = kwargs.pop('skip', ['current_time', 'executable', 'executable_time', 'moose_version'])
        color = kwargs.pop('color', True)

        self._data0 = self._load(input0)
        self._data1 = self._load(input1)

        for key in skip:
            if isinstance(self._data0, dict) and key in self._data0:
                self._data0.pop(key)
            if isinstance(self._data1, dict) and key in self._data1:
                self._data1.pop(key)

        self._diff = mooseutils.text_unidiff(json.dumps(self._data0, **kwargs),
                                             json.dumps(self._data1, **kwargs),
                                             out_fname=input0 if os.path.isfile(input0) else None,
                                             gold_fname=input1 if os.path.isfile(input1) else None,
                                             color=color)

    def fail(self):
        return bool(self._diff)

    def message(self):
        return self._diff

    def __str__(self):
        return self._diff

    @staticmethod
    def _load(input0):
        if os.path.isfile(input0):
            with open(input0, 'r', encoding='utf-8') as fid:
                return json.load(fid, object_pairs_hook=collections.OrderedDict)
        elif isinstance(input0, str):
            return json.loads(input0, object_pairs_hook=collections.OrderedDict)
        return input0


if __name__ == '__main__':
    args = parse_args()
    obj = JSONDiff(*args.files)
    print(obj)
