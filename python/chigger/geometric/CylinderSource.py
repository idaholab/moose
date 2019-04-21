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
from . import GeometricSourceMeta
from .. import base

BaseType = GeometricSourceMeta.create(base.ChiggerSource)
class CylinderSource(BaseType):
    """
    A helper object for defining a cylinder and the settings for it.
    """

    @staticmethod
    def getOptions():
        opt = BaseType.getOptions()
        opt.add('height', None, "The height of the cylinder", vtype=float)
        opt.add('radius', None, "The radius of the cylinder.", vtype=float)
        opt.add('resolution', 8, "The number of sides of the cylinder.", vtype=int)
        opt.add("capping", False, "Display the end caps or not.", vtype=bool)
        return opt

    def __init__(self, **kwargs):
        super(CylinderSource, self).__init__(vtkgeometric_type=vtk.vtkCylinderSource, **kwargs)

    def update(self, **kwargs):
        """
        Set the options for this cylinder. (public)
        """
        super(CylinderSource, self).update(**kwargs)

        if self.isOptionValid('height'):
            self._vtksource.SetHeight(self.getOption('height'))
        if self.isOptionValid('radius'):
            self._vtksource.SetRadius(self.getOption('radius'))
        if self.isOptionValid('resolution'):
            self._vtksource.SetResolution(self.getOption('resolution'))
        if self.isOptionValid('capping'):
            self._vtksource.CappingOn()
