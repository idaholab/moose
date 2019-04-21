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

class ChiggerSource(ChiggerFilterSourceBase):
    """
    The base class for all 3D objects.

    All VTK settings that can be applied to the (VTKACTOR_TYPE and VTKMAPPER_TYPE) should be made in
    this class.

    Inputs:
        see ChiggerFilterSourceBase
    """

    # The 3D base class actor/mapper that this object to which ownership is restricted
    VTKACTOR_TYPE = vtk.vtkActor
    VTKMAPPER_TYPE = vtk.vtkMapper

    @staticmethod
    def getOptions():
        opt = ChiggerFilterSourceBase.getOptions()
        opt.add('orientation', None, "The orientation of the object.", vtype=list)
        opt.add('rotation', [0, 0, 0], "The rotation of the object about x, y, z axes.", vtype=list)
        opt.add('edges', False, "Enable/disable display of object edges.")
        opt.add('edge_color', [0, 0, 1], "Set the edge color.")
        opt.add('edge_width', "The edge width.", vtype=int)
        opt.add('point_size', "The point size.", vtype=float)
        opt.add('opacity', 1, "The object opacity.", vtype=float)
        opt.add('color', "The color of the object.", vtype=list)
        return opt

    def __init__(self, vtkactor_type=vtk.vtkActor, vtkmapper_type=vtk.vtkPolyDataMapper, **kwargs):
        super(ChiggerSource, self).__init__(vtkactor_type, vtkmapper_type, **kwargs)

    def update(self, **kwargs):
        """
        Updates the VTK settings for the VTKACTOR_TYPE/VTKMAPPER_TYPE objects. (override)

        Inputs:
            see ChiggerFilterSourceBase
        """
        super(ChiggerSource, self).update(**kwargs)

        if self.isOptionValid('orientation'):
            self._vtkactor.SetOrientation(self.getOption('orientation'))

        if self.isOptionValid('rotation'):
            x, y, z = self.getOption('rotation')
            self._vtkactor.RotateX(x)
            self._vtkactor.RotateY(y)
            self._vtkactor.RotateZ(z)

        if self.isOptionValid('edges') and \
           hasattr(self._vtkactor.GetProperty(), 'SetEdgeVisibility'):
            self._vtkactor.GetProperty().SetEdgeVisibility(self.getOption('edges'))

        if self.isOptionValid('edge_color') and \
           hasattr(self._vtkactor.GetProperty(), 'SetEdgeColor'):
            self._vtkactor.GetProperty().SetEdgeColor(self.getOption('edge_color'))

        if self.isOptionValid('edge_width') and \
           hasattr(self._vtkactor.GetProperty(), 'SetLineWidth'):
            self._vtkactor.GetProperty().SetLineWidth(self.getOption('edge_width'))

        if self.isOptionValid('point_size') and \
           hasattr(self._vtkactor.GetProperty(), 'SetPointSize'):
            self._vtkactor.GetProperty().SetPointSize(self.getOption('point_size'))

        if self.isOptionValid('opacity'):
            self._vtkactor.GetProperty().SetOpacity(self.getOption('opacity'))

        if self.isOptionValid('color'):
            self._vtkactor.GetProperty().SetColor(self.getOption('color'))
