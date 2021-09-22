#!/usr/bin/env python
import vtk

file0 = 'input_out.e'
file1 = 'input_out.e-s002'
variable = 'point'
range = [0, 3]

##################################################################################
# FILE 0: COARSE MESH WITH SOLUTION 0

coarseReader = vtk.vtkExodusIIReader()
coarseReader.SetFileName(file0)
coarseReader.UpdateInformation()
coarseReader.SetTimeStep(0)
coarseReader.SetAllArrayStatus(vtk.vtkExodusIIReader.NODAL, 1)
coarseReader.Update()

coarseGeometry = vtk.vtkCompositeDataGeometryFilter()
coarseGeometry.SetInputConnection(0, coarseReader.GetOutputPort(0))
coarseGeometry.Update()

coarseMapper = vtk.vtkPolyDataMapper()
coarseMapper.SetInputConnection(coarseGeometry.GetOutputPort())
coarseMapper.SelectColorArray(variable)
coarseMapper.SetScalarModeToUsePointFieldData()
coarseMapper.InterpolateScalarsBeforeMappingOn()
coarseMapper.SetScalarRange(*range)

coarseActor = vtk.vtkActor()
coarseActor.SetMapper(coarseMapper)
coarseActor.GetProperty().SetEdgeVisibility(True)

coarseRenderer = vtk.vtkRenderer()
coarseRenderer.AddViewProp(coarseActor)
coarseRenderer.SetViewport(0, 0, 0.25, 1)

####################################################################################################
# FILE 1: FINE MESH WITH SOLUTION 1

fineReader = vtk.vtkExodusIIReader()
fineReader.SetFileName(file1)
fineReader.UpdateInformation()
fineReader.SetTimeStep(0)
fineReader.SetAllArrayStatus(vtk.vtkExodusIIReader.NODAL, 1)
fineReader.Update()

fineGeometry = vtk.vtkCompositeDataGeometryFilter()
fineGeometry.SetInputConnection(0, fineReader.GetOutputPort(0))
fineGeometry.Update()

fineGeometry.GetOutput().GetPointData().SetActiveScalars(variable)


fineMapper = vtk.vtkPolyDataMapper()
fineMapper.SetInputConnection(fineGeometry.GetOutputPort())
fineMapper.SelectColorArray(variable)
fineMapper.SetScalarModeToUsePointFieldData()
fineMapper.InterpolateScalarsBeforeMappingOn()
fineMapper.SetScalarRange(*range)

fineActor = vtk.vtkActor()
fineActor.SetMapper(fineMapper)
fineActor.GetProperty().SetEdgeVisibility(True)

fineRenderer = vtk.vtkRenderer()
fineRenderer.AddViewProp(fineActor)
fineRenderer.SetViewport(0.75, 0, 1, 1)

####################################################################################################
# PROJECT SOLUTION FROM FILE 0 to GRID FROM FILE 1

# Build the structure to interpolate onto
#fineInterpolatedGrid = vtk.vtkUnstructuredGrid()
#fineMultiBlock = vtk.vtkMultiBlockDataSet.SafeDownCast(fineReader.GetOutput().GetBlock(0))
#fineInterpolatedGrid.DeepCopy(vtk.vtkUnstructuredGrid.SafeDownCast(fineMultiBlock.GetBlock(0)))


#TODO: Create an algorithm that has this as an output...
interp = vtk.vtkMultiBlockDataSet()#vtk.vtkExodusIIReader()
interp.DeepCopy(fineReader.GetOutput())

locator = vtk.vtkStaticPointLocator()
locator.SetDataSet(fineGeometry.GetOutput())
locator.BuildLocator()

kernel = vtk.vtkGaussianKernel()
kernel.SetSharpness(4)
kernel.SetKernelFootprintToNClosest()
kernel.SetNumberOfPoints(10)
kernel.SetSharpness(5.0)

fineInterpolator = vtk.vtkPointInterpolator()
fineInterpolator.SetSourceData(coarseGeometry.GetOutput()) # Pc data set to be probed by input points P
fineInterpolator.SetInputData(fineGeometry.GetOutput())
fineInterpolator.SetKernel(kernel)
fineInterpolator.SetLocator(locator)
fineInterpolator.SetNullPointsStrategyToClosestPoint()
fineInterpolator.PassPointArraysOff() # THIS IS REQUIRED!!!
fineInterpolator.Update()

