import os
import sys
import subprocess
import mooseutils

"""
First, attempt to import hit. If that fails, try adding the hit source directory to the path
and try the import again. If that fails, try running "make hit" before importing.
"""

moose_dir = os.getenv('MOOSE_DIR', os.path.join(os.path.dirname(__file__), '..', '..'))
try:
    import hit
except ImportError:
    hit_dir = os.path.join(moose_dir, 'framework', 'contrib', 'hit')
    sys.path.append(hit_dir)
    try:
        import hit
    except ImportError:
        moose_test_dir = os.path.abspath(os.path.join(moose_dir, 'test'))
        subprocess.run(['make', 'hit'], cwd=moose_test_dir)
        import hit

from hit import TokenType, Token
from .pyhit import Node, load, write, parse, tokenize
