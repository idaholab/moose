#pylint: disable=missing-docstring
####################################################################################################
#                                    DO NOT MODIFY THIS HEADER                                     #
#                   MOOSE - Multiphysics Object Oriented Simulation Environment                    #
#                                                                                                  #
#                              (c) 2010 Battelle Energy Alliance, LLC                              #
#                                       ALL RIGHTS RESERVED                                        #
#                                                                                                  #
#                            Prepared by Battelle Energy Alliance, LLC                             #
#                               Under Contract No. DE-AC07-05ID14517                               #
#                               With the U. S. Department of Energy                                #
#                                                                                                  #
#                               See COPYRIGHT for full restrictions                                #
####################################################################################################
#pylint: enable=missing-docstring
import os
import sys
import argparse
import logging
import multiprocessing
import subprocess

import mooseutils

import MooseDocs
import commands

LOG = logging.getLogger(__name__)

class MooseDocsFormatter(logging.Formatter):
    """
    A formatter that is aware of the class hierarchy of the MooseDocs library.

    Call the init_logging function to initialize the use of this custom formatter.
    """
    COLOR = {'DEBUG':'CYAN',
             'INFO':'RESET',
             'WARNING':'YELLOW',
             'ERROR':'RED',
             'CRITICAL':'MAGENTA'}
    COUNTS = {'ERROR': multiprocessing.Value('I', 0, lock=True),
              'WARNING': multiprocessing.Value('I', 0, lock=True)}

    def format(self, record):
        msg = logging.Formatter.format(self, record)
        if record.levelname in self.COLOR:
            msg = mooseutils.colorText(msg, self.COLOR[record.levelname])

        if record.levelname in self.COUNTS:
            with self.COUNTS[record.levelname].get_lock():
                self.COUNTS[record.levelname].value += 1

        return msg

    def counts(self):
        """
        Return the number of warnings and errors.
        """
        return self.COUNTS['WARNING'].value, self.COUNTS['ERROR'].value

def init_logging(verbose=False):
    """
    Call this function to initialize the MooseDocs logging formatter.
    """

    # Setup the logger object
    if verbose:
        level = logging.DEBUG
    else:
        level = logging.INFO

    # Custom format that colors and counts errors/warnings
    formatter = MooseDocsFormatter()
    handler = logging.StreamHandler()
    handler.setFormatter(formatter)

    # The markdown package dumps way too much information in debug mode (so always set it to INFO)
    log = logging.getLogger('MARKDOWN')
    log.setLevel(logging.INFO)

    # Setup the custom formatter
    log = logging.getLogger('MooseDocs')
    log.addHandler(handler)
    log.setLevel(level)
    log.addHandler(logging.NullHandler())
    return formatter

def purge(exts):
    """
    Removes generated files from repository.

    Args:
      exts[list]: List of file extensions to purge (e.g., 'png'); it will be prefixed with
                 '.moose.' so the files actually removed are '.moose.png'.
    """
    for i, ext in enumerate(exts):
        exts[i] = '.moose.{}'.format(ext)

    for root, _, files in os.walk(os.getcwd(), topdown=False):
        for name in files:
            if any([name.endswith(ext) for ext in exts]):
                full_file = os.path.join(root, name)
                LOG.debug('Removing: %s', full_file)
                os.remove(full_file)

def command_line_options(*args):
    """
    Return the command line options for the moosedocs script.
    """

    # Command-line options
    parser = argparse.ArgumentParser(description="Tool for building and developing MOOSE and "
                                                 "MOOSE-based application documentation.")
    parser.add_argument('--verbose', '-v', action='store_true',
                        help="Execute with verbose (debug) output.")

    subparser = parser.add_subparsers(title='Commands', dest='command',
                                      description="Documentation creation command to execute.")

    # Add the sub-commands
    check_parser = subparser.add_parser('check', help="Check that the documentation exists and is "
                                                      "complete for your application and "
                                                      "optionally generating missing markdown "
                                                      "files.")
    commands.check_options(check_parser)

    build_parser = subparser.add_parser('build', help="Build the documentation for serving on "
                                                      "another system.")
    commands.build_options(build_parser)

    latex_parser = subparser.add_parser('latex', help="Generate a .tex or .pdf document from a "
                                                      "markdown file.")
    commands.latex_options(latex_parser)

    presentation_parser = subparser.add_parser('presentation', help="Convert a markdown file to "
                                                                    "an html presentation.")
    commands.presentation_options(presentation_parser)

    subparser.add_parser('test', help='Deprecated: use "~/projects/moose/python/run_tests -j8 '
                                      '--re=MooseDocs"')
    subparser.add_parser('generate', help='Deprecated: use "check --generate"')
    subparser.add_parser('serve', help='Deprecated: use "build --serve"')

    return parser.parse_args(*args)

def run():
    """
    Main MooseDocs command.
    """

    # Options
    options = vars(command_line_options())

    # Initialize logging
    formatter = init_logging(options.pop('verbose'))

    # Create cache directory.
    # This needs to be done hear rather than in MooseDocs/__init__.py to avoid race condition
    # when multiple processes are loading the MooseDocs module.
    if not os.path.exists(MooseDocs.TEMP_DIR):
        os.makedirs(MooseDocs.TEMP_DIR)

    # Remove moose.svg files (these get generated via dot)
    LOG.debug('Removing *.moose.svg files from %s', os.getcwd())
    purge(['svg'])

    # Pull LFS image files
    try:
        subprocess.check_output(['git', 'lfs', 'pull'], cwd=MooseDocs.ROOT_DIR)
    except subprocess.CalledProcessError:
        LOG.error("Unable to run 'git lfs', it is likely not installed but required for "
                  "the MOOSE documentation system.")
        sys.exit(1)

    # Execute command
    cmd = options.pop('command')
    if cmd == 'test':
        retcode = 0
        LOG.error('Deprecated command, please used "~/projects/moose/python/run_tests ' \
                  '-j8 --re=MooseDocs" instead.')
    elif cmd == 'check':
        retcode = commands.check(**options) #pylint: disable=assignment-from-none
    elif cmd == 'generate':
        retcode = None
        LOG.error('Deprecated command, please used "check --generate" instead.')
    elif cmd == 'serve':
        retcode = None
        LOG.error('Deprecated command, please used "build --serve" instead.')
    elif cmd == 'build':
        retcode = commands.build(**options) #pylint: disable=assignment-from-none
    elif cmd == 'latex':
        retcode = commands.latex(**options) #pylint: disable=assignment-from-none
    elif cmd == 'presentation':
        retcode = commands.presentation(**options) #pylint: disable=assignment-from-none

    # Check retcode
    if retcode is not None:
        return retcode

    # Display logging results
    warn, err = formatter.counts()
    print 'WARNINGS: {}  ERRORS: {}'.format(warn, err)
    return err > 0
