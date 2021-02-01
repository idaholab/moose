[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 50
  []
[]

[UserObjects/study]
  type = RayTracingStudyTest
  ray_kernel_coverage_check = false
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
