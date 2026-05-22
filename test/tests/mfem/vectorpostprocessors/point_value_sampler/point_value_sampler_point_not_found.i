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

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[VectorPostprocessors]
  [point_sample]
    type = MFEMPointValueSampler
    variable = 'l2_scalar'
    points = '0 0 1000'
  []
[]
