[Mesh]
  type = MFEMFileMesh
  file = ../../mesh/mug.e
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

[Variables]
  [h1_scalar]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[VectorPostprocessors]
  [point_sample]
    type = MFEMPointValueSampler
    variable = 'h1_scalar'
    points = '0 0 1000'
  []
[]
