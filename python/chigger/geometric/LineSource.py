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
import GeometricSourceMeta
from .. import base
BaseType = GeometricSourceMeta.create(base.ChiggerSource)
class LineSource(BaseType):
    """
    Single LineSource object.
    """

    @staticmethod
    def getOptions():
        """
        Return the default options for this object.
        """
        opt = BaseType.getOptions()
        opt.add('resolution', 100, "The number of points sampled over the line.")
        opt.add('point1', [0, 0, 0], "The starting point of the line.")
        opt.add('point2', [1, 1, 0], "The ending point of the line.")
        return opt

    def __init__(self, **kwargs):
        super(LineSource, self).__init__(vtkgeometric_type=vtk.vtkLineSource, **kwargs)

    def update(self, **kwargs):
        """
        Set the options for this cube. (public)
        """
        super(LineSource, self).update(**kwargs)

        if self.isOptionValid('resolution'):
            self._vtksource.SetResolution(self.getOption('resolution'))

        if self.isOptionValid('point1'):
            self._vtksource.SetPoint1(self.getOption('point1'))

        if self.isOptionValid('point2'):
            self._vtksource.SetPoint2(self.getOption('point2'))

        self._vtksource.Update()
