#pylint: disable=missing-docstring, wrong-import-position
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import os
import sys
import subprocess
import logging

try:
    import anytree
    from anytree import search
except ImportError as e:
    MSG = "MooseDocs requires anytree (http://anytree.readthedocs.io/en/latest/index.html)\n"
    MSG += "version 2.4.0 or greater. If you are using the MOOSE environment package\n"
    MSG += "you can upgrade by running the following command.\n"
    MSG += "    pip3 install --upgrade --user anytree"
    print(MSG)
    sys.exit(1)

import mooseutils

# Markdown component types TODO: Move these to reader
BLOCK = 'block'
INLINE = 'inline'

# Current logging level, used to allow for debug only type checking, etc.
LOG_LEVEL = logging.NOTSET

# The repository root location
ROOT_DIR = mooseutils.git_root_dir()
os.environ['ROOT_DIR'] = ROOT_DIR

# File extensions to consider when building the content tree
FILE_EXT = ('.md', '.jpg', '.jpeg', '.gif', '.png', '.svg', '.webm', '.ogv', '.mp4', '.m4v', \
            '.pdf', '.css', '.js', '.bib', '.woff', '.woff2', '.html', '.ico', 'md.template')

# Setup MOOSE_DIR/ROOT_DIR
MOOSE_DIR = os.getenv('MOOSE_DIR', None)
if MOOSE_DIR is None:
    print("The MOOSE_DIR environment must be set, this should be set within moosedocs.py.")
    sys.exit(1)

# List all files with git, this is done here to avoid running this command many times
PROJECT_FILES = mooseutils.git_ls_files(ROOT_DIR)
PROJECT_FILES.update(mooseutils.git_ls_files(MOOSE_DIR))
