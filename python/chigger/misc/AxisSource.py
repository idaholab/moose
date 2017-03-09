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
from .. import base
from .. import utils

class AxisSource(base.ChiggerFilterSourceBase):
    """
    Creates a Axis source for use with the ColorBar.
    """

    VTKACTOR_TYPE = vtk.vtkContextActor

    @staticmethod
    def getOptions():
        opt = base.ChiggerFilterSourceBase.getOptions()
        opt += utils.AxisOptions.get_options()
        return opt

    def __init__(self, **kwargs):
        super(AxisSource, self).__init__(vtkactor_type=vtk.vtkContextActor, vtkmapper_type=None,
                                         **kwargs)

        self._vtksource = vtk.vtkAxis()
        self._vtkactor.GetScene().AddItem(self._vtksource)

    def getVTKSource(self):
        """
        Return the vtkAxis object.
        """
        return self._vtksource

    def update(self, **kwargs):
        """
        Update the vtkAxis with given settings. (override)

        Inputs:
            see ChiggerFilterSourceBase
        """
        super(AxisSource, self).update(**kwargs)
        utils.AxisOptions.set_options(self._vtksource, self._options)
        self._vtksource.Update()
