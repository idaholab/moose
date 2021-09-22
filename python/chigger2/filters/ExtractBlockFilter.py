import logging
import vtk
from moosetools import chigger
from .ChiggerFilter import ChiggerFilter

class ExtractBlockFilter(ChiggerFilter):
    VTKFILTERTYPE = vtk.vtkExtractBlock
    FILTERNAME = 'extract'

    @staticmethod
    def validParams():
        opt = ChiggerFilter.validParams()
        opt.add('indices', vtype=list,
                doc="The list of vtkMultiBlockDataSet indices to extract.")
        return opt

    def __init__(self, *args, **kwargs):
        ChiggerFilter.__init__(self, *args, **kwargs)

        self.SetNumberOfInputPorts(1)
        self.InputType = 'vtkMultiBlockDataSet'

        self.SetNumberOfOutputPorts(1)
        self.OutputType = 'vtkMultiBlockDataSet'


    def RequestData(self, request, inInfo, outInfo):
        self.debug('RequestData')

        inp = inInfo[0].GetInformationObject(0).Get(vtk.vtkDataObject.DATA_OBJECT())
        opt = outInfo.GetInformationObject(0).Get(vtk.vtkDataObject.DATA_OBJECT())

        extract_indices = self.getParam('indices')
        if not extract_indices:
            msg = "The 'indices' option was empty for the ExtractBlockFilter, " \
                  "this filter is being bypassed."
            self.error(msg)
            opt.ShallowCopy(inp)

        else:
            self._vtkfilter.RemoveAllIndices()
            for index in extract_indices:
                self._vtkfilter.AddIndex(index)

            self._vtkfilter.SetInputData(inp)
            self._vtkfilter.Update()
            opt.ShallowCopy(self._vtkfilter.GetOutput())

        return 1
