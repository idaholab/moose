[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[VectorPostprocessors]
  [const]
    type = ConstantVectorPostprocessor
    value = '1 2 3 4 5'
    outputs = none
  []
  [stats]
    type = Statistics
    vectorpostprocessors = 'const'
    compute = 'min max sum mean stddev norm2'
  []
[]

[Outputs]
  execute_on = TIMESTEP_END
  csv = true
[]
