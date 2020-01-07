#pylint: disable=missing-docstring
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

import mooseutils

LOG = logging.getLogger(__name__)

class Requirement(object):
    """struct for storing Requirement information."""
    def __init__(self,
                 name=None,
                 path=None,
                 filename=None,
                 text=None,
                 text_line=None,
                 design=None,
                 design_line=None,
                 issues=None,
                 issues_line=None,
                 satisfied=True):
        self.name = name
        self.path = path
        self.filename = filename
        self.text = text
        self.text_line = text_line
        self.design = design
        self.design_line = design_line
        self.issues = issues
        self.issues_line = issues_line
        self.label = None # added by get_requirements function
        self.satisfied = satisfied
        self.prerequisites = []
        self.details = []
        self.verification = None
        self.validation = None

    def __str__(self):
        frmt = '{}:\n    Text: {}\n    Design: {}\n    Issues: {}'
        return frmt.format(self.name, self.text, repr(self.design), repr(self.issues))


class Detail(object):
    """struct for storing Requirement detail information"""
    def __init__(self,
                 name=None,
                 path=None,
                 filename=None,
                 text=None,
                 text_line=None):
        self.name = name
        self.path = path
        self.filename = filename
        self.text = text
        self.text_line = text_line

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

def _check_extra_param(node, param_name, filename):
    """Helper function for error messages in sub-blocks."""
    param = node.get(param_name, None)
    param_line = node.line(param_name, 0)
    if param is not None:
        msg = "%s:%s\n" \
              "The '%s' parameter is not allowed within sub-blocks, all issues must " \
              "must be provided in the top-level of the group."
        LOG.error(msg, filename, param_line, param_name)

def _add_requirements(out, location, filename):
    """Opens tests specification and extracts requirement items."""
    root = mooseutils.hit_load(filename)
    design = root.children[0].get('design', None)
    design_line = root.children[0].line('design', None)
    issues = root.children[0].get('issues', None)
    issues_line = root.children[0].line('issues', None)
    deprecated = root.children[0].get('deprecated', False)
    deprecated_line = root.children[0].line('deprecated', None)

    for child in root.children[0]:
        if 'requirement' in child:
            requirement_line = child.line('requirement')

            # Deprecation
            local_deprecated = child.get('deprecated', deprecated)
            local_deprecated_line = child.line('deprecated', deprecated_line)
            if local_deprecated:
                msg = "%s:%s\nThe 'requirement' parameter is specified for %s, but the test is " \
                      "marked as deprecated on line %s."
                LOG.error(msg, filename, requirement_line, child.name, local_deprecated_line)
                continue

            # Deprecation
            local_deprecated = child.get('deprecated', deprecated)
            local_deprecated_line = child.line('deprecated', deprecated_line)
            if local_deprecated:
                msg = "The 'requirment' parameter is specified for %s, but the test is marked as " \
                      "deprecated on line %s."
                LOG.error(msg, child.name, local_deprecated_line)
                continue

            local_design = child.get('design', design)
            local_design_line = child.line('design', design_line)
            if local_design is None:
                msg = "The 'design' parameter is missing from '%s' in %s. It must be defined at " \
                      "the top level and/or within the individual test specification. It " \
                      "should contain a space separated list of filenames."
                LOG.error(msg, child.name, filename)
                local_design = ''

            local_issues = child.get('issues', issues)
            local_issues_line = child.line('issues', issues_line)
            if local_issues is None:
                msg = "The 'issues' parameter is missing from '%s' in %s. It must be defined at " \
                      "the top level and/or within the individual test specification. It " \
                      "should contain a space separated list of issue numbers (include the #) or " \
                      "a git commit SHA."
                LOG.error(msg, child.name, filename)
                local_issues = ''

            text = child['requirement']
            if 'MOOSE_TEST_NAME' in text:
                idx = filename.index('/tests/')
                name = os.path.join(os.path.dirname(filename[idx+7:]), child.name)
                text = text.replace('MOOSE_TEST_NAME', name)

            satisfied = False if (child['skip'] or child['deleted']) else True
            req = Requirement(name=child.name,
                              path=os.path.relpath(os.path.dirname(filename), location),
                              filename=filename,
                              text=str(text),
                              text_line=child.line('requirement', None),
                              design=local_design.split(),
                              design_line=local_design_line,
                              issues=local_issues.split(),
                              issues_line=local_issues_line,
                              satisfied=satisfied)

            req.verification = child.get('verification', None)
            req.validation = child.get('validation', None)

            group = os.path.relpath(filename, location).split('/')[0]

            prereq = child.get('prereq', None)
            if prereq is not None:
                req.prerequisites = set(prereq.split(' '))

            for grandchild in child.children:
                detail = grandchild.get('detail', None)
                if detail is None:
                    msg = "The 'detail' parameters is missing from '%s' in %s. It must be defined "\
                          "for all sub-blocks within a test group."
                    LOG.error(msg, grandchild.name, filename)

                _check_extra_param(grandchild, 'requirement', filename)
                _check_extra_param(grandchild, 'issues', filename)
                _check_extra_param(grandchild, 'design', filename)

                req.details.append(Detail(name=grandchild.name,
                                          path=req.path,
                                          filename=filename,
                                          text=str(detail),
                                          text_line=grandchild.line('detail')))


            out[group].append(req)
