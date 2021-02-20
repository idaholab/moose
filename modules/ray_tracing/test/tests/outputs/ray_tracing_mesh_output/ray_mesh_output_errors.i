[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[RayBCs/kill]
  type = 'KillRayBC'
  boundary = 'left right'
[]

[UserObjects/study]
  type = RepeatableRayStudy
  start_points = '0 0 0'
  directions = '1 0 0'
  names = 'ray'
  ray_kernel_coverage_check = false
  execute_on = INITIAL
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[Outputs]
  exodus = false
  [rays]
    type = RayTracingExodus
    study = study
    execute_on = final
  []
[]
