[Mesh]
  type = MFEMMesh
  file = ../../mesh/mug.e
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
    basis = GaussLegendre
  []
[]

[Variables]
  [l2_scalar]
    type = MFEMVariable
    fespace = L2FESpace
  []
[]

[Functions]
  [parsed_function]
    type = ParsedFunction
    expression = 'x + y*y + z*z*z'
  []
[]

[ICs]
  [l2_scalar_ic]
    type = MFEMScalarIC
    variable = 'l2_scalar'
    coefficient = parsed_function
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[VectorPostprocessors]
  [point_sample]
    type = MFEMPointValueSampler
    variable = 'l2_scalar'
    # this is a point very close to an internal element face
    points = '0.1 1e-13 -2.3125'
  []
[]
