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

from .ContourFilter import ContourFilter
from .TransformFilter import TransformFilter
from .TubeFilter import TubeFilter
from .RotationalExtrusionFilter import RotationalExtrusionFilter

from .ClipperFilterBase import ClipperFilterBase
from .PlaneClipper import PlaneClipper
from .BoxClipper import BoxClipper

from .ChiggerFilterBase import ChiggerFilterBase
def create_basic_filter(vtkfilter_type):
    """
    Function for creating meta filter objects.
    """
    class ChiggerMetaFilter(ChiggerFilterBase):
        """
        Meta object for generating chigger filter objects.
        """
        def __init__(self, **kwargs):
            super(ChiggerMetaFilter, self).__init__(vtkfilter_type=vtkfilter_type, **kwargs)
    return ChiggerMetaFilter

GeometryFilter = create_basic_filter(vtk.vtkCompositeDataGeometryFilter)
IdFilter = create_basic_filter(vtk.vtkIdFilter)
CellCenters = create_basic_filter(vtk.vtkCellCenters)
SelectVisiblePoints = create_basic_filter(vtk.vtkSelectVisiblePoints)
CompositeDataProbeFilter = create_basic_filter(vtk.vtkCompositeDataProbeFilter)
