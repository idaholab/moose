#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from peacock.PeacockException import FileExistsException, BadExecutableException
import mooseutils
import subprocess
import os

def runExe(app_path, args, print_errors=True):
    """
    Convience function to run a executable with arguments and return the output
    Input:
        app_path: str: Path to the excutable
        args: either str or list: Arguments to pass to the executable
    Return:
        str: output of running the command
    Exceptions:
        FileExistsException: If there was a problem running the executable
        BadExecutableException: If the executable didn't exit cleanly
    """
    popen_args = [str(app_path)]
    if isinstance(args, str):
        popen_args.append(args)
    else:
        popen_args.extend(args)

    proc = None
    try:
        proc = subprocess.Popen(popen_args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    except OSError as e:
        msg = "Problem running '%s'" % ' '.join(popen_args)
        if print_errors:
            mooseutils.mooseWarning(msg)
        msg += "\nError: %s" % e
        raise FileExistsException(msg)

    data = proc.communicate()
    stdout_data = data[0].decode("utf-8")
    if proc.returncode != 0:
        msg = "'%s' exited with non zero status %s.\n\n"\
                "Please make sure your application is built and able to execute the given arguments.\n"\
                "Working dir: %s\n"\
                "Output: %s" % (' '.join(popen_args), proc.returncode, os.getcwd(), stdout_data)
        if print_errors:
            mooseutils.mooseWarning(msg)
        raise BadExecutableException(msg)
    return stdout_data
