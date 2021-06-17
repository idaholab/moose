[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    xmax = 6
    nx = 6
  []

  [central_block]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    block_id = 1
    bottom_left = '2 0 0'
    top_right = '4 0 0'
  []

  [central_boundary]
    type = SideSetsBetweenSubdomainsGenerator
    input = central_block
    primary_block = 1
    paired_block = 0
    new_boundary = 7
  []
[]

[RayBCs]
  active = 'kill_internal'
  # active = 'kill_external reflect_internal'

  # for testing internal kill
  [kill_internal]
    type = KillRayBC
    boundary = 7
  []

  # for testing internal reflect
  [kill_external]
    type = KillRayBC
    boundary = 'left right'
  []
  [reflect_internal]
    type = ReflectRayBC
    boundary = 7
  []
[]

[UserObjects/study]
  type = RepeatableRayStudy
  start_points = '0 0 0
                  2 0 0
                  6 0 0
                  4 0 0
                  3 0 0'
  directions = '1 0 0
                1 0 0
                -1 0 0
                -1 0 0
                -1 0 0'
  names = 'left_in at_left right_in at_right inside_left'
  ray_distance = 10
  execute_on = initial
  ray_kernel_coverage_check = false
  use_internal_sidesets = true
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
  exodus = false
  csv = true
[]
