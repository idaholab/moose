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
                 satisfied=True):
        self.name = name
        self.path = path
        self.filename = filename
        self.text = text
        self.text_line = text_line
        self.design = design
        self.design_line = design_line
        self.issues = issues
        self.label = None # added by get_requirements function
        self.satisfied = satisfied
        self.prerequisites = []
        self.verification = False
        self.validation = False

    def __str__(self):
        frmt = '{}:\n    Text: {}\n    Design: {}\n    Issues: {}'
        return frmt.format(self.name, self.text, repr(self.design), repr(self.issues))

def get_requirements(directories, specs):
    """
    Build requirements dictionary from the provided directories.
    """
    out = collections.defaultdict(list)
    for location in directories:
        for filename in mooseutils.git_ls_files(location):
            if not os.path.isfile(filename):
                continue
            fname = os.path.basename(filename)
            if fname in specs:
                _add_requirements(out, location, filename)

    for i, requirements in enumerate(out.itervalues()):
        for j, req in enumerate(requirements):
            req.label = "F{}.{}".format(i+1, j+1)
    return out

def _add_requirements(out, location, filename):
    """Opens tests specification and extracts requirement items."""
    root = mooseutils.hit_load(filename)
    design = root.children[0].get('design', None)
    design_line = root.children[0].line('design', None)
    issues = root.children[0].get('issues', None)
    for child in root.children[0]:
        if 'requirement' in child:

            local_design = child.get('design', design)
            local_design_line = child.line('design', design_line)
            if local_design is None:
                msg = "The 'design' parameter is missing from '%s' in %s. It must be defined at " \
                      "the top level and/or within the individual test specification. It " \
                      "should contain a space separated list of filenames."
                LOG.error(msg, child.name, filename)
                local_design = ''

            local_issues = child.get('issues', issues)
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
                              text=unicode(text),
                              text_line=child.line('requirement', None),
                              design=local_design.split(),
                              design_line=local_design_line,
                              issues=local_issues.split(),
                              satisfied=satisfied)

            req.verification = child.get('verification', False)
            req.validation = child.get('validation', False)

            group = os.path.relpath(filename, location).split('/')[0]

            prereq = child.get('prereq', None)
            if prereq is not None:
                req.prerequisites = set(prereq.split(' '))

            out[group].append(req)
