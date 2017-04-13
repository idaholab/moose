#pylint: disable=missing-docstring
#################################################################
#                   DO NOT MODIFY THIS HEADER                   #
#  MOOSE - Multiphysics Object Oriented Simulation Environment  #
#                                                               #
#            (c) 2010 Battelle Energy Alliance, LLC             #
#                      ALL RIGHTS RESERVED                      #
#                                                               #
#           Prepared by Battelle Energy Alliance, LLC           #
#             Under Contract No. DE-AC07-05ID14517              #
#              With the U. S. Department of Energy              #
#                                                               #
#              See COPYRIGHT for full restrictions              #
#################################################################
import vtk
import mooseutils
from ChiggerFilterBase import ChiggerFilterBase
from .. import utils

class TubeFilter(ChiggerFilterBase):
    """
    Filter for applying tube filters to 1D results.
    """

    @staticmethod
    def getOptions():
        opt = ChiggerFilterBase.getOptions()
        opt.add('radius', None, "Radius of the tube.", vtype=float)
        opt.add('normalized_radius', 0.1, "Specify the radius as a percentage of the 'length' of "
                                          "the object, where the length is compute as the distance "
                                          "between the two points that comprise the object "
                                          "bounding box.")
        opt.add('caps', True, "Toggle the end-caps of the tube.")
        opt.add('sides', 30, "The number of edges for the tube.", vtype=int)
        return opt

    def __init__(self, **kwargs):
        super(TubeFilter, self).__init__(vtkfilter_type=vtk.vtkTubeFilter, **kwargs)

    def update(self, **kwargs):
        """
        Computes the contour levels for the vtkContourFilter.
        """
        super(TubeFilter, self).update(**kwargs)

        if self.isOptionValid('radius'):
            self._vtkfilter.SetRadius(self.getOption('radius'))

        if self.isOptionValid('normalized_radius'):
            if self.isOptionValid('radius'):
                mooseutils.mooseWarning("The 'radius' and 'normalized_radius' options are both "
                                        "set, the 'radius is being used.'")
            else:
                self._vtkfilter.SetRadius(utils.compute_distance(self._source) *
                                          self.getOption('normalized_radius'))

        if self.isOptionValid('sides'):
            self._vtkfilter.SetNumberOfSides(self.getOption('sides'))

        if self.isOptionValid('caps'):
            self._vtkfilter.SetCapping(self.getOption('caps'))
