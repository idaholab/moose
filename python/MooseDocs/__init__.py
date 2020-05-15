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
ROOT_DIR = mooseutils.git_root_dir()
os.environ['ROOT_DIR'] = ROOT_DIR

# Setup MOOSE_DIR/ROOT_DIR
MOOSE_DIR = os.getenv('MOOSE_DIR', None)
if MOOSE_DIR is None:
    print("The MOOSE_DIR environment must be set, this should be set within moosedocs.py.")
    sys.exit(1)

# List all files with git, this is done here to avoid running this command many times
PROJECT_FILES = mooseutils.git_ls_files(ROOT_DIR)
PROJECT_FILES.update(mooseutils.git_ls_files(MOOSE_DIR))
PROJECT_FILES.update(mooseutils.git_ls_files(os.path.join(MOOSE_DIR, 'large_media')))
