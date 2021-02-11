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
import os
import argparse
import logging
import mooseutils

from .commands import build, verify, check, generate, syntax, init
from .common import log

def command_line_options():
    """
    The main command line parser, this creates the main parser and calls the
    calls the command_line_options method for each command.
    """
    desc = "MooseDocs: A utility to build MOOSE documentation from a single source."
    parser = argparse.ArgumentParser(description=desc)
    subparser = parser.add_subparsers(dest='command', help='Available commands.')
    subparser.required = True

    # Common arguments
    parent = argparse.ArgumentParser(add_help=False)
    levels = ['CRITICAL', 'ERROR', 'WARNING', 'INFO', 'DEBUG']
    parent.add_argument('--level', '-l',
                        choices=levels,
                        default='INFO',
                        help="Set the python logging level (default: %(default)s).")

    build.command_line_options(subparser, parent)
    check.command_line_options(subparser, parent)
    verify.command_line_options(subparser, parent)
    generate.command_line_options(subparser, parent)
    syntax.command_line_options(subparser, parent)
    init.command_line_options(subparser, parent)

    return parser.parse_args()

def run():
    """
    Parse the command line options and run the correct command.
    """
    options = command_line_options()
    log.init_logging(getattr(logging, options.level))

    if options.command == 'build':
        errno = build.main(options)
    elif options.command == 'check':
        errno = check.main(options)
    elif options.command == 'verify':
        errno = verify.main(options)
    elif options.command == 'generate':
        errno = generate.main(options)
    elif options.command == 'syntax':
        errno = syntax.main(options)
    elif options.command == 'init':
        errno = init.main(options)
    else:
        errno = 1


    handler = logging.getLogger('MooseDocs').handlers[0]
    critical = handler.getCount(logging.CRITICAL)
    errors =   handler.getCount(logging.ERROR)
    warnings = handler.getCount(logging.WARNING)

    print('CRITICAL:{} ERROR:{} WARNING:{}'.format(critical, errors, warnings))
    if critical or errors or (errno != 0):
        return 1
    return 0

if __name__ == '__main__':
    run()
