[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 2
  []
[]

[UserObjects]
  active = 'study'
  [study]
    type = RayTracingStudyTest
    ray_kernel_coverage_check = false
  []
  [repeatable]
    type = RepeatableRayStudy
    start_points = '0 0 0'
    directions = '1 0 0'
    names = 'ray'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
