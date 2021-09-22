import math
import vtk
from .. import utils
from .GeometricSource import GeometricSource, GeometricSource2D

class HighlightBase(object):
    @staticmethod
    def validParams():
        opt = utils.ChiggerInputParameters()
        opt.add('color', vtype=(utils.Color, utils.AutoColor), doc="The color of the outline")
        opt.add("offset", 0, vtype=(int, float),
                doc="Offset percentage applied to the bounding box")
        opt.add('linewidth', 1, vtype=int,
                doc="The highlight line width (in pixels)")
        opt.add('source', required=True,
                doc='The chigger.base.ChiggerSourceBase object that shall be highlighted')
        return opt

    def _onRequestInformation(self, *args):
        if self.isParamValid('color'):
            self._vtkactor.GetProperty().SetColor(self.getParam('color').rgb())
        if self.isParamValid('linewidth'):
            self._vtkactor.GetProperty().SetLineWidth(self.getParam('linewidth'))


class Highlight(GeometricSource, HighlightBase):
    VTKSOURCETYPE = vtk.vtkOutlineCornerSource

    @staticmethod
    def validParams():
        opt = GeometricSource.validParams()
        opt += HighlightBase.validParams()
        return opt

    def __init__(self, **kwargs):
        GeometricSource.__init__(self, nInputPorts=1, inputType='vtkPolyData', **kwargs)

        src = self.getParam('source')
        self.SetInputDataObject(src.getVTKMapper().GetInput())

    def _onRequestInformation(self, *args):
        GeometricSource._onRequestInformation(self, *args)
        HighlightBase._onRequestInformation(self, *args)

    def _onRequestData(self, inInfo, outInfo):
        inObj = inInfo[0].GetInformationObject(0).Get(vtk.vtkDataObject.DATA_OBJECT())
        bnds = list(inObj.GetBounds())
        offset = self.getParam('offset')
        for i in [1,3,5]:
            dx = bnds[i] - bnds[i-1]
            bnds[i-1] = bnds[i-1] - dx * offset
            bnds[i] = bnds[i] + dx * offset
        self._vtksource.SetBounds(bnds)
        GeometricSource._onRequestData(self, inInfo, outInfo)


class Highlight2D(GeometricSource2D, HighlightBase):
    VTKSOURCETYPE = vtk.vtkOutlineCornerSource

    @staticmethod
    def validParams():
        opt = GeometricSource2D.validParams()
        opt += HighlightBase.validParams()
        return opt

    def __init__(self, **kwargs):
        GeometricSource2D.__init__(self, nInputPorts=0, inputType='vtkPolyData', **kwargs)

    def _onRequestInformation(self, *args):
        GeometricSource2D._onRequestInformation(self, *args)
        HighlightBase._onRequestInformation(self, *args)

    def _onRequestData(self, inInfo, outInfo):
        src = self.getParam('source')
        bnds = list(src.getVTKMapper().GetInput().GetBounds())

        offset = self.getParam('offset')
        for i in [1,3,5]:
            bnds[i-1] = bnds[i-1] - offset
            bnds[i] = bnds[i] + offset

        self._vtksource.SetBounds(bnds)
        GeometricSource2D._onRequestData(self, inInfo, outInfo)
