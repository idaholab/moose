"""chigger filter module"""
import vtk

from ContourFilter import ContourFilter
from TransformFilter import TransformFilter
from TubeFilter import TubeFilter

from ClipperFilterBase import ClipperFilterBase
from PlaneClipper import PlaneClipper
from BoxClipper import BoxClipper

from ChiggerFilterBase import ChiggerFilterBase
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
