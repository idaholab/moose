#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from enum import Enum

class ExtendedEnum(Enum):
    """
    Extension to Enum that allows getting
    all possible enumerations in a list
    """
    @classmethod
    def list(cls) -> list:
        """
        Get all possible enumerations in a list
        """
        return list(cls)
