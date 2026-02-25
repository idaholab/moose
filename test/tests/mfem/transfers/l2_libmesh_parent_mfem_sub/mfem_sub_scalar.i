[Mesh]
  type = MFEMMesh
  file = ../../mesh/square_quad.e
[]

[Problem]
  type = MFEMProblem
  solve = false
[]

[FESpaces]
  [L2FESpace]
    type = MFEMScalarFESpace
    fec_type = L2
    fec_order = CONSTANT
  []
[]

[Variables]
  [temperature]
    type = MFEMVariable
    fespace = L2FESpace
  []
[]

[Functions]
  [parsed_function]
    type = ParsedFunction
    expression = 'x*x + y*y'
  []
[]

[ICs]
  [libmesh_scalar_var_ic]
    type = MFEMScalarIC
    variable = 'temperature'
    coefficient = parsed_function
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]
