#pylint: disable=missing-docstring
#################################################################
#                   DO NOT MODIFY THIS HEADER                   #
#  MOOSE - Multiphysics Object Oriented Simulation Environment  #
#                                                               #
#            (c) 2010 Battelle Energy Alliance, LLC             #
#                      ALL RIGHTS RESERVED                      #
#                                                               #
#           Prepared by Battelle Energy Alliance, LLC           #
#             Under Contract No. DE-AC07-05ID14517              #
#              With the U. S. Department of Energy              #
#                                                               #
#              See COPYRIGHT for full restrictions              #
#################################################################
from ChiggerObject import ChiggerObject

from ChiggerSourceBase import ChiggerSourceBase
from ChiggerFilterSourceBase import ChiggerFilterSourceBase
from ChiggerSource import ChiggerSource
from ChiggerSource2D import ChiggerSource2D

from ChiggerResultBase import ChiggerResultBase
from ChiggerResult import ChiggerResult
from KeyPressInteractorStyle import KeyPressInteractorStyle

from ColorMap import ColorMap

from ChiggerTimer import ChiggerTimer

from ResultGroup import ResultGroup

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
