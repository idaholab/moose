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


def create(base_type):
    source_type = GeometricSourceMeta.create(base_type)
    class OutlineSourceMeta(source_type):
        """
        Source for outline a result object.
        """

        @staticmethod
        def validOptions():
            opt = source_type.validOptions()
            opt.add('line_width', vtype=float, doc="The line width for the outline.")
            return opt

        def __init__(self, input_source, **kwargs):
            super(OutlineSourceMeta, self).__init__(vtkgeometric_type=vtk.vtkOutlineSource, **kwargs)
            self._input_source = input_source

        def update(self, **kwargs):
            super(OutlineSourceMeta, self).update(**kwargs)

            bnds = self._input_source.getBounds()
            self._vtksource.SetBounds(*bnds)

            if self.isOptionValid('line_width'):
                self._vtkactor.GetProperty().SetLineWidth(self.applyOption('line_width'))

            #xmin, xmax = self._input_source.getBounds()
            #print xmin, xmax
            #self._vtksource.SetBounds(xmin[0], xmax[0], xmin[1], xmax[1], xmin[2], xmax[2])
            #if not self._vtksource.GetNumberOfInputConnections(0):
            #self._vtksource.SetInputConnection(self._input_source.getVTKMapper().GetInputConnection(0,0))
    return OutlineSourceMeta
