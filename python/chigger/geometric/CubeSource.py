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
class CubeSource(BaseType):
    """
    Single CubeSource object.
    """

    @staticmethod
    def getOptions():
        opt = BaseType.getOptions()
        opt.add("bounds", None, "The bounding box for the cube [xmin, xmax, ymin, ymax, zmin, "
                                "zmax]. This will overwrite the 'lengths' and 'center' options.",
                vtype=list)
        opt.add('lengths', None, "The lengths of the cube in the x,y,z-directions.", vtype=list)
        opt.add('center', None, "The center of the box.", vtype=list)
        return opt

    def __init__(self, **kwargs):
        super(CubeSource, self).__init__(vtkgeometric_type=vtk.vtkCubeSource, **kwargs)

    def update(self, **kwargs):
        """
        Set the options for this cube. (public)
        """
        super(CubeSource, self).update(**kwargs)

        if self.isOptionValid('center'):
            self._vtksource.SetCenter(self.getOption('center'))

        if self.isOptionValid('lengths'):
            x, y, z = self.getOption('lengths')
            self._vtksource.SetXLength(x)
            self._vtksource.SetYLength(y)
            self._vtksource.SetZLength(z)

        if self.isOptionValid('bounds'):
            self._vtksource.SetBounds(*self.getOption('bounds'))
