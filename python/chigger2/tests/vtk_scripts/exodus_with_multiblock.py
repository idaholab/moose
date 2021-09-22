#!/usr/bin/env python
import vtk

# Input file and variable
filename = r"mug_blocks_out.e"
nodal_var = "diffused"
component = -1 # -1 = magnitude, 0=x, 1=y, 2=z

# Read Exodus Data
reader = vtk.vtkExodusIIReader()
reader.SetFileName(filename)
reader.SetPointResultArrayStatus(nodal_var, 1)
reader.UpdateInformation()
reader.SetTimeStep(5)

## Create Geometry
geometry = vtk.vtkCompositeDataGeometryFilter()
geometry.SetInputConnection(0, reader.GetOutputPort(0))

## Mapper
mapper = vtk.vtkPolyDataMapper()
mapper.SetInputConnection(geometry.GetOutputPort())
mapper.SelectColorArray(nodal_var)
mapper.SetScalarModeToUsePointFieldData()
mapper.InterpolateScalarsBeforeMappingOn()
mapper.SetUseLookupTableScalarRange(True)

## Actor
fan = vtk.vtkActor()
fan.SetMapper(mapper)

# Get the range from the actual data is the tricky part, at least in my experience. The following is
# the best method that I have found so far, there is likely a cleaner way.
reader.Update()

# The vtkMultiBlockDataSet stored by vtkExodusIIReader has 8 data blocks, each data block is
# associated with a different connectivity type. This list contains a list of enums used by VTK
# to make the linkage between the connectivity and the point/field data stored.
#
# vtkThe MultiBlockDataSet (vtkExodusIIReader::GetOutput()) is ordered according to
# vtkExodusIIReader::cont_types, which is the order used here. In my own code I have setup
# method that accepts the vtkExodusIIReader::cont_types and returns the MultiBlockIndex
#
# Block 0: vtk.vtkExodusIIReader.ELEM_BLOCK (Subdomains)
# Block 1: vtk.vtkExodusIIReader.FACE_BLOCK
# Block 2: vtk.vtkExodusIIReader.EDGE_BLOCK
# Block 3: vtk.vtkExodusIIReader.ELEM_SET
# Block 4: vtk.vtkExodusIIReader.SIDE_SET (Boundaries)
# Block 5: vtk.vtkExodusIIReader.FACE_SET
# Block 6: vtk.vtkExodusIIReader.EDGE_SET
# Block 7: vtk.vtkExodusIIReader.NODE_SET (Nodesets)
#
# Within each of these blocks there may be may more blocks depending on the mesh, for example
# the mesh used here has two subdomains.
data = reader.GetOutput().GetBlock(0)
print data.GetNumberOfBlocks() # 2

# Each subdomain has field, cell, and point data associated. Thus, for this mesh there
# to find the data range you must look through all the subdomains.
rng = [float('Inf'), -float('Inf')]
for i in range(data.GetNumberOfBlocks()):
    point_data = data.GetBlock(i).GetPointData()

    # The field, point, or cell data can have multiple arrays (i.e, variable names), so you need
    # to get the one of interest.
    array = point_data.GetAbstractArray(nodal_var)

    # Next the data range can be determined, using the component, to set the min/max values
    rng[0] = min(rng[0], array.GetRange(component)[0])
    rng[1] = max(rng[1], array.GetRange(component)[1])

# FYI, it is possible to use vtk.vtkExtractBlock to pull out potions of the file for producing
# multiple mappers from a single reader, see
# http://vtk.1045678.n5.nabble.com/How-to-extract-nodesets-and-sidesets-for-Exodus-data-td5735427.html#a5736264

## Scalebar
scalar_bar = vtk.vtkScalarBarActor()
scalar_bar.SetLookupTable(mapper.GetLookupTable())
scalar_bar.SetTitle("S")
scalar_bar.SetNumberOfLabels(4)
scalar_bar.GetLookupTable().SetRange(rng)

# create a rendering window and renderer
ren = vtk.vtkRenderer()
renWin = vtk.vtkRenderWindow()
renWin.AddRenderer(ren)

# create a renderwindowinteractor
iren = vtk.vtkRenderWindowInteractor()
iren.SetRenderWindow(renWin)

# assign actor to the renderer
ren.AddActor(fan)
ren.AddActor2D(scalar_bar)

# enable user interface interactor
iren.Initialize()
renWin.Render()
iren.Start()
