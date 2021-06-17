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

if sys.version_info < (3, 6):
    print('"MOOSEDocs" requires python version 3.6 or greater, version {}.{} is being used.' \
          .format(sys.version_info[0], sys.version_info[1]))
    sys.exit(1)

import mooseutils

# Current logging level, used to allow for debug only type checking, etc.
LOG_LEVEL = logging.NOTSET

# The repository root location
is_git_repo = mooseutils.git_is_repo()
if is_git_repo:
    os.environ.setdefault('ROOT_DIR',  mooseutils.git_root_dir())
elif 'ROOT_DIR' not in os.environ:
    print('MooseDocs requires the ROOT_DIR environment variable to set when operating outside of a git repository.')
    sys.exit(1)

ROOT_DIR = os.environ['ROOT_DIR']

# Setup MOOSE_DIR/ROOT_DIR
MOOSE_DIR = os.getenv('MOOSE_DIR', None)
if MOOSE_DIR is None:
    print("The MOOSE_DIR environment must be set, this should be set within moosedocs.py.")
    sys.exit(1)

# Initialize submodule(s)
mooseutils.git_init_submodule('large_media', MOOSE_DIR)

# List all files, this is done here to avoid running this command many times
ls_files = mooseutils.git_ls_files if is_git_repo else mooseutils.list_files
PROJECT_FILES = ls_files(ROOT_DIR)
PROJECT_FILES.update(ls_files(MOOSE_DIR))
PROJECT_FILES.update(ls_files(os.path.join(MOOSE_DIR, 'large_media')))
