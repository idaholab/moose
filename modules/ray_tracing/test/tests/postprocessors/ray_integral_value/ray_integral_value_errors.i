[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[UserObjects/study]
  type = RepeatableRayStudy
  start_points = '0 0 0'
  directions = '1 1 1'
  names = ray
  ray_kernel_coverage_check = false
[]

[RayBCs/kill]
  type = KillRayBC
  boundary = right
  study = study
  rays = ray
[]

[RayKernels]
  active = ''

  [null]
    type = NullRayKernel
    rays = ray
  []
[]

[Postprocessors]
  active = ''

  [not_integral_ray_kernel]
    type = RayIntegralValue
    ray_kernel = null
    ray = ray
  []
  [not_found]
    type = RayIntegralValue
    ray_kernel = dummy
    ray = ray
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
