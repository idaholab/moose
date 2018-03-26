#pylint: disable=missing-docstring
import os
import collections

import mooseutils

#: tuple for storing "requirement" from test specification
Requirement = collections.namedtuple('Requirement', "name path filename requirement design issues")

def get_requirements(directories, specs):
    """
    Build requirements dictionary from the provided directories.
    """
    out = collections.defaultdict(set)
    for location in directories:
        for base, _, files in os.walk(location):
            for fname in files:
                if fname in specs:
                    _add_requirements(out, location, base, fname)
    return out

def _add_requirements(out, location, base, fname):
    """Load the test spec and add requirements."""
    full_file = os.path.join(base, fname)
    root = mooseutils.hit_load(full_file)
    group = root.children[0].get('group', u'Unknown')
    for child in root.children[0]:
        if ('requirement' in child) and ('design' in child) and ('issues' in child):
            req = Requirement(name=child.name,
                              path=os.path.relpath(base, location),
                              filename=full_file,
                              requirement=child['requirement'],
                              design=child['design'],
                              issues=child['issues'])
            out[group].add(req)
