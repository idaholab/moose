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
from .ClipperFilterBase import ClipperFilterBase

class BoxClipper(ClipperFilterBase):
    """
    Clip object using a box.

    see vtkBox
    """

    @staticmethod
    def getOptions():
        opt = ClipperFilterBase.getOptions()
        opt.add('lower', [0.5, 0.5, 0.5], "The outward normal of the clipping plane.")
        opt.add('upper', [1, 1, 1], "The origin of the clipping plane.")
        return opt

    def __init__(self, **kwargs):
        super(BoxClipper, self).__init__(vtkclipfunction=vtk.vtkBox, **kwargs)

    def update(self, **kwargs):
        """
        Update the bounds of the clipping box.
        """
        super(BoxClipper, self).update(**kwargs)
        lower = self.getPosition(self.getOption('lower'))
        upper = self.getPosition(self.getOption('upper'))
        self._vtkclipfunction.SetXMin(lower)
        self._vtkclipfunction.SetXMax(upper)
