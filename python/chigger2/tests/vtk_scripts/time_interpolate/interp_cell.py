#!/usr/bin/env python
import vtk

file0 = 'input_out.e'
file1 = 'input_out.e-s002'
variable = 'cell'
range = [0, 3]

##################################################################################
# FILE 0: COARSE MESH WITH SOLUTION 0

coarseReader = vtk.vtkExodusIIReader()
coarseReader.SetFileName(file0)
coarseReader.UpdateInformation()
coarseReader.SetTimeStep(0)
coarseReader.SetAllArrayStatus(vtk.vtkExodusIIReader.ELEM_BLOCK, 1)
coarseReader.Update()

coarseGeometry = vtk.vtkCompositeDataGeometryFilter()
coarseGeometry.SetInputConnection(0, coarseReader.GetOutputPort(0))
coarseGeometry.Update()

coarseMapper = vtk.vtkPolyDataMapper()
coarseMapper.SetInputConnection(coarseGeometry.GetOutputPort())
coarseMapper.SelectColorArray(variable)
coarseMapper.SetScalarModeToUseCellFieldData()
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
fineReader.SetAllArrayStatus(vtk.vtkExodusIIReader.ELEM_BLOCK, 1)
fineReader.Update()

fineGeometry = vtk.vtkCompositeDataGeometryFilter()
fineGeometry.SetInputConnection(0, fineReader.GetOutputPort(0))
fineGeometry.Update()

#fineGeometry.GetOutput().GetCellData().SetActiveScalars(variable)


fineMapper = vtk.vtkPolyDataMapper()
fineMapper.SetInputConnection(fineGeometry.GetOutputPort())
fineMapper.SelectColorArray(variable)
fineMapper.SetScalarModeToUseCellFieldData()
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

# (1) Run vtkCellCenters on the t=1 (fine) mesh. The result is a "mesh" made only of vertices placed
# at cell centers whose PointData arrays are actually CellData values from the input (t=1, fine)
# mesh. Call the output of this filter mesh B.
fineCenters = vtk.vtkCellCenters()
fineCenters.SetInputConnection(fineGeometry.GetOutputPort())
fineCenters.Update()

locator = vtk.vtkStaticPointLocator()
locator.SetDataSet(fineCenters.GetOutput())
locator.BuildLocator()

kernel = vtk.vtkGaussianKernel()
kernel.SetSharpness(4)
kernel.SetKernelFootprintToNClosest()
kernel.SetNumberOfPoints(10)
kernel.SetSharpness(5.0)

# (2) Run vtkPointInterpolator using mesh B as the "input" and mesh A (t=0 coarse) as the "source",
# having it pass _cell_ arrays and not point arrays. The result is a mesh made of vertex cells with
# cell arrays from time the t=0 (coarse) mesh. Call this result mesh C.
fineInterpolator = vtk.vtkPointInterpolator()
fineInterpolator.SetInputData(fineCenters.GetOutput())
fineInterpolator.SetSourceData(coarseGeometry.GetOutput())
fineInterpolator.SetKernel(kernel)
fineInterpolator.SetLocator(locator)
#fineInterpolator.SetNullPointsStrategyToClosestPoint()
fineInterpolator.PassCellArraysOff()
fineInterpolator.Update()

fineInterpolatorMapper = vtk.vtkDataSetMapper()
fineInterpolatorMapper.SetInputConnection(fineInterpolator.GetOutputPort())
fineInterpolatorMapper.SetScalarModeToUsePointFieldData()
fineInterpolatorMapper.SelectColorArray(variable)
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


# (3) Run vtkCellCenters on mesh C from step 2 above. The "center" of a vertex cell is simply the
# vertex itself, so the net effect of this pass is to convert the CellData arrays into PointData
# arrays. Call the output mesh D.
fineInterpolatorCenters = vtk.vtkCellCenters()
fineInterpolatorCenters.SetInputConnection(fineInterpolator.GetOutputPort())
fineInterpolatorCenters.Update()
print fineInterpolatorCenters.GetOutputPort()

# (4) Run vtkInterpolateDataSetAttributes to generate a t=0.5 tween between meshes B and D. This
# filter runs in "point-data" mode (vtkCellCenters has been used to convert cells into points at both
# time steps, so this interpolates cell data values that now appear as point data). Call the output
# mesh E.
fineInterpolateAttributes = vtk.vtkInterpolateDataSetAttributes()
fineInterpolateAttributes.AddInputData(0, fineCenters.GetOutput())
fineInterpolateAttributes.AddInputData(0, fineInterpolatorCenters.GetOutput())
fineInterpolateAttributes.SetT(0.5)
fineInterpolateAttributes.Update()

# (5) Finally, create a shallow copy of mesh G (t=1, fine) named mesh F (i.e.,
# F.ShallowCopy(G)). Then overwrite its cell data with mesh E's point data by calling
# F.GetCellData().ShallowCopy(E.GetPointData()).
fineCopy = vtk.vtkMultiBlockDataSet()
fineCopy.ShallowCopy(fineReader.GetOutput())
fineCopy.GetBlock(0).GetBlock(0).GetCellData().ShallowCopy(fineInterpolateAttributes.GetOutput().GetPointData())

# THESE ARE REQUIRED!!!
fineCopy.GetBlock(0).GetBlock(0).GetCellData().SetActiveScalars(variable)
#fineInterpolator.GetOutput().GetCellData().SetActiveScalars(variable)

fineInterpolatorGeometry = vtk.vtkCompositeDataGeometryFilter()
fineInterpolatorGeometry.SetInputData(0, fineCopy)
fineInterpolatorGeometry.Update()

fineInterpolateAttibutesMapper = vtk.vtkPolyDataMapper()
fineInterpolateAttibutesMapper.SetInputConnection(fineInterpolatorGeometry.GetOutputPort(0))
fineInterpolateAttibutesMapper.SelectColorArray(variable)
fineInterpolateAttibutesMapper.SetScalarModeToUseCellFieldData()
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
