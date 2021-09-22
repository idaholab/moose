import vtk
import logging
from .ChiggerObject import ChiggerObjectBase
from vtk.util.vtkAlgorithm import VTKPythonAlgorithmBase

class ChiggerAlgorithm(ChiggerObjectBase, VTKPythonAlgorithmBase):
    """
    A base class for objects that require options and are a part of the VTK pipeline.

    Objects inheriting from this are VTK objects, as such have all the associated methods. However,
    it is intended that those methods are for internal use. When creating a ChiggerAlgorithm object
    two methods should be overridden. Both methods are automatically invoked by the VTK pipeline,
    when needed.

    _onRequestInformation(self, inInfo, outInfo): This is the chigger version of RequestInformation
        and should be called manually using updateInformation (notice the case of the leading letter)

    _onRequestData(self, inInfo, outInfo): This is the chigger version of RequestData
        and should be called manually using updateData (notice the case of the leading letter)

    For help with the VTK pipeline see https://blog.kitware.com/a-vtk-pipeline-primer-part-1/
    """

    def __init__(self, nInputPorts=0, nOutputPorts=0, outputType=None, inputType=None, **kwargs):
        ChiggerObjectBase.__init__(self, **kwargs)
        VTKPythonAlgorithmBase.__init__(self)

        self.SetNumberOfInputPorts(nInputPorts)
        self.SetNumberOfOutputPorts(nOutputPorts)
        if outputType is not None:
            self.OutputType = outputType
        if inputType is not None:
            self.InputType = inputType

        # Set the VTK modified time, this is needed to make sure the options for this class are all
        # older than the class itself.
        self.Modified()

    def update(self):
        """
        Manually update the algorithm information and data.

        NOTE: Calling this or the other update methods directly should NOT be necessary, the objects
        within chigger should be calling this automatically. However, if it is needed this is the
        version that should be used in scripts. Also, please contact the developers for your use case
        and perhaps the call could be automated.
        """
        self.debug('update')
        self.UpdateInformation()
        self.Update()

    def updateInformation(self):
        """
        Manually update the algorithm information.

        See NOTE in ChiggerAlgorithm.update
        """
        self.debug('updateInformation')
        self.UpdateInformation()

    def updateData(self):
        """
        Manually update the algorithm data.

        See NOTE in ChiggerAlgorithm.update
        """
        self.debug('updateData')
        self.Update()

    def setParam(self, *args, **kwargs):
        """
        Set the supplied option, if anything changes mark the class as modified for VTK.

        See ChiggerObjectBase.setParam
        """
        ChiggerObjectBase.setParam(self, *args, **kwargs)
        if self._parameters.modified() > self.GetMTime():
            self.debug('setParam::Modified')
            self.Modified()

    def setParams(self, *args, **kwargs):
        """
        Set the supplied options, if anything changes mark the class as modified for VTK.

        See ChiggerObjectBase.setParams
        """
        ChiggerObjectBase.setParams(self, *args, **kwargs)
        if self._parameters.modified() > self.GetMTime():
            self.debug('setParams::Modified')
            self.Modified()

    def _onRequestInformation(self, inInfo, outInfo):
        """
        Override this to update the VTK algorithm information
        """
        self.debug('_onRequestInformation')

    def _onRequestData(self, inInfo, outInfo):
        """
        Override this to update the VTK algorithm data
        """
        self.debug('_onRequestData')

    def RequestInformation(self, request, inInfo, outInfo):
        """
        Override VTK method to call _onRequestInformation and handle retcode automatically

        This is an internal method not intended for general use.
        """
        self.debug('RequestInformation')
        retcode = self._onRequestInformation(inInfo, outInfo)
        if retcode is None:
            retcode = 1
        return retcode

    def RequestData(self, request, inInfo, outInfo):
        """
        Override VTK method to call _onRequestData and handle retcode automatically

        This is an internal method not intended for general use.
        """
        self.debug('RequestData')
        retcode = self._onRequestData(inInfo, outInfo)
        if retcode is None:
            retcode = 1
        return retcode
