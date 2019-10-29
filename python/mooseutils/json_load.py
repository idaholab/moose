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
    if sys.version_info[0] == 2:
        tree = json.loads(raw, object_pairs_hook=deunicodify_hook)
    else:
        tree = json.loads(raw, object_pairs_hook=collections.OrderedDict)
    return tree
