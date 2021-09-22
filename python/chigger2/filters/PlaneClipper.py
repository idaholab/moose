#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import copy
import vtk
from ClipperFilterBase import ClipperFilterBase

class PlaneClipper(ClipperFilterBase):
    """
    Clip object using a plane.
    """

    @staticmethod
    def validParams():
        opt = ClipperFilterBase.validParams()
        opt.add('origin', default=(0.5, 0.5, 0.5), size=3, vtype=(int, float),
                doc="The origin of the clipping plane.")
        opt.add('normal', default=(1, 0, 0), size=3, vtype=(int, float),
                doc="The outward normal of the clipping plane.")
        return opt

    def __init__(self, **kwargs):
        super(PlaneClipper, self).__init__(vtkclipfunction=vtk.vtkPlane, **kwargs)

    def update(self, **kwargs):
        """
        Update the normal and origin of the clipping plane.
        """
        super(PlaneClipper, self).update(**kwargs)

        if self.isParamValid('origin'):
            origin = self.getPosition(copy.copy(list(self.applyOption('origin'))))
            self._vtkclipfunction.SetOrigin(origin)

        if self.isParamValid('normal'):
            self._vtkclipfunction.SetNormal(self.applyOption('normal'))
