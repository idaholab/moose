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
import os
from mooseutils import mooseutils

from .commands import build, check, verify
from .common import log

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
    check.command_line_options(subparser, parent)
    verify.command_line_options(subparser, parent)
    return parser.parse_args()

def init_large_media():
    """
    Be sure large_media is checked out.
    """
    get_large_media = os.path.join(os.getenv('MOOSE_DIR'), 'scripts', 'get_large_media.sh')
    large_media_git = os.path.join(os.getenv('MOOSE_DIR'), 'large_media', '.git')
    if os.path.exists(get_large_media) and not os.path.exists(large_media_git):
        print('Checking out large_media...')
        mooseutils.shellCommand(get_large_media, os.getenv('MOOSE_DIR'))
        print('Done.')

def run():
    """
    Parse the command line options and run the correct command.
    """
    options = command_line_options()
    init_large_media()
    log.init_logging(getattr(logging, options.level))

    if options.command == 'build':
        errno = build.main(options)
    elif options.command == 'check':
        errno = check.main(options)
    elif options.command == 'verify':
        errno = verify.main(options)

    critical = log.MooseDocsFormatter.COUNTS['CRITICAL'].value
    errors = log.MooseDocsFormatter.COUNTS['ERROR'].value
    warnings = log.MooseDocsFormatter.COUNTS['WARNING'].value

    print('CRITICAL:{} ERROR:{} WARNING:{}'.format(critical, errors, warnings))
    if critical or errors or (errno != 0):
        return 1
    return 0

if __name__ == '__main__':
    run()
