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
from .ChiggerFilter import ChiggerFilter
class Transform(ChiggerFilter):

    VTKFILTERTYPE = vtk.vtkTransformFilter
    #FILTERNAME = 'transform'


    @staticmethod
    def validParams():
        opt = ChiggerFilter.validParams()
        opt.add('scale', default=(1, 1, 1), vtype=(int, float), size=3,
                doc="The scale to apply in the x, y, z coordinate dimensions.")
        opt.add('translate', default=(0, 0, 0), vtype=(int, float), size=3,
                doc="The translation to apply the x, y, z coordinate dimensions.")
        opt.add('rotate', default=(0, 0, 0), vtype=(int, float), size=3,
                doc="Rotation to apply about the x, y, and z axis.")
        return opt

    def __init__(self, *args, **kwargs):
        ChiggerFilter.__init__(self, *args, **kwargs)

        self.SetNumberOfInputPorts(1)
        self.InputType = 'vtkMultiBlockDataSet'

        self.SetNumberOfOutputPorts(1)
        self.OutputType = 'vtkPolyData'


    def update(self, **kwargs):
        """
        Computes the contour levels for the vtkContourFilter.
        """
        super(Transform, self).update(**kwargs)

        if self.isOptinValid('scale'):
            inverse = self.__transform.GetInverse().GetScale()
            scale = self.applyOption('scale')
            scale = [scale[i]*inverse[i] for i in range(len(scale))]
            self.__transform.Scale(scale)

        if self.isParamValid('translate'):
            translate = self.applyOption('translate')
            self.__transform.Translate(translate)

        if self.isParamValid('rotate'):
            rot = self.applyOption('rotate')
            self.__transform.RotateX(rot[0])
            self.__transform.RotateY(rot[1])
            self.__transform.RotateZ(rot[2])
