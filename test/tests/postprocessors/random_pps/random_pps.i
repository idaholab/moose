[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
[]

[Variables]
  [./u]
  [../]
[]

[Problem]
  solve = false
[]

[Postprocessors]
  [./random_pps]
    type = RandomPostprocessor
    seed = 1
    generator = 2
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 10
[]

[Outputs]
  csv = true
[]
