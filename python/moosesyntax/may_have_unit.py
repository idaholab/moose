# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html


def may_have_unit(cpp_type):
    """
    Return True if an input parameter of the given C++ type is likely to carry a physical unit,
    and thus should have a documentation unit set with InputParameters::setDocUnit.
    """
    return any(
        name in cpp_type
        for name in (
            "Real",
            "double",
            "Variable",
            "Postprocessor",
            "Funct",
            "MaterialProperty",
        )
    )
