[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
  []
[]

[UserObjects/study]
  type = RepeatableRayStudy
  start_points = '0 0 0'
  directions = '1 0 0'
  names = 'ray'
  execute_on = PRE_KERNELS
[]

[Variables/u]
[]

[RayKernels/line_source]
  type = ADLineSourceRayKernel
  variable = u
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
