import os
import sys
import subprocess
from mooseutils import find_moose_directory

"""
First, attempt to import capabilities. If that fails, try adding the capabilities source directory to the path
and try the import again. If that fails, try running "make capabilities" before importing.
"""

moose_dir = find_moose_directory()
try:
    from capabilities import *  # noqa: F403
except ImportError:
    capabilities_dir = os.path.join(moose_dir, "framework", "contrib", "capabilities")
    sys.path.append(capabilities_dir)
    try:
        from capabilities import *  # noqa: F403
    except ImportError:
        moose_test_dir = os.path.abspath(os.path.join(moose_dir, "test"))
        cmd = ["make", "capabilities"]
        print(f'INFO: Building capabilities with "{" ".join(cmd)}" in {moose_test_dir}')
        try:
            subprocess.run(cmd, cwd=moose_test_dir, check=True)
        except subprocess.CalledProcessError:
            raise SystemExit("ERROR: Failed to build capabilities")
        try:
            from capabilities import *  # noqa: F403
        except:
            print("ERROR: Failed to import capabilities after building")
            raise
