#!/usr/bin/env python2
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import subprocess
from hit_load import hit_load

def check_requirement(filename):
    """Check spec file for requirement documentation."""

    messages = []
    root = hit_load(filename)
    design = root.children[0].get('design', None)
    issues = root.children[0].get('issues', None)
    for child in root.children[0]:
        if 'requirement' not in child:
            messages.append("    'requirement' parameter is missing in '{}' block.".format(child.name))

        if child.get('design', design) is None:
            messages.append("    'design' parameter is missing in '{}' block.".format(child.name))

        if child.get('issues', issues) is None:
            messages.append("    'issues' parameter is missing in '{}' block.".format(child.name))

    if messages:
        print 'ERROR in {}'.format(filename)
        print '\n'.join(messages) + '\n'
        return 1
    return 0

def sqa_check(working_dir=os.getcwd(), remote='origin', branch='devel', specs=['tests'], skip=[]):

    # Fetch
    cmd = ['git', 'fetch', remote]
    subprocess.call(cmd)

    # Check requirements on changed tests specs
    count = 0
    cmd = ['git', 'merge-base', '{}/{}'.format(remote, branch), 'HEAD']
    sha = subprocess.check_output(cmd).strip()
    cmd = ['git', 'diff', sha, '--name-only']
    for filename in subprocess.check_output(cmd).split('\n'):
        if os.path.isfile(filename) and (os.path.basename(filename) in specs) and \
           not any(s in filename for s in skip):
            count += check_requirement(os.path.join(working_dir, filename))

    return count
