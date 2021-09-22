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
from .. import utils
from .GeometricSource import GeometricSource

class Cube(GeometricSource):
    """
    Single Cube object.
    """
    VTKSOURCETYPE = vtk.vtkCubeSource

    @staticmethod
    def validParams():
        opt = GeometricSource.validParams()
        opt.add('color', vtype=(utils.Color, utils.AutoColor), doc="The color of the cube")
        opt.add('xmin', default=0, vtype=(int, float),
                doc="Minimum x-value in 3D renderer coordinates.")
        opt.add('xmax', default=1, vtype=(int, float),
                doc="Minimum x-value in 3D renderer coordinates.")
        opt.add('ymin', default=0, vtype=(int, float),
                doc="Minimum y-value in 3D renderer coordinates.")
        opt.add('ymax', default=1, vtype=(int, float),
                doc="Maximum y-value in 3D renderer coordinates.")
        opt.add('zmin', default=0, vtype=(int, float),
                doc="Minimum z-value in 3D renderer coordinates.")
        opt.add('zmax', default=1, vtype=(int, float),
                doc="Maximum z-value in 3D renderer coordinates.")
        return opt

    def _onRequestInformation(self, *args):
        """
        Set the options for this cube. (public)
        """
        GeometricSource._onRequestInformation(self, *args)
        self.assignParam('color', self._vtkactor.GetProperty().SetColor)
        self._vtksource.SetBounds(self.getParam('xmin'), self.getParam('xmax'),
                                  self.getParam('ymin'), self.getParam('ymax'),
                                  self.getParam('zmin'), self.getParam('zmax'))
