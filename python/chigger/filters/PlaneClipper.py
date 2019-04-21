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
from .ClipperFilterBase import ClipperFilterBase

class PlaneClipper(ClipperFilterBase):
    """
    Clip object using a plane.
    """

    @staticmethod
    def getOptions():
        opt = ClipperFilterBase.getOptions()
        opt.add('origin', [0.5, 0.5, 0.5], "The origin of the clipping plane.")
        opt.add('normal', [1, 0, 0], "The outward normal of the clipping plane.")
        return opt

    def __init__(self, **kwargs):
        super(PlaneClipper, self).__init__(vtkclipfunction=vtk.vtkPlane, **kwargs)

    def update(self, **kwargs):
        """
        Update the normal and origin of the clipping plane.
        """
        super(PlaneClipper, self).update(**kwargs)

        origin = self.getPosition(copy.copy(self.getOption('origin')))
        self._vtkclipfunction.SetNormal(self.getOption('normal'))
        self._vtkclipfunction.SetOrigin(origin)
