[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 3
    dx = '2 2 2'
    dy = '2 2 2'
    dz = '2 2 2'
    ix = '2 2 2'
    iy = '2 2 2'
    iz = '2 2 2'
    subdomain_id = '0 0 0
                    0 1 0
                    0 0 0

                    0 2 0
                    3 7 4
                    0 5 0

                    0 0 0
                    0 6 0
                    0 0 0'
  []

  [interior_back]
    type = SideSetsBetweenSubdomainsGenerator
    input = cmg
    primary_block = 7
    paired_block = 1
    new_boundary = 'interior_back'
  []
  [interior_bottom]
    type = SideSetsBetweenSubdomainsGenerator
    input = interior_back
    primary_block = 7
    paired_block = 2
    new_boundary = 'interior_bottom'
  []
  [interior_left]
    type = SideSetsBetweenSubdomainsGenerator
    input = interior_bottom
    primary_block = 7
    paired_block = 3
    new_boundary = 'interior_left'
  []
  [interior_right]
    type = SideSetsBetweenSubdomainsGenerator
    input = interior_left
    primary_block = 7
    paired_block = 4
    new_boundary = 'interior_right'
  []
  [interior_top]
    type = SideSetsBetweenSubdomainsGenerator
    input = interior_right
    primary_block = 7
    paired_block = 5
    new_boundary = 'interior_top'
  []
  [interior_front]
    type = SideSetsBetweenSubdomainsGenerator
    input = interior_top
    primary_block = 7
    paired_block = 6
    new_boundary = 'interior_front'
  []
[]

[RayBCs]
  active = 'kill_internal'
  # active = 'kill_external reflect_internal'

  # for testing internal kill
  [kill_internal]
    type = KillRayBC
    boundary = 'interior_top interior_right interior_bottom interior_left interior_front interior_back'
  []

  # for testing internal reflect
  [kill_external]
    type = KillRayBC
    boundary = 'top right bottom left front back'
  []
  [reflect_internal]
    type = ReflectRayBC
    boundary = 'interior_top interior_right interior_bottom interior_left interior_front interior_back'
  []
[]

[UserObjects/study]
  type = RepeatableRayStudy
  start_points = '0 0 0
                  2 2 2
                  6 6 6
                  4 4 4
                  0 2.5 2.5
                  3 3 6
                  2.5 0 0
                  3 3 3
                  2.5 2.5 2.5'
  directions = '1 1 1
                1 1 1
                -1 -1 -1
                -1 -1 -1
                1 0.1 0
                0 0 -1
                0 1 1
                1 1 1
                0.5 1.5 1.5'
  names = 'to_bottom_left_corner
           at_bottom_left_corner
           to_top_right_corner
           at_top_right_corner
           centroid_offset
           top_down
           left_to_edge
           inside_to_corner
           inside_offset'
  execute_on = initial
  ray_distance = 10
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
