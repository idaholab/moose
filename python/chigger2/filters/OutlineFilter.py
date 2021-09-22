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
from ChiggerFilterBase import ChiggerFilterBase

class OutlineFilter(ChiggerFilterBase):
    """
    Applies visual bounding box to the result.
    """

    @staticmethod
    def validParams():
        opt = ChiggerFilterBase.validParams()
        return opt

    def __init__(self, **kwargs):
        super(OutlineFilter, self).__init__(vtkfilter_type=vtk.vtkOutlineFilter, **kwargs)

    def update(self, **kwargs):
        """
        Computes the contour levels for the vtkContourFilter.
        """
        super(OutlineFilter, self).update(**kwargs)
