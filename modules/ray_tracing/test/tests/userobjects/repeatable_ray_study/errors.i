[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[UserObjects/study]
  type = RepeatableRayStudy
  names = 'ray'
  start_points = '0 0 0'
[]

[RayKernels/null]
  type = NullRayKernel
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
