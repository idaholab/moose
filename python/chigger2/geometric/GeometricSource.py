import vtk
from .. import base


class GeometricSource(base.ChiggerSource):
    VTKMAPPERTYPE = vtk.vtkPolyDataMapper

    #: The underlying VTK type, this should be set by the child class.
    VTKSOURCETYPE = None

    @staticmethod
    def validParams():
        opt = base.ChiggerSource.validParams()
        return opt

    def __init__(self, *args, **kwargs):
        self._vtksource = self.VTKSOURCETYPE()
        base.ChiggerSource.__init__(self, *args,
                                    nOutputPorts=1, outputType='vtkPolyData',
                                    **kwargs)

    def _onRequestData(self, inInfo, outInfo):
        base.ChiggerSource._onRequestData(self, inInfo, outInfo)
        opt = outInfo.GetInformationObject(0).Get(vtk.vtkDataObject.DATA_OBJECT())
        self._vtksource.Update()
        opt.ShallowCopy(self._vtksource.GetOutput())

class GeometricSource2D(base.ChiggerSource2D):
    VTKMAPPERTYPE = vtk.vtkPolyDataMapper2D

    #: The underlying VTK type, this should be set by the child class.
    VTKSOURCETYPE = None

    @staticmethod
    def validParams():
        opt = base.ChiggerSource2D.validParams()
        return opt

    def __init__(self, *args, **kwargs):
        self._vtksource = self.VTKSOURCETYPE()
        base.ChiggerSource2D.__init__(self, *args,
                                    nOutputPorts=1, outputType='vtkPolyData',
                                    **kwargs)

    def _onRequestInformation(self, *args):
        base.ChiggerSource2D._onRequestInformation(self, *args)
        if self._vtkmapper.GetTransformCoordinate() is None:
            self._vtkmapper.SetTransformCoordinate(vtk.vtkCoordinate())
        self._vtkmapper.GetTransformCoordinate().SetCoordinateSystemToNormalizedViewport()

    def _onRequestData(self, inInfo, outInfo):
        base.ChiggerSource2D._onRequestData(self, inInfo, outInfo)
        opt = outInfo.GetInformationObject(0).Get(vtk.vtkDataObject.DATA_OBJECT())
        self._vtksource.Update()
        opt.ShallowCopy(self._vtksource.GetOutput())
