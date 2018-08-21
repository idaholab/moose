#!/usr/bin/env python
#
# This example was built using Python2.7 and VTK6.3 on OSX
import vtk

# Input file and variable
filename = '../tests/input/mug_blocks_out.e'
nodal_var = 'convected'

# Read Exodus Data
reader = vtk.vtkExodusIIReader()
reader.SetFileName(filename)
reader.UpdateInformation()
reader.SetTimeStep(10)
reader.SetAllArrayStatus(vtk.vtkExodusIIReader.NODAL, 1)  # enables all NODAL variables
reader.Update()
# print reader # uncomment this to show the file information

# Create Geometry
geometry = vtk.vtkCompositeDataGeometryFilter()
geometry.SetInputConnection(0, reader.GetOutputPort(0))
geometry.Update()

# Mapper
mapper = vtk.vtkPolyDataMapper()
mapper.SetInputConnection(geometry.GetOutputPort())
mapper.SelectColorArray(nodal_var)
mapper.SetScalarModeToUsePointFieldData()
mapper.InterpolateScalarsBeforeMappingOn()

# Actor
actor = vtk.vtkActor()
actor.SetMapper(mapper)

# Renderer
renderer = vtk.vtkRenderer()
renderer.AddViewProp(actor)

outline = vtk.vtkOutlineFilter()
outline.SetInputConnection(mapper.GetInputConnection(0,0))

outlineMapper = vtk.vtkPolyDataMapper()
outlineMapper.SetInputConnection(outline.GetOutputPort())

outlineActor = vtk.vtkActor()
outlineActor.SetMapper(outlineMapper)
outlineActor.GetProperty().SetColor(1,0,0)
outlineActor.GetProperty().SetLineWidth(4)
renderer.AddActor(outlineActor)

# Window and Interactor
window = vtk.vtkRenderWindow()
window.AddRenderer(renderer)
window.SetSize(600, 600)

interactor = vtk.vtkRenderWindowInteractor()
interactor.SetRenderWindow(window)
interactor.Initialize()

# Show the result
window.Render()
interactor.Start()
