#!/usr/bin/env python
import os
import vtk

specifiedTime = 1.5

# Input file and variable
filename = os.path.abspath('../tests/input/input_out.e')
nodal_var = 'u'

# time = 0, timestep=0
reader0 = vtk.vtkExodusIIReader()
reader0.SetFileName(filename)
reader0.UpdateInformation()
reader0.SetTimeStep(0)
reader0.SetAllArrayStatus(vtk.vtkExodusIIReader.NODAL, 1)

geometry0 = vtk.vtkCompositeDataGeometryFilter()
geometry0.SetInputConnection(0, reader0.GetOutputPort(0))
geometry0.Update()

mapper0 = vtk.vtkPolyDataMapper()
mapper0.SetInputConnection(geometry0.GetOutputPort())
mapper0.SelectColorArray(nodal_var)
mapper0.SetScalarRange(1, 3)
mapper0.SetScalarModeToUsePointFieldData()
mapper0.InterpolateScalarsBeforeMappingOn()

actor0 = vtk.vtkActor()
actor0.SetMapper(mapper0)
actor0.GetProperty().SetEdgeVisibility(True)

renderer0 = vtk.vtkRenderer()
renderer0.SetViewport([0, 0, 0.33, 1])
renderer0.AddActor(actor0)

#camera0 = renderer0.GetActiveCamera()
#camera0.SetViewUp(0.115004, 0.0763775, 0.990424)
#camera0.SetFocalPoint(0, 0, 0.125)
#camera0.SetDistance(23)
#renderer0.ResetCameraClippingRange()

# time = 1, timestep=1
reader2 = vtk.vtkExodusIIReader()
reader2.SetFileName(filename)
reader2.UpdateInformation()
reader2.SetTimeStep(1)
reader2.SetAllArrayStatus(vtk.vtkExodusIIReader.NODAL, 1)

geometry2 = vtk.vtkCompositeDataGeometryFilter()
geometry2.SetInputConnection(0, reader2.GetOutputPort(0))
geometry2.Update()

mapper2 = vtk.vtkPolyDataMapper()
mapper2.SetInputConnection(geometry2.GetOutputPort())
mapper2.SelectColorArray(nodal_var)
mapper2.SetScalarRange(1, 3)
mapper2.SetScalarModeToUsePointFieldData()
mapper2.InterpolateScalarsBeforeMappingOn()

actor2 = vtk.vtkActor()
actor2.SetMapper(mapper2)
actor2.GetProperty().SetEdgeVisibility(True)

renderer2 = vtk.vtkRenderer()
renderer2.SetViewport([0.66, 0, 1, 1])
renderer2.AddActor(actor2)


# Read Exodus Data
reader = vtk.vtkExodusIIReader()
reader.SetFileName(filename)
reader.UpdateInformation()
#reader.SetTimeStep(10)
reader.SetAllArrayStatus(vtk.vtkExodusIIReader.NODAL, 1)
#reader.Update(); print reader

#info = reader.GetExecutive().GetOutputInformation().GetInformationObject(0)
#key = vtk.vtkStreamingDemandDrivenPipeline.TIME_STEPS()
#times = np.array([info.Get(key,i) for i in range(info.Length(key))])
#print times
#
#index = np.max(np.where(times <= specifiedTime))
#print index

#extractTime = vtk.vtkExtractTimeSteps()
#extractTime.SetInputConnection(0, reader.GetOutputPort(0))
#extractTime.SetTimeStepIndices(2, [index, index+1])

# Time interpolation (How do I set this up?)
time = vtk.vtkTemporalInterpolator()
time.SetInputConnection(0, reader.GetOutputPort(0))
#time.SetInputConnection(0, reader.GetOutputPort(0))
#time.SetDiscreteTimeStepInterval(times[index+1] - times[index])
time.UpdateTimeStep(specifiedTime)

#print time.GetNumberOfOutputPorts()

#extractTime2 = vtk.vtkExtractTimeSteps()
#extractTime2.SetInputConnection(0, time.GetOutputPort(0))
#extractTime2.AddTimeStepIndex(1)

# Create Geometry
geometry = vtk.vtkCompositeDataGeometryFilter()
#geometry.SetInputConnection(0, extractTime2.GetOutputPort(0))
geometry.SetInputConnection(0, time.GetOutputPort(0))
geometry.Update()

#writer = vtk.vtkExodusIIWriter()
#writer.SetInputConnection(0, geometry.GetOutputPort())
#writer.SetFileName('time.e')
#writer.WriteAllTimeStepsOn()
#writer.Write()

# Mapper
mapper = vtk.vtkPolyDataMapper()
mapper.SetInputConnection(geometry.GetOutputPort())
mapper.SelectColorArray(nodal_var)
mapper.SetScalarRange(1, 3)
mapper.SetScalarModeToUsePointFieldData()
mapper.InterpolateScalarsBeforeMappingOn()

# Actor
actor = vtk.vtkActor()
actor.SetMapper(mapper)
actor.GetProperty().SetEdgeVisibility(True)

# Renderer
renderer = vtk.vtkRenderer()
renderer.AddActor(actor)
renderer.SetViewport([0.33, 0, 0.66, 1])



# Window and Interactor
window = vtk.vtkRenderWindow()
window.AddRenderer(renderer0)
window.AddRenderer(renderer)
window.AddRenderer(renderer2)
window.SetSize(1200, 600)

interactor = vtk.vtkRenderWindowInteractor()
interactor.SetRenderWindow(window)
interactor.Initialize()

# Show the result
window.Render()
interactor.Start()
