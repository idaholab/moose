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
from .ChiggerFilterBase import ChiggerFilterBase

class RotationalExtrusionFilter(ChiggerFilterBase):
    """
    Filter for rotating object about the Z-axis.

    Use the TransformFilter to position the object correctly for rotation.
    """

    @staticmethod
    def getOptions():
        opt = ChiggerFilterBase.getOptions()
        opt.add('angle', None, "Set the angle of rotation.", vtype=float)
        opt.add('resolution', None, "Set the rotational resolution.", vtype=int)
        return opt

    def __init__(self, **kwargs):
        super(RotationalExtrusionFilter, self).__init__(vtkfilter_type= \
                                                        vtk.vtkRotationalExtrusionFilter, **kwargs)

    def update(self, **kwargs):
        """
        Computes the contour levels for the vtkContourFilter.
        """
        super(RotationalExtrusionFilter, self).update(**kwargs)

        if self.isOptionValid('angle'):
            self._vtkfilter.SetAngle(self.getOption('angle'))

        if self.isOptionValid('resolution'):
            self._vtkfilter.SetResolution(self.getOption('resolution'))
