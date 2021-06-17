[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '2 2 2'
    dy = '2 2 2'
    ix = '2 2 2'
    iy = '2 2 2'
    subdomain_id = '0 1 0
                    2 5 3
                    0 4 0'
  []

  [interior_bottom]
    type = SideSetsBetweenSubdomainsGenerator
    input = cmg
    primary_block = 5
    paired_block = 1
    new_boundary = 'interior_bottom'
  []
  [interior_left]
    type = SideSetsBetweenSubdomainsGenerator
    input = interior_bottom
    primary_block = 5
    paired_block = 2
    new_boundary = 'interior_left'
  []
  [interior_right]
    type = SideSetsBetweenSubdomainsGenerator
    input = interior_left
    primary_block = 5
    paired_block = 3
    new_boundary = 'interior_right'
  []
  [interior_top]
    type = SideSetsBetweenSubdomainsGenerator
    input = interior_right
    primary_block = 5
    paired_block = 4
    new_boundary = 'interior_top'
  []
[]

[RayBCs]
  active = 'kill_internal'
  # active = 'kill_external reflect_internal'

  # for testing internal kill
  [kill_internal]
    type = KillRayBC
    boundary = 'interior_top interior_right interior_bottom interior_left'
  []

  # for testing internal reflect
  [kill_external]
    type = KillRayBC
    boundary = 'top right bottom left'
  []
  [reflect_internal]
    type = ReflectRayBC
    boundary = 'interior_top interior_right interior_bottom interior_left'
  []
[]

[UserObjects/study]
  type = RepeatableRayStudy
  start_points = '0 0 0
                  2 4 0
                  6 6 0
                  0 2.5 0
                  3 6 0
                  2.5 2.5 0'
  directions = '1 1 0
                1 -1 0
                -1 -1 0
                1 0.1 0
                0 -1 0
                0.5 1.5 0'
  names = 'to_bottom_left_corner
           at_top_left_corner
           to_top_right_corner
           to_left_offset
           to_top_center_node
           inside_to_top'
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
