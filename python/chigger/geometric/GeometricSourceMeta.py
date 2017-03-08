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
import mooseutils
from .. import base

def create(base_type):
    """
    Function for creating GeometricSource objects.
    """
    class GeometricSource(base_type):
        """
        Base class for geometric objects (e.g., cube, sphere, plane, etc.)
        """
        @staticmethod
        def getOptions():
            """
            Return the default options for this object.
            """
            opt = base_type.getOptions()
            opt.add('position', None, "The position of the object.", vtype=list)
            opt.add('scale', None, "The scale of the object.", vtype=float)
            return opt

        def __init__(self, vtkgeometric_type=None, **kwargs):
            super(GeometricSource, self).__init__(**kwargs)

            if not vtkgeometric_type:
                raise mooseutils.MooseException('The vtk source object type must be supplied.')

            self._vtksource = vtkgeometric_type()

            self._colormap = base.ColorMap()

        def getVTKSource(self):
            """
            Return the vtk geometric object.
            """
            return self._vtksource

        def update(self, **kwargs):
            """
            Update the source object.
            """
            super(GeometricSource, self).update(**kwargs)

            if self.isOptionValid('position'):
                self._vtkactor.SetPosition(self.getOption('position'))

            if self.isOptionValid('scale'):
                self._vtkactor.SetScale(self.getOption('scale'))

    return GeometricSource
