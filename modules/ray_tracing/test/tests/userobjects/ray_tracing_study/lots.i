[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
  []
[]

[UserObjects/lots]
  type = LotsOfRaysRayStudy
  ray_kernel_coverage_check = false # no need for RayKernels
  execute_on = initial
[]

[RayBCs/kill]
  type = KillRayBC
  boundary = 'top left right bottom'
[]

[Postprocessors/total_distance]
  type = RayTracingStudyResult
  study = lots
  result = total_distance
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[Outputs]
  csv = true
  exodus = false
[]
