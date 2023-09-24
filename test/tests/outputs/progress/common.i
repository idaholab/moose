[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[Variables]
  [u]
  []
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Executioner]
  type = Transient
  start_time = 10
  end_time = 20
  dt = 5
[]

[Outputs]
  progress = true
[]
