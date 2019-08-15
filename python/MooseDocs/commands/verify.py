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
import re
import subprocess
import mooseutils
import MooseDocs

try:
    import bs4
except ImportError:
    print("The BeutifulSoup package (bs4) is required for the verify command, " \
          "it may be downloaded by running the following:\n" \
          "    pip install --user bs4")
    sys.exit(1)

def command_line_options(subparser, parent):
    """Command line options for 'verify' command."""
    parser = subparser.add_parser('verify', parents=[parent], help="Testing only, do not use.")
    parser.add_argument('-f', '--form', default='materialize',
                        choices=['materialize', 'html', 'latex'],
                        help="The desired output format to verify.")
    parser.add_argument('--disable', nargs='*', default=[],
                        help="A list of extensions to disable.")
    parser.add_argument('--update-gold', action='store_true',
                        help="Copy the rendered results to the gold directory.")
    parser.add_argument('--executioner',
                        default='MooseDocs.base.ParallelBarrier',
                        help="Select the mode of execution.")

def prepare_content(content):
    """Performs necessary substitutions for consistent comparisions."""
    content = insert_moose_dir(content)
    content = replace_uuid4(content)
    content = replace_package_file(content)
    content = replace_tmp_file(content)
    return content

def insert_moose_dir(content):
    """Helper for adding '${MOOSE_DIR}' to content."""
    return content.replace(os.getenv('MOOSE_DIR'), '${MOOSE_DIR}')

def replace_uuid4(content):
    """Replace uuid.uuid4() numbers."""
    return re.sub(r'[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}',
                  r'XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX', content)

def replace_package_file(content):
    """Replace the package filename with something consistent."""
    return re.sub(r'(moose-environment\S*\.pkg)', r'moose-environment.pkg', content)

def replace_tmp_file(content):
    """Replace tmp filenames."""
    return re.sub(r'tmp(.*?)(\.\w{3})', r'tmpXXXXX\2', content)

def update_gold_helper(gold, out_content):
    """Update the gold files."""
    if os.path.exists(gold):
        with open(gold, 'w') as fid:
            print("WRITING GOLD: {}".format(gold))
            fid.write(out_content)

def compare(out_fname, out_dir, gold_dir, update_gold=False):
    """Compare supplied file with gold counter part."""

    errno = 0
    gold_fname = out_fname.replace(out_dir, gold_dir)

    # Read the content to be tested
    with open(out_fname, 'r') as fid:
        out_content = prepare_content(fid.read())

    # Update gold content
    if update_gold:
        update_gold_helper(gold_fname, out_content)

    if not os.path.exists(gold_fname):
        # The verify command is going away because work is being done to move all testing to be
        # unittest based; but the test pages are still useful for development and will be used to
        # make sure the various executioner types achieve the same results.
        #
        # To allow test pages to exist without testing assume that if a gold doesn't exist then
        # it should not be tested.
        pass

    else:
        with open(gold_fname, 'r') as fid:
            gold_content = fid.read()
            gold_content = bs4.BeautifulSoup(gold_content, 'lxml').prettify()

        out_content = bs4.BeautifulSoup(out_content, 'lxml').prettify()
        diff = mooseutils.text_unidiff(out_content,
                                       gold_content,
                                       out_fname=out_fname,
                                       gold_fname=gold_fname,
                                       color=True, num_lines=2)
        if diff:
            print(mooseutils.colorText("DIFF: {} != {}".format(out_fname, gold_fname), 'YELLOW'))
            print(diff.encode('utf-8'))
            errno = 1
        else:
            print(mooseutils.colorText("PASS: {} == {}".format(out_fname, gold_fname), 'GREEN'))

    return errno

def main(options):
    """Test all files in output with those in the gold."""

    # Create the content
    config = options.form + '.yml'
    subprocess.check_output(['python', 'moosedocs.py', 'build',
                             '--config', config,
                             '--executioner', options.executioner,
                             '--disable', ' '.join(options.disable)],
                            cwd=os.path.join(MooseDocs.MOOSE_DIR, 'python', 'MooseDocs', 'test'))

    # Define output and gold directories
    out_dir = os.path.join('output', options.form)
    gold_dir = os.path.join('gold', options.form)

    # Setup extensions
    if options.form in ['materialize', 'html']:
        extensions = ['.html']
    elif options.form == 'latex':
        extensions = ['.tex']

    # Compare all files
    errno = 0
    for root, _, files in os.walk(out_dir):
        for fname in files:
            _, ext = os.path.splitext(fname)
            if ext in extensions:
                errno += compare(os.path.join(root, fname), out_dir, gold_dir, options.update_gold)

    return errno
