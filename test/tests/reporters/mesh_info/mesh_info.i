[Mesh]
  parallel_type = DISTRIBUTED

  [generate]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    bias_x = 0.9
    bias_y = 1.1
  []

  [add_block]
    type = ParsedSubdomainMeshGenerator
    input = generate
    combinatorial_geometry = 'y < 0.5'
    block_id = 1
    block_name = foo
  []

  # For consistent partitioning across platforms
  [Partitioner]
    type = GridPartitioner
    grid_computation = 'automatic'
  []
[]

[Adaptivity]
  initial_marker = marker
  [Markers/marker]
    type = BoxMarker
    bottom_left = '0 0 0'
    top_right = '1 0.5 0'
    inside = 'refine'
    outside = 'do_nothing'
  []
[]

[Variables/u]
[]

[Executioner]
  type = Steady
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Reporters/mesh_info]
  type = MeshInfo
[]

[Outputs]
  [out]
    type = JSON
    execute_system_information_on = 'NONE'
  []
[]
