[Problem]
  type = MFEMProblem
  solve = false
[]

[Mesh]
  type = MFEMFileMesh
  file = base_strip.e
[]

[FESpaces]
  [H1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = FIRST
  []
[]

[AuxVariables]
  [indicator_field]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[Functions]
  [background]
    type = ParsedFunction
    expression = '1+x*x'
  []
[]

[ICs]
  [background_ic]
    type = MFEMScalarIC
    coefficient = background
    variable = indicator_field
  []
[]

[Executioner]
  type = MFEMTransient
  dt = 0.05
  num_steps = 10
  device = cpu
[]
