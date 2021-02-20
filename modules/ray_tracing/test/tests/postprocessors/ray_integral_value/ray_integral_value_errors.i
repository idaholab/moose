[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[AuxVariables/u]
[]

[UserObjects]
  active = 'repeatable'

  [repeatable]
    type = RepeatableRayStudy
    start_points = '0 0 0'
    directions = '1 0 0'
    names = ray
  []
  [lots]
    type = LotsOfRaysRayStudy
    ray_kernel_coverage_check = false
  []
  [no_banking_study]
    type = DisableRayBankingStudy
    start_points = '0 0 0'
    directions = '1 0 0'
    names = ray
  []
[]

[RayBCs/kill]
  type = KillRayBC
  boundary = 'left right'
[]

[RayKernels]
  active = ''

  [null]
    type = NullRayKernel
  []
  [variable_integral]
    type = VariableIntegralRayKernel
    variable = u
  []
[]

[Postprocessors]
  active = ''

  [not_integral_ray_kernel]
    type = RayIntegralValue
    ray_kernel = null
    ray = ray
  []
  [kernel_not_found]
    type = RayIntegralValue
    ray_kernel = dummy
    ray = ray
  []
  [ray_not_found]
    type = RayIntegralValue
    ray_kernel = variable_integral
    ray = dummy
  []
  [no_registration]
    type = RayIntegralValue
    ray_kernel = variable_integral
    ray = dummy
  []
  [no_banking]
    type = RayIntegralValue
    ray_kernel = variable_integral
    ray = dummy
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
