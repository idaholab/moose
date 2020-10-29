[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
    xmax = 5
    ymax = 5
    elem_type = QUAD4
  []

  [middle_block]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    block_id = 1
    bottom_left = '2 0 0'
    top_right = '3 5 0'
  []

  [middle_block_sideset]
    type = SideSetsBetweenSubdomainsGenerator
    input = middle_block
    primary_block = 1
    paired_block = 0
    new_boundary = 7
  []
[]

[RayBCs]
  [kill_top_right]
    type = 'KillRayBC'
    boundary = 'top right'
    rays = 'to_top_right
            centroid_left_to_right
            along_top'
  []
  [kill_bottom_left]
    type = 'KillRayBC'
    boundary = 'left bottom'
    rays = 'reflect_right_and_top
            reflect_right_at_node
            reflect_internal'
  []
  [reflect]
    type = 'ReflectRayBC'
    boundary = 'top right'
    rays = 'reflect_right_and_top
            reflect_right_at_node'
  []
  [reflect_internal]
    type = 'ReflectRayBC'
    boundary = 7
    rays = 'reflect_internal'
  []
  [kill_internal]
    type = 'KillRayBC'
    boundary = 7
    rays = 'kill_internal'
  []
  [nothing_internal]
    type = 'NullRayBC'
    boundary = 7
    rays = 'to_top_right
            centroid_left_to_right
            along_top
            reflect_right_and_top
            reflect_right_at_node'
  []
[]

[UserObjects/study]
  type = RepeatableRayStudy
  ray_kernel_coverage_check = false
  start_points = '0 0 0
                  0.5 2.6 0
                  0 5 0
                  0 0.23 0
                  3 0 0
                  0 2.25 0
                  4.8 0.2 0'
  directions = '1 1 0
                1 0 0
                1 0 0
                1 0.6 0
                1 0.5 0
                1 0.58 0
                -1 0.2 0'
  names = 'to_top_right
           centroid_left_to_right
           along_top
           reflect_right_and_top
           reflect_right_at_node
           reflect_internal
           kill_internal'
  execute_on = initial
  always_cache_traces = true
  use_internal_sidesets = true
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
