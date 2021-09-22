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
import GeometricSourceMeta
from .. import base, filters

BaseType = GeometricSourceMeta.create(base.ChiggerSource)
class LineSource(BaseType):
    """
    Single LineSource object.
    """
    FILTER_TYPES = [filters.TubeFilter]

    @staticmethod
    def validParams():
        """
        Return the default options for this object.
        """
        opt = BaseType.validParams()
        opt.add('resolution', default=100, vtype=int,
                doc="The number of points sampled over the line.")
        opt.add('point1', default=(0, 0, 0), size=3, vtype=(int, float),
                doc="The starting point of the line.")
        opt.add('point2', default=(1, 1, 0), size=3, vtype=(int, float),
                doc="The ending point of the line.")
        opt.add('data', vtype=list,
                doc="Values to assign to the line, if used the resolution will be "
                     "set to match the number of points.")
        return opt

    def __init__(self, **kwargs):
        super(LineSource, self).__init__(vtkgeometric_type=vtk.vtkLineSource, **kwargs)
        self._data = vtk.vtkDoubleArray()
        self._data.SetName("data")

    def update(self, **kwargs):
        """
        Set the options for this cube. (public)
        """
        super(LineSource, self).update(**kwargs)

        if self.isValid('resolution'):
            self._vtksource.SetResolution(self.applyOption('resolution'))

        if self.isValid('point1'):
            self._vtksource.SetPoint1(self.applyOption('point1'))

        if self.isValid('point2'):
            self._vtksource.SetPoint2(self.applyOption('point2'))

        if self.isValid('data'):
            data = self.getParam('data')
            n = len(data)
            self._vtksource.SetResolution(n-1)
            self._vtksource.Update()
            self._data.SetNumberOfTuples(n)
            for i, d in enumerate(data):
                self._data.SetValue(i, d)
            self._vtksource.GetOutput().GetPointData().AddArray(self._data)
            self._vtkmapper.SelectColorArray(self._data.GetName())
            self._vtkmapper.SetScalarRange(self._data.GetRange(0))
            self._vtkmapper.SetScalarModeToUsePointFieldData()

            cmap_params = {key:self.getParam(key) for key in ['cmap', 'cmap_reverse',
                                                              'cmap_num_colors',
                                                              'cmap_range']}
            self._colormap.setParams(**cmap_params)
            self._vtkmapper.SetLookupTable(self._colormap())
