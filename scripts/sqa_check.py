#!/usr/bin/env python2
import os
import sys
import subprocess
import argparse

moose_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(moose_dir, 'python'))

import mooseutils

def get_options():
    """Command-line options."""
    parser = argparse.ArgumentParser(description='SQA Requirement checking tool.')

    parser.add_argument('-r', '--remote', type=str, default='origin',
                        help="The name of the git remote to compare against.")
    parser.add_argument('-b', '--branch', type=str, default='devel',
                        help="The name of the branch to compare against.")
    parser.add_argument('--specs', type=list, default=['tests'],
                        help="The name of the specification files to consider.")
    return parser.parse_args()

def check_requirement(filename):
    """Check spec file for requirement documentation."""

    messages = []
    root = mooseutils.hit_load(filename)
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

if __name__ == '__main__':

    opt = get_options()

    # Fetch
    cmd = ['git', 'fetch', opt.remote]
    subprocess.call(cmd)

    # Root git directory
    root = mooseutils.git_root_dir()

    # Check requirements on changed tests specs
    count = 0
    cmd = ['git', 'diff', '{}/{}'.format(opt.remote, opt.branch), '--name-only']
    for filename in subprocess.check_output(cmd).split('\n'):
        if os.path.isfile(filename) and (os.path.basename(filename) in opt.specs):
            count += check_requirement(os.path.join(root, filename))

    sys.exit(count)
