[Mesh/gmg]
  type = GeneratedMeshGenerator
  dim = 1
  nx = 10
[]

[RayBCs/periodic]
  type = PeriodicRayBC
  auto_direction = x
[]

[UserObjects/study]
  type = RepeatableRayStudy
  start_points = '0 0 0
                  1 0 0'
  directions = '1 0 0
                -1 0 0'
  max_distances = '25 18'
  ray_kernel_coverage_check = false
  names = 'left_to_right
           right_to_left'
[]

[Postprocessors/total_distance]
  type = RayTracingStudyResult
  study = study
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
[]
