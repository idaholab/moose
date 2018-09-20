#pylint: disable=missing-docstring, no-member
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
"""
Main program for running MooseDocs. The moosedocs.py script that exists within the
documentation directory for applications call this in similar fashion to
MOOSE run_tests.
"""
import argparse
import logging

from commands import build, devel, check, update, profiler, verify
from common import log

def command_line_options():
    """
    The main command line parser, this creates the main parser and calls the
    calls the command_line_options method for each command.
    """
    desc = "MooseDocs: A utility to build MOOSE documentation from a single source."
    parser = argparse.ArgumentParser(description=desc)
    subparser = parser.add_subparsers(dest='command', help='Available commands.')

    # Common arguments
    parent = argparse.ArgumentParser(add_help=False)
    levels = ['CRITICAL', 'ERROR', 'WARNING', 'INFO', 'DEBUG']
    parent.add_argument('--level', '-l',
                        choices=levels,
                        default='INFO',
                        help="Set the python logging level (default: %(default)s).")

    build.command_line_options(subparser, parent)
    devel.command_line_options(subparser, parent)
    check.command_line_options(subparser, parent)
    update.command_line_options(subparser, parent)
    profiler.command_line_options(subparser, parent)
    verify.command_line_options(subparser, parent)
    return parser.parse_args()

def run():
    """
    Parse the command line options and run the correct command.
    """

    options = command_line_options()
    log.init_logging(getattr(logging, options.level))

    if options.command == 'build':
        build.main(options)
    elif options.command == 'devel':
        devel.main(options)
    elif options.command == 'check':
        check.main(options)
    elif options.command == 'update':
        update.main(options)
    elif options.command == 'profile':
        profiler.main(options)
    elif options.command == 'verify':
        verify.main(options)

    critical = log.MooseDocsFormatter.COUNTS['CRITICAL'].value
    errors = log.MooseDocsFormatter.COUNTS['ERROR'].value
    warnings = log.MooseDocsFormatter.COUNTS['WARNING'].value

    print 'CRITICAL:{} ERROR:{} WARNING:{}'.format(critical, errors, warnings)
    if critical or errors:
        return 1
    return 0

if __name__ == '__main__':
    run()
