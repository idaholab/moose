# This utility is here to simplify using various python applications
# in this directory.  Import this module and call "active_path" specifiying
# the name of the system from which you would like to import python modules
# All directories within that module will be added to your PYTHON_PATH

import os, sys, inspect

def activate_module(module):
    base_dir = os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))
    module_dir = os.path.join(base_dir, module)

    if not os.path.exists(module_dir):
        print '"' + module_dir + '" is not a valid module'
        sys.exit(1)

    # Add all directories within the requested module to the system path
    for dirpath, dirnames, filenames in os.walk(module_dir):
        sys.path.append(dirpath)
