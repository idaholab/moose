import logging
import vtk
from vtk.util.vtkAlgorithm import VTKPythonAlgorithmBase
from moosetools import chigger

class ChiggerFilter(chigger.base.ChiggerAlgorithm):
    """Base class for filter objects that are passed into ChiggerResult objects."""

    #: The underlying VTK type, this should be set by the child class.
    VTKFILTERTYPE = None
    FILTERNAME = None

    @staticmethod
    def validParams():
        opt = chigger.base.ChiggerAlgorithm.validParams()
        return opt

    def __init__(self, **kwargs):
        kwargs['name'] = self.FILTERNAME if self.FILTERNAME is not None else self.__class__.__name__.lower()
        chigger.base.ChiggerAlgorithm.__init__(self, **kwargs)

        self._vtkfilter = self.VTKFILTERTYPE()

    def RequestData(self, request, inInfo, outInfo):
        self.debug('RequestData')

        inp = inInfo[0].GetInformationObject(0).Get(vtk.vtkDataObject.DATA_OBJECT())
        opt = outInfo.GetInformationObject(0).Get(vtk.vtkDataObject.DATA_OBJECT())

        self._vtkfilter.SetInputData(inp)
        self._vtkfilter.Update()
        opt.ShallowCopy(self._vtkfilter.GetOutput())
        return 1

    def applyParams(self):
        pass

    def __del__(self):
        self.debug('__del__()')
