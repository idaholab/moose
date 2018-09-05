#!/usr/bin/env python
"""
Report statistics regarding the SQA documentation in MOOSE.
"""
import os
import mooseutils

def report_requirement_stats(location, specs):
    """
    Report requirement statistics for the test spec files with the supplied location.
    """
    requirements = 0
    tests = 0
    for filename in mooseutils.git_ls_files(location):
        if not os.path.isfile(filename):
            continue
        fname = os.path.basename(filename)
        if fname in specs:
            root = mooseutils.hit_load(filename)
            for child in root.children[0]:
                tests += 1
                if child.get('requirement', None):
                    requirements += 1

    complete = float(requirements)/float(tests)
    print 'Requirement Definitions ({:2.1f}% complete):'.format(complete*100)
    print '                 Location: {}'.format(location)
    print '                    Specs: {}'.format(' '.join(specs))
    print '    Total Number of Tests: {}'.format(tests)
    print '  Tests with Requirements: {}'.format(requirements)

if __name__ == '__main__':
    report_requirement_stats(os.path.abspath(os.path.join('..', 'test', 'tests')), ['tests'])
