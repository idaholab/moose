#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import collections
import sys
import json

def deunicodify_hook(pairs):
    new_pairs = []
    for key, value in pairs:
        if isinstance(value, unicode):
            value = value.encode('utf-8')
        if isinstance(key, unicode):
            key = key.encode('utf-8')
        new_pairs.append((key, value))
    return collections.OrderedDict(new_pairs)

def json_load(filename):
    with open(filename, 'r') as fid:
        raw = fid.read()
    return json_parse(raw)

def json_parse(raw):
    tree = json.loads(raw, object_pairs_hook=collections.OrderedDict)
    return tree
