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
from .ChiggerFilterSourceBase import ChiggerFilterSourceBase

class ChiggerSource2D(ChiggerFilterSourceBase):
    """
    The base class for all 2D objects.

    All VTK settings that can be applied to the (VTKACTOR_TYPE and VTKMAPPER_TYPE) should be made in
    this class.

    Inputs:
        see ChiggerFilterSourceBase
    """

    # The 2D base class actor/mapper that this object to which ownership is restricted
    VTKACTOR_TYPE = vtk.vtkActor2D
    VTKMAPPER_TYPE = vtk.vtkMapper2D

    @staticmethod
    def getOptions():
        opt = ChiggerFilterSourceBase.getOptions()
        opt.add('opacity', 1, "The object opacity.", vtype=float)
        opt.add('color', "The color of the object.", vtype=list)
        return opt

    def __init__(self, vtkactor_type=vtk.vtkActor2D, vtkmapper_type=vtk.vtkPolyDataMapper2D,
                 **kwargs):
        super(ChiggerSource2D, self).__init__(vtkactor_type, vtkmapper_type, **kwargs)

    def update(self, **kwargs):
        """
        Updates the VTK settings for the VTKACTOR_TYPE/VTKMAPPER_TYPE objects. (override)

        Inputs:
            see ChiggerFilterSourceBase
        """
        super(ChiggerSource2D, self).update(**kwargs)

        if self.isOptionValid('opacity'):
            self._vtkactor.GetProperty().SetOpacity(self.getOption('opacity'))

        if self.isOptionValid('color'):
            self._vtkactor.GetProperty().SetColor(self.getOption('color'))
