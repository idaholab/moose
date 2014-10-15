import os, sys

# Should just add ~/projects/moose/python to PYTHONPATH
MOOSE_DIR = os.path.abspath(os.path.join('..','..'))
if os.environ.has_key("MOOSE_DIR"):
  MOOSE_DIR = os.environ['MOOSE_DIR']
sys.path.append(os.path.join(MOOSE_DIR, 'python'))

# Setup the files for this package
from base import *
from slides import *
from slidesets import *
from images import *

__all__ = ['base', 'slidesets', 'slides', 'import']
