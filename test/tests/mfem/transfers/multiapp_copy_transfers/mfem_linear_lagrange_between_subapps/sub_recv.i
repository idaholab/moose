[Mesh]
  type = MFEMMesh
  file = ../../../../../../unit/data/square.msh
  dim = 3
[]

[Problem]
  type = MFEMProblem
  solve = false
[]

[FESpaces]
  [H1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = FIRST
  []
[]

[AuxVariables]
  [recv]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/DiffusionRecvApp
    vtk_format = ASCII
  []
[]
