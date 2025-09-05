# pylint: disable=missing-docstring
# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

from .ChiggerObject import ChiggerObject as ChiggerObject

from .ChiggerSourceBase import ChiggerSourceBase as ChiggerSourceBase
from .ChiggerFilterSourceBase import ChiggerFilterSourceBase as ChiggerFilterSourceBase
from .ChiggerSource import ChiggerSource as ChiggerSource
from .ChiggerSource2D import ChiggerSource2D as ChiggerSource2D

from .ChiggerResultBase import ChiggerResultBase as ChiggerResultBase
from .ChiggerResult import ChiggerResult as ChiggerResult
from .KeyPressInteractorStyle import KeyPressInteractorStyle as KeyPressInteractorStyle

from .ColorMap import ColorMap as ColorMap

from .ResultGroup import ResultGroup as ResultGroup


def create_single_source_result(source_type):
    """
    In many cases only a single source object (ChiggerSourceBase) must be linked to a renderer
    (ChiggerResult), as is the case for the annotations (e.g., TextAnnotation). To avoid creating
    nearly identical classes for each these cases this function can be used to generate the class
    needed based on the type provided.

    Inputs:
        SOURCE_TYPE: The type of source to attach to a ChiggerResult object.
    """

    class ChiggerResultMeta(ChiggerResult):
        """
        Meta class for creating ChiggerResult classes of different types.
        """

        @staticmethod
        def getOptions():
            opt = ChiggerResult.getOptions()
            opt += source_type.getOptions()
            return opt

        def __init__(self, **kwargs):
            super(ChiggerResultMeta, self).__init__(source_type(), **kwargs)

    return ChiggerResultMeta
