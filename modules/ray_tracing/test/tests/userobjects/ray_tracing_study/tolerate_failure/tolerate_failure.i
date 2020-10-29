[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
  []
[]

[UserObjects/study]
  type = RepeatableRayStudy
  start_points = '0 0 0
                  0 0 0'
  directions = '1 0 0
                1 0 0'
  names = 'ray0 ray1'
  ray_kernel_coverage_check = false
  tolerate_failure = true
[]

[RayBCs]
  [kill]
    type = KillRayBC
    rays = 'ray1'
    boundary = 'right'
  []
  [null]
    type = NullRayBC
    rays = 'ray0'
    boundary = 'right'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
