[Mesh]
  type = MFEMMesh
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
    # this point is considered found
    # unless tolerance is increased, then it is outside the mesh
    points = '0.01 0.01 -2.37501'
  []
[]
