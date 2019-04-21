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
class TransformFilter(ChiggerFilterBase):
    """
    Filter for computing and visualizing contours.
    """

    @staticmethod
    def getOptions():
        opt = ChiggerFilterBase.getOptions()
        opt.add('scale', [1, 1, 1], "The scale to apply in the x, y, z coordinate dimensions.")
        opt.add('translate', [0, 0, 0],
                "The translation to apply the x, y, z coordinate dimensions.")
        opt.add('rotate', [0, 0, 0], "Rotation to apply about the x, y, and z axis.")
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

        if self.isOptionValid('translate'):
            translate = self.getOption('translate')
            self.__transform.Translate(translate)

        if self.isOptionValid('rotate'):
            rot = self.getOption('rotate')
            self.__transform.RotateX(rot[0])
            self.__transform.RotateY(rot[1])
            self.__transform.RotateZ(rot[2])
