[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    ny = 1
    xmax = 1
    ymax = 1
  []
[]

[UserObjects/study]
  type = RepeatableRayStudy
  start_points = '0 0 0'
  directions = '1 1 0'
  names = ray
[]

[RayKernels/null]
  type = NullRayKernel
[]

[RayBCs]
  active = ''
  [top]
    type = NullRayBC
    boundary = top
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
