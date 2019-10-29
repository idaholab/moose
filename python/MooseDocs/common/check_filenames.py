#:pylint: disable=missing-docstring
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
    filenames = project_find(filename)
    if len(filenames) == 0:
        msg = "{} does not exist."
        raise exceptions.MooseDocsException(msg, filename)
    elif len(filenames) > 1:
        msg = "Multiple files located with matching name '{}':\n".format(filename)
        for f in filenames:
            msg += '    {}\n'.format(f)
        raise exceptions.MooseDocsException(msg)
    else:
        filename = filenames[0]

    return filename
