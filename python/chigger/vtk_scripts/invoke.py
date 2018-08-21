#!/usr/bin/env python
#
import vtk
#print '\n'.join(dir(vtk))

# Input file and variable
filename = '../tests/input/mug_blocks_out.e'
nodal_var = 'convected'

TIMESTEP_CHANGED = vtk.vtkCommand.UserEvent + 1

@vtk.calldata_type(vtk.VTK_INT)
def callback(reader, event, timestep):
    print 'Timestep = ', timestep

# Read Exodus Data
reader = vtk.vtkExodusIIReader()
creader.AddObserver(TIMESTEP_CHANGED, callback)
reader.SetFileName(filename)
reader.UpdateInformation()
reader.SetTimeStep(10)
reader.InvokeEvent(TIMESTEP_CHANGED, 10)
reader.SetAllArrayStatus(vtk.vtkExodusIIReader.NODAL, 1)  # enables all NODAL variables
reader.Update()
