[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Reporters]
  [day]
    type = ConstantReporter
    integer_names = day
    integer_values = 24
  []
  [month]
    type = ConstantReporter
    integer_names = month
    integer_values = 6
    outputs = none
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  [out]
    type = JSON
  []
[]
