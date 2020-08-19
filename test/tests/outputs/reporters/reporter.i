[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables/u]
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Reporters]
  [values]
    type = TestDeclareReporter
  []
[]

[Postprocessors]
  [numdofs]
    type = NumDOFs
  []
[]

[Executioner]
  type = Transient
  num_steps = 3
[]

[Outputs]
  csv = true
[]
