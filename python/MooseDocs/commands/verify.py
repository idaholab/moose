#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
#pylint: enable=missing-docstring
import os
import sys
import subprocess
import shutil
import mooseutils
import MooseDocs

def command_line_options(subparser, parent):
    """Command line options for 'verify' command."""
    parser = subparser.add_parser('verify', parents=[parent],
                                  help="Verify that rendering is working as expected.")

    parser.add_argument('-f', '--form', default='materialize',
                        choices=['materialize', 'html', 'json', 'latex'],
                        help="The desired output format to verify.")
    parser.add_argument('--update-gold', action='store_true',
                        help="Copy the rendered results to the gold directory.")

def main(options):
    """Test all files in output with those in the gold."""

    # Create the content
    config = options.form + '.yml'
    subprocess.check_output(['python', 'moosedocs.py', 'build', '--config', config],
                            cwd=os.path.join(MooseDocs.MOOSE_DIR, 'python', 'MooseDocs', 'test'))

    # Define output and gold directories
    out_dir = os.path.join('output', options.form)
    gold_dir = os.path.join('gold', options.form)

    # Setup extensions
    if options.form in ['materialize', 'html']:
        extensions = ['.html']
    elif options.form == 'json':
        extensions = ['.json']
    elif options.form == 'latex':
        extensions = ['.tex']

    # Compare all files
    errno = 0
    for root, _, files in os.walk(out_dir):
        for fname in files:
            _, ext = os.path.splitext(fname)
            if ext in extensions:
                out = os.path.join(root, fname)
                gold = out.replace(out_dir, gold_dir)

                if options.update_gold:
                    dirname = os.path.dirname(gold)
                    if not os.path.isdir(dirname):
                        os.makedirs(dirname)
                    shutil.copyfile(out, gold)
                else:
                    diff = mooseutils.unidiff(out, gold, color=True, num_lines=1)
                    if diff:
                        print diff
                        errno = 1
    sys.exit(errno)
