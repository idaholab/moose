[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 3
[]

[Reporters]
  [test]
    type = ConstantReporter
    integer_names = 'year'
    integer_values = '1980'
    execute_on = INITIAL
  []
[]

[Outputs]
  [out]
    type = JSON
    one_file_per_timestep = true
  []
[]