fineInterpolatorMapper = vtk.vtkDataSetMapper()
fineInterpolatorMapper.SetInputConnection(fineInterpolator.GetOutputPort())
fineInterpolatorMapper.SelectColorArray(variable)
fineInterpolatorMapper.SetScalarModeToUsePointFieldData()
fineInterpolatorMapper.InterpolateScalarsBeforeMappingOn()
fineInterpolatorMapper.SetScalarRange(*range)

fineInterpolatorActor = vtk.vtkActor()
fineInterpolatorActor.SetMapper(fineInterpolatorMapper)
fineInterpolatorActor.GetProperty().SetEdgeVisibility(True)

fineInterpolatorRenderer = vtk.vtkRenderer()
fineInterpolatorRenderer.AddActor(fineInterpolatorActor)
fineInterpolatorRenderer.SetViewport(0.25, 0, 0.5, 1)

#####################################################################################################
# INTERPOLATE BETWEEN THE PROJECTED SOLUTION AND SOLUTION FROM FILE 1

# THESE ARE REQUIRED!!!
#fineInterpolatedGrid.GetPointData().SetActiveScalars(variable)
interp.GetBlock(0).GetBlock(0).GetPointData().SetActiveScalars(variable)
fineInterpolator.GetOutput().GetPointData().SetActiveScalars(variable)

# TODO: This needs to loop over all active data, interpolate, and copy data into vtkMultiBlockDataSet

fineInterpolateAttributes = vtk.vtkInterpolateDataSetAttributes()
#fineInterpolateAttributes.AddInputData(0, fineInterpolatedGrid)
fineInterpolateAttributes.AddInputData(0, interp.GetBlock(0).GetBlock(0))
fineInterpolateAttributes.AddInputData(0, fineInterpolator.GetOutput())
fineInterpolateAttributes.SetT(0.5)
fineInterpolateAttributes.Update()

#print fineReader.GetOutput().GetBlock(0).GetBlock(0)
#print interp.GetBlock(0).GetBlock(0)
interp.GetBlock(0).GetBlock(0).DeepCopy(fineInterpolateAttributes.GetOutput())



fineInterpolatorGeometry = vtk.vtkCompositeDataGeometryFilter()
fineInterpolatorGeometry.SetInputData(0, interp)
fineInterpolatorGeometry.Update()

fineInterpolateAttibutesMapper = vtk.vtkPolyDataMapper()
fineInterpolateAttibutesMapper.SetInputConnection(fineInterpolatorGeometry.GetOutputPort(0))

#fineInterpolateAttibutesMapper = vtk.vtkDataSetMapper()
#fineInterpolateAttibutesMapper.SetInputConnection(fineInterpolateAttributes.GetOutputPort())
fineInterpolateAttibutesMapper.SelectColorArray(variable)
fineInterpolateAttibutesMapper.SetScalarModeToUsePointFieldData()
fineInterpolateAttibutesMapper.InterpolateScalarsBeforeMappingOn()
fineInterpolateAttibutesMapper.SetScalarRange(*range)

fineInterpolateAttibutesActor = vtk.vtkActor()
fineInterpolateAttibutesActor.SetMapper(fineInterpolateAttibutesMapper)
fineInterpolateAttibutesActor.GetProperty().SetEdgeVisibility(True)

fineInterpolateAttibutesMapperRenderer = vtk.vtkRenderer()
fineInterpolateAttibutesMapperRenderer.AddActor(fineInterpolateAttibutesActor)
fineInterpolateAttibutesMapperRenderer.SetViewport(0.5, 0, 0.75, 1)




####################################################################################################
# Window and Interactor

window = vtk.vtkRenderWindow()
window.AddRenderer(fineRenderer)
window.AddRenderer(fineInterpolatorRenderer)
window.AddRenderer(fineInterpolateAttibutesMapperRenderer)
window.AddRenderer(coarseRenderer)
window.SetSize(1920, 1080)

interactor = vtk.vtkRenderWindowInteractor()
interactor.SetRenderWindow(window)
interactor.Initialize()

window.Render()
interactor.Start()
