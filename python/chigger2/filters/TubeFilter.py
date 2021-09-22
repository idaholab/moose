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
from moosetools import mooseutils
from ChiggerFilterBase import ChiggerFilterBase
from .. import utils

class TubeFilter(ChiggerFilterBase):
    """
    Filter for applying tube filters to 1D results.
    """

    @staticmethod
    def validParams():
        opt = ChiggerFilterBase.validParams()
        opt.add('radius', doc="Radius of the tube.", vtype=(int, float))
        opt.add('normalized_radius', default=0.1, vtype=(int, float),
                doc="Specify the radius as a percentage of the 'length' of "
                "the object, where the length is compute as the distance "
                "between the two points that comprise the object "
                "bounding box.")
        opt.add('caps', default=True, vtype=bool, doc="Toggle the end-caps of the tube.")
        opt.add('sides', default=30, doc="The number of edges for the tube.", vtype=int)
        return opt

    def __init__(self, **kwargs):
        super(TubeFilter, self).__init__(vtkfilter_type=vtk.vtkTubeFilter, **kwargs)

    def update(self, **kwargs):
        """
        Computes the contour levels for the vtkContourFilter.
        """
        super(TubeFilter, self).update(**kwargs)

        if self.isParamValid('radius'):
            self._vtkfilter.SetRadius(self.getParam('radius'))

        if self.isParamValid('normalized_radius'):
            if self.isParamValid('radius'):
                mooseutils.mooseWarning("The 'radius' and 'normalized_radius' options are both "
                                        "set, the 'radius is being used.'")
            else:
                self._vtkfilter.SetRadius(utils.compute_distance(self._source) *
                                          self.getParam('normalized_radius'))

        if self.isOptoinValid('sides'):
            self._vtkfilter.SetNumberOfSides(self.getParam('sides'))

        if self.isParamValid('caps'):
            self._vtkfilter.SetCapping(self.getParam('caps'))
