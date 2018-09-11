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
import GeometricSourceMeta
from .. import base
BaseType = GeometricSourceMeta.create(base.ChiggerSource)
class LineSource(BaseType):
    """
    Single LineSource object.
    """

    @staticmethod
    def validOptions():
        """
        Return the default options for this object.
        """
        opt = BaseType.validOptions()
        opt.add('resolution', default=100, vtype=int,
                doc="The number of points sampled over the line.")
        opt.add('point1', default=(0, 0, 0), size=3, vtype=float,
                doc="The starting point of the line.")
        opt.add('point2', default=(1, 1, 0), size=3, vtype=float,
                doc="The ending point of the line.")
        return opt

    def __init__(self, **kwargs):
        super(LineSource, self).__init__(vtkgeometric_type=vtk.vtkLineSource, **kwargs)

    def update(self, **kwargs):
        """
        Set the options for this cube. (public)
        """
        super(LineSource, self).update(**kwargs)

        if self.isOptionValid('resolution'):
            self._vtksource.SetResolution(self.applyOption('resolution'))

        if self.isOptionValid('point1'):
            self._vtksource.SetPoint1(self.applyOption('point1'))

        if self.isOptionValid('point2'):
            self._vtksource.SetPoint2(self.applyOption('point2'))
