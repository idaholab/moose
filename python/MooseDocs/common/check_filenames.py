#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from .project_find import project_find
from . import exceptions

def check_filenames(filename):
    """
    Helper function for error checking project file search.

    Inputs:
        filename[str]: The filename to search.
    """
    if filename == '':
        raise exceptions.MooseDocsException("Empty file name")
    filenames = project_find(filename)
    if len(filenames) == 0:
        msg = "{} does not exist in the repository. The command 'git ls-files' is used for " \
              "locating files, so the desired file must be committed or staged prior to " \
              "running moosedocs.py"
        raise exceptions.MooseDocsException(msg, filename)
    elif len(filenames) > 1:
        msg = "Multiple files located with matching name '{}':\n".format(filename)
        for i, f in enumerate(filenames):
            if (i < 20):
                msg += '    {}\n'.format(f)
            else:
                msg += 'Truncated\n'
                break
        raise exceptions.MooseDocsException(msg)
    else:
        filename = filenames[0]

    return filename
