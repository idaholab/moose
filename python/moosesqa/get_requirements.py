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
import moosetree
import moosesqa
from .Requirement import TestSpecification, Requirement, Detail

def get_requirements_from_tests(directories, specs, include_non_testable=False):
    """
    Build requirements dictionary from the provided directories.

    Input:
        directories[list]: A list of directories to consider
        specs[list]: A list of test specification names (e.g., ['tests'])
    """
    out = collections.defaultdict(list)
    for location in directories:
        root_dir = mooseutils.git_root_dir(location)
        for filename in sorted(mooseutils.git_ls_files(location)):
            if os.path.isfile(filename) and (os.path.basename(filename) in specs):
                local = os.path.relpath(filename, location)
                group = local.split('/')[0]
                out[group] += get_requirements_from_file(filename,
                                                         os.path.dirname(local),
                                                         include_non_testable,
                                                         root_dir)
    return out

def number_requirements(requirement_dict, category):
    """
    Apply a number label to the requirements.

    Input:
        requirement_dict[dict]: Container of Requirement objects, as returned from
                                get_requirement_from_tests.
        category[int]: Category index to apply to label.

    The format of the number is <category>.<group>.<number>, e.g., 3.2.1. The group
    is the indexed according to the supplied dict keys.

    IMPORTANT: These numbers are not designed to be referenced in any manner, they are simply applied
               for organizational purposes.
    """
    for i, requirements in enumerate(requirement_dict.values()):
        for j, req in enumerate(requirements):
            req.label = "{}.{}.{}".format(category, i+1, j+1)

def get_requirements_from_file(filename, prefix=None, include_non_testable=False, root_dir=None):
    """
    Opens hit file and extracts requirement items.

    Input:
        filename[str]: The HIT file to open and extract Requirements

    Returns:
         A list of Requirement objects.
    """
    if not os.path.isfile(filename):
        raise FileNotFoundError("The supplied filename does not exist: {}".format(filename))
    requirements = list()
    root = pyhit.load(filename)

    # Options available at the top-level
    #   [Tests]
    #      design = 'foo.md bar.md'
    #      issues = '#12345 ab23bd34'
    #      verification = 'ver-foo.md'
    #      validation = 'val-bar.md'
    design = root.children[0].get('design', None)
    design_line = root.children[0].line('design', None)
    issues = root.children[0].get('issues', None)
    issues_line = root.children[0].line('issues', None)
    verification = root.children[0].get('verification', None)
    verification_line = root.children[0].line('verification', None)
    validation = root.children[0].get('validation', None)
    validation_line = root.children[0].line('validation', None)
    deprecated = root.children[0].get('deprecated', False)
    deprecated_line = root.children[0].line('deprecated', None)
    collections = root.children[0].get('collections', None)
    collections_line = root.children[0].line('collections', None)

    for child in root.children[0]:
        req = _create_requirement(child, filename,
                                  design, design_line,
                                  issues, issues_line,
                                  verification, verification_line,
                                  validation, validation_line,
                                  collections, collections_line,
                                  deprecated, deprecated_line)
        req.prefix = prefix

        # Get "detail" parameter from nested tests
        for grandchild in child.children:
            detail = _create_detail(grandchild, filename)
            detail.specification = _create_specification(grandchild, '{}/{}'.format(child.name, grandchild.name), filename, root_dir)
            req.details.append(detail)

        if not req.details:
            req.specification = _create_specification(child, child.name, filename, root_dir)

        if req.testable or include_non_testable:
            requirements.append(req)

    return requirements

def get_test_specification(filename, block):
    """
    Create a TestSpecification object from the HIT file and block name.

    Input:
        filename[str]: Complete filename of a HIT file containing test specification(s)
        block[str]: The name of the block to use for creating the TestSpecification object

    This function exists to allow for requirements to be defined outside of the test specifications,
    but still reference tests for the purpose of SQA traceability. Support for this was added to
    allow for non-functional requirements to be defined outside of the test specifications.
    """
    root = pyhit.load(filename)

    # Locate the desired block
    node = moosetree.find(root, lambda n: n.fullpath.endswith(block))
    if node is None:
        raise KeyError("Unable to locate '{}' in {}".format(block, filename))

    # Build/return TestSpecification object
    name = node.name if node.parent.parent.is_root else '{}/{}'.format(node.parent.name, node.name)
    return _create_specification(node, name, filename, mooseutils.git_root_dir(os.path.dirname(filename)))

def _find_file(working_dir, pattern):
    """
    Helper for finding file in repository.

    see _create_specification
    """
    if pattern.startswith('/'):
        pattern = os.path.join(working_dir, pattern.strip('/'))

    matches = [f for f in mooseutils.git_ls_files(working_dir) if f.endswith(pattern)]

    if not matches:
        raise NameError("Unable to locate a test specification with pattern: {}".format(pattern))
    elif len(matches) > 1:
        msg = "Located multiple test specifications with pattern: {}\n".format(pattern)
        msg += "    \n".join(matches)
        raise NameError(msg)

    return matches[0]

def _create_specification(child, name, filename, root_dir):
    """
    Create and return a TestSpecificaiton object.

    Inputs:
        child[pyhit.Node]: Node containing test specification
        name: The name to apply to the specification
        filename: Location of the specification
    """
    spec = TestSpecification(name=name, filename=filename, line=child.line())
    spec.type = child.get('type').strip() if child.get('type', None) is not None else None
    spec.text = child.render()
    spec.local = os.path.relpath(filename, root_dir)

    # "skip" and "deleted" for creating satisfied parameter (i.e., does the test run)
    spec.skip = child.get('skip', None) is not None
    spec.deleted = child.get('deleted', None) is not None

    # Store the prerequisites, if any
    prereq = child.get('prereq', None)
    if prereq is not None:
        spec.prerequisites = set(prereq.split(' '))

    return spec

def _create_requirement(child, filename, design, design_line, issues, issues_line,
                        verification, verification_line, validation, validation_line,
                        collections, collections_line, deprecated, deprecated_line):

    # Create the Requirement object
    req = Requirement(name=child.name,
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
    verification = child.get('verification', verification if (verification is not None) else None)
    req.verification = verification.split() if (verification is not None) else None
    req.verification_line = child.line('verification', verification_line)

    validation = child.get('validation', validation if (validation is not None) else None)
    req.validation = validation.split() if (validation is not None) else None
    req.validation_line = child.line('validation', validation_line)

    # "detail" parameter (this will error in check_requirements)
    req.detail = child.get('detail', None)
    req.detail_line = child.line('detail', None)
    return req

def _create_detail(child, filename):
    req = Detail(name=child.name,
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
