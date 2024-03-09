import os
import sys
import subprocess
import mooseutils

"""
First, attempt to import capabilities. If that fails, try adding the capabilities source directory to the path
and try the import again. If that fails, try running "make capabilities" before importing.
"""

moose_dir = os.getenv('MOOSE_DIR', os.path.join(os.path.dirname(__file__), '..', '..'))
try:
    import capabilities
except ImportError:
    capabilities_dir = os.path.join(moose_dir, 'framework', 'contrib', 'capabilities')
    sys.path.append(capabilities_dir)
    try:
        import capabilities
    except ImportError:
        moose_test_dir = os.path.abspath(os.path.join(moose_dir, 'test'))
        subprocess.run(['make', 'capabilities'], cwd=moose_test_dir)
        import capabilities

from capabilities import *
