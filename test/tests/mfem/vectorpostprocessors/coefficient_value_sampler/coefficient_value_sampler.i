[Mesh]
  type = MFEMMesh
  file = ../../mesh/inline-quad.mesh
[]

[Problem]
  type = MFEMProblem
  solve = false
[]

[Functions]
  [linear_coefficient]
    type = MFEMParsedFunction
    expression = 'x + 2*y'
  []
[]

[FunctorMaterials]
  [material]
    type = MFEMGenericFunctorMaterial
    prop_names = material_coefficient
    prop_values = linear_coefficient
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[VectorPostprocessors]
  [function_sample]
    type = MFEMPointScalarCoefficientValueSampler
    coefficient = linear_coefficient
    points = '0.125 0.125 0
              0.375 0.625 0
              0.875 0.875 0'
  []
  [material_sample]
    type = MFEMPointScalarCoefficientValueSampler
    coefficient = material_coefficient
    points = '0.125 0.125 0
              0.375 0.625 0
              0.875 0.875 0'
  []
[]

[Outputs]
  csv = true
[]
