[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
  [../]
[]

[Executioner]
  type = Steady
[]

[Problem]
  kernel_coverage_check = false
[]

[Outputs]
  exodus = true
[]
