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
import vtk
from LabelExodusSource import LabelExodusSource
from .. import base

class LabelExodusResult(base.ChiggerResult):
    """
    Object for attaching labels to ExodusResult options.

    Args:
        result[ExodusResult]: The result object to label.
    """
    @staticmethod
    def getOptions():
        opt = base.ChiggerResult.getOptions()
        opt += LabelExodusSource.getOptions()
        return opt

    def __init__(self, exodus_result, **kwargs):

        # Populate the sources to display
        self._exodus_result = exodus_result
        sources = []
        for src in self._exodus_result:
            sources.append(LabelExodusSource(src, **kwargs))

        super(LabelExodusResult, self).__init__(*sources, renderer=exodus_result.getVTKRenderer(),
                                                viewport=exodus_result.getOption('viewport'),
                                                vtkmapper_type=vtk.vtkLabeledDataMapper, **kwargs)
