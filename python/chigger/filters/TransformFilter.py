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
from ChiggerFilterBase import ChiggerFilterBase
class TransformFilter(ChiggerFilterBase):
    """
    Filter for computing and visualizing contours.
    """

    @staticmethod
    def getOptions():
        opt = ChiggerFilterBase.getOptions()
        opt.add('scale', [1, 1, 1], "The scale to apply in the x, y, z coordinate dimensions.")
        return opt

    def __init__(self, **kwargs):
        super(TransformFilter, self).__init__(vtkfilter_type=vtk.vtkTransformPolyDataFilter,
                                              **kwargs)
        self.__transform = vtk.vtkTransform()
        self._vtkfilter.SetTransform(self.__transform)

    def update(self, **kwargs):
        """
        Computes the contour levels for the vtkContourFilter.
        """
        super(TransformFilter, self).update(**kwargs)

        if self.isOptionValid('scale'):
            inverse = self.__transform.GetInverse().GetScale()
            scale = self.getOption('scale')
            scale = [scale[i]*inverse[i] for i in range(len(scale))]
            self.__transform.Scale(scale)
