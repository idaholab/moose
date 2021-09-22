#!/usr/bin/env python
import os
import vtk
from vtk.util.vtkAlgorithm import VTKPythonAlgorithmBase

from chigger import utils


class ExodusReader(ChiggerAlgorithm, VTKPythonAlgorithmBase):
    """I am building a tool for Exodus files, it will be doing all sorts of helpful tasks..."""

    @staticmethod
    def validParams():
        opt = ChiggerAlgorithm.validParams()
        #opt.add('time', vtype=(int, float),
        #        doc="The time to view, if not specified the last timestep is displayed.")
        opt.add("timestep", default=-1, vtype=int,
                doc="The simulation timestep. (Use -1 for latest.)")
        #opt.add("adaptive", default=True, vtype=bool,
        #        doc="Load adaptive files (*.e-s* files).")

        return opt

    def __init__(self, filename, **kwargs):
        ChiggerAlgorithm.__init__(self, **kwargs)
        VTKPythonAlgorithmBase.__init__(self, nInputPorts=0, nOutputPorts=1,
                                        outputType='vtkMultiBlockDataSet')

        self.__filename = filename
        self.__active = None

        #for opt in self._parameters.itervalues():
        #    opt._Option__modified.Modified()


        self.__reader0 = vtk.vtkExodusIIReader()






    def RequestData(self, request, inInfo, outInfo):
        print 'RequestData'

        #self.__reader0.SetFileName(self.__filename)
        #self.__reader0.UpdateInformation()

        self.__reader0.SetTimeStep(self.getParam('timestep'))
        self.__reader0.SetAllArrayStatus(vtk.vtkExodusIIReader.NODAL, 1) # enables all NODAL variables
        self.__reader0.Update()

        out_data = outInfo.GetInformationObject(0).Get(vtk.vtkDataObject.DATA_OBJECT())
        out_data.ShallowCopy(self.__reader0.GetOutput())
        return 1

    def RequestInformation(self, request, inInfo, outInfo):
        print 'RequestInformation'

        # Create list of filenames to consider
        #filenames = self.__getActiveFilenames()
        #if filenames == self.__active:
        #    return
        #self.__active = filenames



        self.__reader0.SetFileName(self.__filename)
        self.__reader0.UpdateInformation()
        #self.__reader0.SetTimeStep(10)
        return 1


    #def __getActiveFilenames(self):
    #    """
    #    The active ExodusII file(s). (private)

    #    Returns:
    #        list: Contains tuples (filename, modified time) of active file(s).
    #    """
    #    if self.isValid('adaptive'):
    #        return utils.get_active_filenames(self.__filename, self.__filename + '-s*')
    #    else:
    #        return utils.get_active_filenames(self.__filename)




if __name__ == '__main__':E
    # Input file and variable
    filename = 'mug_blocks_out.e'
    nodal_var = 'convected'

    reader = ExodusReader(filename, timestep=10)

    # Create Geometry
    geometry = vtk.vtkCompositeDataGeometryFilter()
    geometry.SetInputConnection(0, reader.GetOutputPort(0))

    # Mapper
    mapper = vtk.vtkPolyDataMapper()
    mapper.SetInputConnection(geometry.GetOutputPort())
    mapper.SelectColorArray(nodal_var) # when I used the VTKPytonAlgorithm this doesn't get applied
    mapper.SetScalarModeToUsePointFieldData()
    mapper.InterpolateScalarsBeforeMappingOn()

    # Actor
    actor = vtk.vtkActor()
    actor.SetMapper(mapper)

    # Renderer
    renderer = vtk.vtkRenderer()
    renderer.AddViewProp(actor)

    # Window and Interactor
    window = vtk.vtkRenderWindow()
    window.AddRenderer(renderer)
    window.SetSize(600, 600)

    interactor = vtk.vtkRenderWindowInteractor()
    interactor.SetRenderWindow(window)
    interactor.Initialize()

    # Show the result
    window.Render()
    reader.update(timestep=10)
    window.Render()
    interactor.Start()
