[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 2
  []
[]

[UserObjects/study]
  type = TestRay
  execute_on = initial
  ray_kernel_coverage_check = false
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]
