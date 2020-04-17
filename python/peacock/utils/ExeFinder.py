#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import glob
import platform

def recursiveFindFile(current_path, glob_pattern, problems_dir="problems"):
    """
    Finds an executable matching glob_pattern in the current directory.
    If it is not in the current directory then it looks in the parent directory.
    Inputs:
        current_path: str: Current directory to look in
        glob_pattern: str: Pattern to match on the executable
    Return:
        str: Path to executable if found, else None
    """
    if not os.path.exists(current_path):
        return None

    files = glob.glob(os.path.join(current_path, glob_pattern))
    for f in files:
        if os.access(f, os.X_OK):
            return f

    parent_dir = os.path.dirname(current_path)
    if os.path.basename(parent_dir) == problems_dir:
        # if we're in the "problems" directory... hop over to this application's directory instead
        grandparent_dir = os.path.dirname(parent_dir)
        new_dir = os.path.join(grandparent_dir, os.path.basename(current_path))
        the_file = recursiveFindFile(new_dir, glob_pattern)
        # Still didn't find it, we must keep looking up this path so fall through here
        if the_file != None:
            return the_file

    if parent_dir != "/":
        return recursiveFindFile(parent_dir, glob_pattern)

    return None

def searchForExe(start_dir=None, methods=None):
    """
    Find a MOOSE based executable with the format *-$METHOD
    This function looks up the directory tree for the executable,
    all the way to / . Does not search branches.
    Returns:
        str: Path to executable or None if nothing found
    """
    if not methods:
        methods = ["opt", "dbg", "oprof", "devel"]
        method = os.environ.get("METHOD")
        if method:
            methods = [method]

    for method in methods:
        if platform.system() == 'Windows':
            glob_pattern = "*-%s.exe" % method
        else:
            glob_pattern = "*-%s" % method

        if not start_dir:
            start_dir = os.getcwd()
        executable = recursiveFindFile(start_dir, glob_pattern)
        if executable:
            print("Found executable: %s" % executable)
            return executable

    print('No executable found for method type(s): %s' % ', '.join(methods))
    return None

def getExecutablePath(options, start_dir=None):
    """
    Tries to find an executable.
    It first looks in the command line options.
    If not found it will search up the directory path.
    Input:
        options[argparse namespace]: The command line options as returned by ArgumentParser.parse_args()
    """
    methods = ["opt", "dbg", "oprof", "devel"]
    if "METHOD" in os.environ:
        methods = [os.environ.get("METHOD")]
    if options:
        if options.executable:
            exe_path = os.path.abspath(options.executable)
            options.executable = exe_path
            return exe_path

        if options.method:
            methods = [options.method]

        for arg in options.arguments:
            for method in methods:
                if arg.endswith("-%s" % method):
                    if os.access(arg, os.X_OK):
                        exe_path = os.path.abspath(arg)
                        options.executable = exe_path
                        return exe_path
                    else:
                        print("Executable argument %s is not executable!" % arg)
        if not options.exe_search:
            print("Automatic executable search disabled")
            return

    # not on the command line, do a search
    exe_path = searchForExe(methods=methods, start_dir=start_dir)
    if exe_path:
        exe_path = os.path.abspath(exe_path)
        return exe_path
    return None
