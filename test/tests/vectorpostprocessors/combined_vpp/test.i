[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 1
    dx = 1
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[VectorPostprocessors]
  [vpp1]
    type = ConstantVectorPostprocessor
    vector_names = 'v1 v2 v3'
    value = '1 2 3; 4 5; 7 10 11 12'
  []
  [vpp2]
    type = ConstantVectorPostprocessor
    vector_names = 'v5 v6'
    value = '1 11 13; 21 23 1'
  []
  [combined]
    type = CombinedVectorPostprocessor
    vectorpostprocessors = 'vpp1 vpp2'
  []
[]

[Outputs]
  csv = true
[]
