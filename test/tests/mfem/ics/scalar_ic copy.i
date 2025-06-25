[Mesh]
  type = MFEMMesh
  file = ../mesh/mug.e
[]

[Problem]
  type = MFEMProblem
  solve = false
[]

[Functions]
  [height]
    type = ParsedFunction
    expression = 'z'
  []  
[]

[FESpaces]
  [H1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = FIRST
  []
  [L2FESpace]
    type = MFEMScalarFESpace
    fec_type = L2
    fec_order = CONSTANT
  []
[]

[Variables]
  [h1_scalar]
    type = MFEMVariable
    fespace = H1FESpace
    [./InitialCondition]
      type = MFEMScalarIC
      coefficient = height
    [../]
  []
  [l2_scalar]
    type = MFEMVariable
    fespace = L2FESpace
  []  
[]

[ICs]
  [l2_scalar_ic]
    type = MFEMScalarIC
    variable = l2_scalar
    coefficient = 2.0
  []
[]

[Preconditioner]
  [boomeramg]
    type = MFEMHypreBoomerAMG
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/ScalarIC
    vtk_format = ASCII
  []
[]
