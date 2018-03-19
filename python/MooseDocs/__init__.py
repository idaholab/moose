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
import subprocess
import logging

# Markdown component types TODO: Move these to reader
BLOCK = 'block'
INLINE = 'inline'

# Current logging level, used to allow for debug only type checking, etc.
LOG_LEVEL = logging.NOTSET

# The repository root locatoin
ROOT_DIR = subprocess.check_output(['git', 'rev-parse', '--show-toplevel'],
                                   cwd=os.getcwd(),
                                   stderr=subprocess.STDOUT).strip('\n')

# File extensions to consider when building the content tree
FILE_EXT = ('.md', '.jpg', '.jpeg', '.gif', '.png', '.svg', '.ogg', '.webm', '.mp4', '.css', \
            '.js', '.bib', '.woff', '.woff2')

# MOOSE
MOOSE_DIR = os.getenv('MOOSE_DIR', os.path.join(os.getcwd(), '..', 'moose'))
if not os.path.exists(MOOSE_DIR):
    MOOSE_DIR = os.path.join(os.getenv('HOME'), 'projects', 'moose')

# Add the system environment
# TODO: Update code to use this rather than MooseDocs.MOOSE_DIR, which would avoid needing to import
#       MooseDocs in many locations, use "MOOSEDOCS_MOOSE_DIR" and "MOOSEDOCS_ROOT_DIR".
os.environ['MOOSE_DIR'] = MOOSE_DIR
os.environ['ROOT_DIR'] = ROOT_DIR

# List all files with git, this is done here to avoid running this command many times
PROJECT_FILES = set()
for fname in subprocess.check_output(['git', 'ls-files'], cwd=ROOT_DIR).split('\n'):
    PROJECT_FILES.add(os.path.join(ROOT_DIR, fname))
for fname in subprocess.check_output(['git', 'ls-files'], cwd=MOOSE_DIR).split('\n'):
    PROJECT_FILES.add(os.path.join(MOOSE_DIR, fname))
