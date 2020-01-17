#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import MooseDocs

def project_find(filename):
    """
    Utility for finding files within a project based on 'git ls-files', see MooseDocs.__init__.py.

    The main use for this function is locating source files for inclusion in listings or for
    creating bottom modals with source code.

    The list of files is populated in MooseDocs.__init__.py, otherwise the list was created
    multiple times.
    """
    matches = [fname for fname in MooseDocs.PROJECT_FILES if fname.endswith(filename)]
    return matches
