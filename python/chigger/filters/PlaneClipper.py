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
import copy
import vtk
from ClipperFilterBase import ClipperFilterBase

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
