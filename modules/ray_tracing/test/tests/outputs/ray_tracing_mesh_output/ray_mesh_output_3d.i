[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 5
    ny = 5
    nz = 5
    xmax = 5
    ymax = 5
    zmax = 5
    elem_type = HEX8
  []

  [middle_block]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    block_id = 1
    bottom_left = '2 0 0'
    top_right = '3 5 5'
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
  [kill]
    type = 'KillRayBC'
    boundary = 'top right front left'
    rays = 'to_top_right
            along_edge
            within'
  []
  [kill_left]
    type = 'KillRayBC'
    boundary = 'left'
    rays = 'reflect_three_times
            reflect_at_nodes
            reflect_internal'
  []
  [reflect]
    type = 'ReflectRayBC'
    boundary = 'back right top'
    rays = 'reflect_three_times
            reflect_at_nodes'
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
            reflect_three_times
            reflect_at_nodes'
  []
[]

[UserObjects/study]
  type = RepeatableRayStudy
  ray_kernel_coverage_check = false
  start_points = '0 0 0
                  0 5 0
                  0.6 0.6 0
                  0 1.2 0.8
                  3 0 1
                  0 1.3 2.5
                  5 0 2'
  directions = '1 1 1
                0 0 1
                0 0 1
                1 0.6 -0.4
                2 2 1
                0.8 0.5 0.4
                -1 1 1'
  names = 'to_top_right
           along_edge
           within
           reflect_three_times
           reflect_at_nodes
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
