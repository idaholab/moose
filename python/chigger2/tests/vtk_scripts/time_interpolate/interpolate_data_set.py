#!/usr/bin/env python
import vtk

filename = 'input_no_adapt_out.e'
variable = 'u'
drange = [0, 10]

reader0 = vtk.vtkExodusIIReader()
reader0.SetFileName(filename)
reader0.UpdateInformation()
reader0.SetTimeStep(0)
reader0.SetAllArrayStatus(vtk.vtkExodusIIReader.NODAL, 1)
reader0.Update()

data0 = reader0.GetOutput().GetBlock(0).GetBlock(0)
print data0.GetPointData().GetAbstractArray(0).GetRange() # [0, 5]

reader1 = vtk.vtkExodusIIReader()
reader1.SetFileName(filename)
reader1.UpdateInformation()
reader1.SetTimeStep(1)
reader1.SetAllArrayStatus(vtk.vtkExodusIIReader.NODAL, 1)
reader1.Update()

data1 = reader1.GetOutput().GetBlock(0).GetBlock(0)
print data1.GetPointData().GetAbstractArray(0).GetRange() # [0, 14]

data1.GetPointData().SetActiveScalars(variable)

# How do I get this working?
interpolator = vtk.vtkInterpolateDataSetAttributes()
interpolator.AddInputData(reader0.GetOutput().GetBlock(0).GetBlock(0))
interpolator.AddInputData(reader1.GetOutput().GetBlock(0).GetBlock(0))
interpolator.SetT(0.5)
interpolator.Update()
print interpolator.GetOutput().GetPointData().GetAbstractArray(0)#.GetRange() # [0, 9.5]

# How do I get this working? This is what happens with the vtkInterpolateDataSetAttributes
n = data0.GetPointData().GetNumberOfTuples()
data = vtk.vtkPointData()
data.CopyStructure(data0.GetPointData())
data.SetNumberOfTuples(n)
#data.CopyAllOn()
#data0.GetPointData().CopyAllOn()
#data1.GetPointData().CopyAllOn()
#print data0.GetPointData()
#print data1.GetPointData()
#data.SetNumberOfTuples(n)
#data0.GetPointData().InterpolateAllocate(data)
#data1.GetPointData().InterpolateAllocate(data)
for i in xrange(data.GetNumberOfTuples()):
    data.InterpolateTime(data0.GetPointData(),
                         data1.GetPointData(),
                         i,
                         0.5)
print data.GetAbstractArray(0).GetRange() # [0, 9.5]

# THIS WORKS!
data = vtk.vtkPointData()
data.CopyStructure(data0.GetPointData())
data.SetNumberOfTuples(n)
for i in xrange(data.GetNumberOfTuples()):
    data.GetAbstractArray(0).InterpolateTuple(i,
                                              i, data0.GetPointData().GetAbstractArray(0),
                                              i, data1.GetPointData().GetAbstractArray(0),
                                              0.5)
print data.GetAbstractArray(0).GetRange() # [0, 9.5]
