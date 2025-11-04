[Mesh]
  parallel_type = DISTRIBUTED

  [generate]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
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
  []
[]
