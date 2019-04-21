#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from .MultiAppExodusReader import MultiAppExodusReader

class NemesisReader(MultiAppExodusReader):
    """
    A reader for Nemesis files.

    With respect to rendering Nemesis files are identical to MultiApp files, this class is simply
    defined for naming reasons and possible future changes that may differ between the two
    types of file sets.
    """
    pass
