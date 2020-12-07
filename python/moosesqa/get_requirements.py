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
from .Requirement import TestSpecification, Requirement, Detail

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

    # Options available at the top-level
    #   [Tests]
    #      design = 'foo.md bar.md'
    #      issues = '#12345 ab23bd34'
    design = root.children[0].get('design', None)
    design_line = root.children[0].line('design', None)
    issues = root.children[0].get('issues', None)
    issues_line = root.children[0].line('issues', None)
    deprecated = root.children[0].get('deprecated', False)
    deprecated_line = root.children[0].line('deprecated', None)
    collections = root.children[0].get('collections', None)
    collections_line = root.children[0].line('collections', None)

    group = os.path.relpath(filename, location).split('/')[0]
    path = os.path.relpath(os.path.dirname(filename), location)
    for child in root.children[0]:
        req = _create_requirement(child, path, filename,
                                  design, design_line,
                                  issues, issues_line,
                                  collections, collections_line,
                                  deprecated, deprecated_line)
        out[group].append(req)

        # Get "detail" parameter from nested tests
        for grandchild in child.children:
            detail = _create_detail(grandchild, path, filename)
            detail.specification = _create_specification(grandchild, '{}/{}'.format(child.name, grandchild.name), path, filename)
            req.details.append(detail)

        if not req.details:
            req.specification = _create_specification(child, child.name, path, filename)

def _create_specification(child, name, path, filename):
    spec = TestSpecification(name=name,
                             path=path,
                             filename=filename,
                             line=child.line())
    # "skip" and "deleted" for creating satisfied parameter (i.e., does the test run)
    spec.skip = child.get('skip', None) is not None
    spec.deleted = child.get('deleted', None) is not None

    # Store the prerequisites, if any
    prereq = child.get('prereq', None)
    if prereq is not None:
        spec.prerequisites = set(prereq.split(' '))

    # Type
    spec.type = child.get('type').strip() if child.get('type', None) is not None else None

    return spec


def _create_requirement(child, path, filename, design, design_line, issues, issues_line,
                        collections, collections_line, deprecated, deprecated_line):

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

    # "collections" parameter
    collections = child.get('collections', collections if (collections is not None) else None)
    req.collections = set(collections.strip().split()) if (collections is not None) else None
    req.collections_line = child.line('collections', collections_line)

    # V&V document
    verification = child.get('verification', None)
    req.verification = verification.strip().split() if (verification is not None) else None
    req.verification_line = child.line('verification', None)

    validation = child.get('validation', None)
    req.validation = validation.strip().split() if (validation is not None) else None
    req.validation_line = child.line('validation', None)

    # "detail" parameter (this will error in check_requirements)
    req.detail = child.get('detail', None)
    req.detail_line = child.line('detail', None)
    return req

def _create_detail(child, path, filename):
    req = Detail(name=child.name,
                 path=path,
                 filename=filename,
                 line=child.line())

    req.detail = child.get('detail', None)
    req.detail_line = child.line('detail', None)

    # "requirement" parameter  (this will error in check_requirements)
    req.requirement = child.get('requirement', None)
    req.requirement_line = child.line('requirement', None)
    req.design = child.get('design', None)
    req.design_line = child.line('design', None)
    req.issues = child.get('issues', None)
    req.issues_line = child.line('issues', None)
    req.deprecated = child.get('deprecated', None)
    req.deprecated_line = child.line('deprecated', None)
    req.collections = child.get('collections', None)
    req.collections_line = child.line('collections', None)
    return req
