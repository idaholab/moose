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
        color = kwargs.pop('color', True)
        self._skip_keys = kwargs.pop('skip_keys', [])

        self._data0 = self._load(input0)
        self._data1 = self._load(input1)

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
    obj = JSONDiff(*args.files)
    print(obj)
