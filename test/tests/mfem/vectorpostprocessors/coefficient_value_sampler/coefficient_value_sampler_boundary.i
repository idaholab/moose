[Mesh]
  type = MFEMMesh
  file = two_block_quad.mesh
[]

[Problem]
  type = MFEMProblem
  solve = false
[]

[FunctorMaterials]
  [left]
    type = MFEMGenericFunctorMaterial
    block = 1
    prop_names = piecewise_coefficient
    prop_values = 10
  []
  [right]
    type = MFEMGenericFunctorMaterial
    block = 2
    prop_names = piecewise_coefficient
    prop_values = 10
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[VectorPostprocessors]
  [boundary_sample]
    type = MFEMPointScalarCoefficientValueSampler
    coefficient = piecewise_coefficient
    points = '0.5 0.5 0'
  []
[]

[Outputs]
  csv = true
[]
