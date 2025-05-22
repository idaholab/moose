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

[Reporters/reporter]
  type = ConstantReporter
  real_names = 'value1 value2 value3'
  real_values = '1 2 3'
[]

[Outputs]
  [specific]
    type = JSON
    reporters = 'reporter/value1 reporter/value3'
  []
  json = true
[]
