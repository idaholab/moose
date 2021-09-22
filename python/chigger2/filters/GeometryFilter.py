import vtk
from .ChiggerFilter import ChiggerFilter

class GeometryFilter(ChiggerFilter):
    VTKFILTERTYPE = vtk.vtkCompositeDataGeometryFilter
    FILTERNAME = 'geometry'

    def __init__(self, *args, **kwargs):
        ChiggerFilter.__init__(self, *args, **kwargs)

        self.SetNumberOfInputPorts(1)
        self.InputType = 'vtkMultiBlockDataSet'

        self.SetNumberOfOutputPorts(1)
        self.OutputType = 'vtkPolyData'
