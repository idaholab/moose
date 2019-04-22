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
import mooseutils
from . import GeometricSourceMeta
from ..base import ColorMap

def create(base_type):
    """
    Helper for creating PlaneSource objects.
    """

    source_type = GeometricSourceMeta.create(base_type)
    class PlaneSourceMeta(source_type):
        """
        Creates a source object containing a vtkPlaneSource object.

        This is a meta class because the vtkPlaneSource object can operate with both 2D and 3D
        actors, thus this create method should be called with either ChiggerSource or
        ChiggerSource2D as input.

        This has already been done in the "geometric" module (see geometric/__init__.py), therefore
        in your script or result objects containing geometric sources the following are available:
            plane = geometric.PlaneSource()
            plane2D = geometric.PlaneSource2D()
        """
        FILTER_TYPES = []

        @staticmethod
        def getOptions():
            opt = source_type.getOptions()
            opt.add('origin', [0, 0, 0], 'Define the origin of the plane.')
            opt.add('point1', [1, 0, 0], 'Define the first edge of the plane (origin->point1).')
            opt.add('point2', [0, 1, 0], 'Define the second edge of the plane (origin->point2).')
            opt.add('resolution', [1, 1], "Define the number of subdivisions in the x- and "
                                          "y-direction of the plane.")
            opt.add('data', None, "The VTK data to attach to the vtkMapper for this object, for "
                                  "used with the 'cmap' option.", vtype=vtk.vtkFloatArray)
            opt += ColorMap.getOptions()
            opt.setDefault('color', None)
            opt.setDefault('cmap', None)
            return opt

        def __init__(self, **kwargs):
            super(PlaneSourceMeta, self).__init__(vtkgeometric_type=vtk.vtkPlaneSource, **kwargs)

        def update(self, **kwargs):
            """
            Set the options for this cube. (public)
            """
            super(PlaneSourceMeta, self).update(**kwargs)

            if self.isOptionValid('origin'):
                self._vtksource.SetOrigin(*self.getOption('origin'))

            if self.isOptionValid('point1'):
                self._vtksource.SetPoint1(*self.getOption('point1'))

            if self.isOptionValid('point2'):
                self._vtksource.SetPoint2(*self.getOption('point2'))

            if self.isOptionValid('resolution'):
                self._vtksource.SetResolution(*self.getOption('resolution'))

            if self.isOptionValid('cmap'):
                if self.isOptionValid('color'):
                    mooseutils.mooseWarning('The "color" and "cmap" options are both being set, '
                                            'the "color" will be ignored.')

                if not self.isOptionValid('data'):
                    mooseutils.mooseError('The "cmap" option requires that "data" option also '
                                          'be supplied.')

                if self.isOptionValid('data'):
                    self._vtksource.Update()
                    data = self.getOption('data')
                    self._vtksource.GetOutput().GetCellData().SetScalars(data)
                    cmap_options = {key:self.getOption(key) for key in ['cmap', 'cmap_reverse',
                                                                        'cmap_num_colors',
                                                                        'cmap_range']}
                    self._colormap.setOptions(**cmap_options)
                    self._vtkmapper.SetScalarRange(data.GetRange(0))
                    self._vtkmapper.SetLookupTable(self._colormap())

    return PlaneSourceMeta
