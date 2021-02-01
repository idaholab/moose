[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 1
  []
[]

[UserObjects/study]
  type = RepeatableRayStudyBaseTest
  names = 'dummy'
  start_points = '0 0 0'
  directions = '1 0 0'
  ray_kernel_coverage_check = false
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
