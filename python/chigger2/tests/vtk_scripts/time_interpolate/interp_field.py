#!/usr/bin/env python
import vtk

file0 = 'input_out.e'
file1 = 'input_out.e-s002'
variable = 'cell_max'

##################################################################################
# FILE 0: COARSE MESH WITH SOLUTION 0

coarseReader = vtk.vtkExodusIIReader()
coarseReader.SetFileName(file0)
coarseReader.UpdateInformation()
coarseReader.SetTimeStep(0)
coarseReader.SetAllArrayStatus(vtk.vtkExodusIIReader.GLOBAL, 1)
coarseReader.Update()

coarseFieldData = coarseReader.GetOutput().GetBlock(0).GetBlock(0).GetFieldData()
coarse_array = coarseFieldData.GetArray(variable)

for i in range(coarse_array.GetNumberOfTuples()):
    print coarse_array.GetComponent(i, 0)

####################################################################################################
# FILE 1: FINE MESH WITH SOLUTION 1

fineReader = vtk.vtkExodusIIReader()
fineReader.SetFileName(file1)
fineReader.UpdateInformation()
fineReader.SetTimeStep(0)
fineReader.SetAllArrayStatus(vtk.vtkExodusIIReader.GLOBAL, 1)
fineReader.Update()

fineFieldData = fineReader.GetOutput().GetBlock(0).GetBlock(0).GetFieldData()
fine_array = fineFieldData.GetArray(variable)

for i in range(fine_array.GetNumberOfTuples()):
    print fine_array.GetComponent(i, 0)

#####################################################################################################
# INTERPOLATE BETWEEN DATA SETS

interpFieldData = vtk.vtkFieldData()
interpFieldData.DeepCopy(coarseFieldData)

for a_idx in range(coarseFieldData.GetNumberOfArrays()):
    c_array = coarseFieldData.GetArray(a_idx)
    f_array = fineFieldData.GetArray(a_idx)

    if c_array is None or f_array is None:
        continue

    for i in range(c_array.GetNumberOfTuples()):
        for j in range(c_array.GetNumberOfComponents()):
            c_value = c_array.GetComponent(i, j)
            f_value = f_array.GetComponent(i, j)
            interpFieldData.GetArray(a_idx).SetComponent(i, j, (c_value + f_value)/2.)

interp_array = interpFieldData.GetArray(variable)
print interp_array
for i in range(interp_array.GetNumberOfTuples()):
    print interp_array.GetComponent(i, 0)

#fineInterpolateAttributes = vtk.vtkInterpolateDataSetAttributes()
#fineInterpolateAttributes.AddInputData(0, coarseReader.GetOutput().GetBlock(0).GetBlock(0))
#fineInterpolateAttributes.AddInputData(0, fineReader.GetOutput().GetBlock(0).GetBlock(0))
#fineInterpolateAttributes.SetT(0.5)
#fineInterpolateAttributes.Update()

#interp_array = fineInterpolateAttributes.GetOutput().GetFieldData().GetArray(variable)
#for i in range(interp_array.GetNumberOfTuples()):
#    print interp_array.GetComponent(i, 0)
