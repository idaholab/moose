#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

#
# This example was built using Python2.7 and VTK6.3 on OSX
import vtk

# Input file and variable
filename = '../tests/input/block_vars_out.e'
varname = 'right_elemental'

# Read Exodus Data
reader = vtk.vtkExodusIIReader()
reader.SetFileName(filename)
reader.UpdateInformation()
reader.SetTimeStep(0)
reader.SetAllArrayStatus(vtk.vtkExodusIIReader.ELEM_BLOCK, 1)
reader.Update()
#print reader # uncomment this to show the file information

blk0 = reader.GetOutput().GetBlock(0).GetBlock(0)

data = vtk.vtkDoubleArray()
data.SetName(varname)
data.SetNumberOfTuples(blk0.GetCellData().GetArray(0).GetNumberOfTuples())
data.FillComponent(0, vtk.vtkMath.Nan())
blk0.GetCellData().AddArray(data)

lookup = vtk.vtkLookupTable()
lookup.SetNanColor(0.5, 0.5, 0.5, 1)
lookup.SetTableRange(0, 2)

# Create Geometry
geometry = vtk.vtkCompositeDataGeometryFilter()
geometry.SetInputConnection(0, reader.GetOutputPort(0))
geometry.Update()

# Mapper
mapper = vtk.vtkPolyDataMapper()
mapper.SetInputConnection(geometry.GetOutputPort())
mapper.SelectColorArray(varname)
mapper.SetLookupTable(lookup)
mapper.UseLookupTableScalarRangeOn()
mapper.SetScalarModeToUseCellFieldData()
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
interactor.Start()
