#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import collections
import logging
import pyhit
import mooseutils
from .Requirement import Requirement

def get_requirements(directories, specs, prefix='F', category=None):
    """
    Build requirements dictionary from the provided directories.
    """
    out = collections.defaultdict(list)
    for location in directories:
        for filename in sorted(mooseutils.git_ls_files(location)):
            if not os.path.isfile(filename):
                continue
            fname = os.path.basename(filename)
            if fname in specs:
                _add_requirements(out, location, filename)

    for i, requirements in enumerate(out.values()):
        for j, req in enumerate(requirements):
            if category:
                req.label = "{}{}.{}.{}".format(prefix, category, i+1, j+1)
            else:
                req.label = "{}{}.{}".format(prefix, i+1, j+1)

    return out

def _add_requirements(out, location, filename):
    """
    Opens tests specification and extracts requirement items.
    """
    root = pyhit.load(filename)
    design = root.children[0].get('design', None)
    design_line = root.children[0].line('design', None)
    issues = root.children[0].get('issues', None)
    issues_line = root.children[0].line('issues', None)
    deprecated = root.children[0].get('deprecated', False)
    deprecated_line = root.children[0].line('deprecated', None)

    group = os.path.relpath(filename, location).split('/')[0]
    path = os.path.relpath(os.path.dirname(filename), location)

    for child in root.children[0]:
        req = _create_requirement(child, path, filename,
                                  design, design_line,
                                  issues, issues_line,
                                  deprecated, deprecated_line)
        out[group].append(req)

        # Get "detail" parameter from nested tests
        for grandchild in child.children:
            detail = _create_requirement(grandchild, path, filename, None, None, None, None, None, None)
            req.details.append(detail)

def _create_requirement(child, path, filename, design, design_line, issues, issues_line, deprecated, deprecated_line):

    # Create the Requirement object
    req = Requirement(name=child.name,
                      path=path,
                      filename=filename,
                      line=child.line())

    # "deprecated" parameter
    req.deprecated = child.get('deprecated', deprecated)
    req.deprecated_line = child.line('deprecated', deprecated_line)

    # "requirement" parameter
    req.requirement = child.get('requirement', None)
    req.requirement_line = child.line('requirement', None)

    # "design" parameter
    design = child.get('design', design if not req.deprecated else None)
    req.design = design.split() if (design is not None) else None
    req.design_line = child.line('design', design_line)

    # "issues" parameter
    issues = child.get('issues', issues if not req.deprecated else None)
    req.issues = issues.split() if (issues is not None) else None
    req.issues_line = child.line('issues', issues_line)

    # "detail" parameter
    req.detail = child.get('detail', None)
    req.detail_line = child.line('detail', None)

    # "skip" and "deleted" for creating satisfied parameter (i.e., does the test run)
    req.skip = child.get('skip', None) is not None
    req.deleted = child.get('deleted', None) is not None

    # V&V document
    verification = child.get('verification', None)
    req.verification = verification.split() if (verification is not None) else None
    req.verification_line = child.line('verification', None)

    validation = child.get('validation', None)
    req.validation = validation.split() if (validation is not None) else None
    req.validation_line = child.line('validation', None)

    # Store the prerequisites, if any
    prereq = child.get('prereq', None)
    if prereq is not None:
        req.prerequisites = set(prereq.split(' '))

    return req
