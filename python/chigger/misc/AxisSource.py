#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

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
