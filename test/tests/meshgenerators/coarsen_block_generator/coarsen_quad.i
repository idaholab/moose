[Mesh]
  allow_renumbering = false
  [big_one]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    ny = 1
    xmin = 1
    xmax = 2
    ymin = 0
    ymax = 1
  []
  [cut_one]
    type = CartesianMeshGenerator
    dim = 2
    dx = 1
    dy = 1
    ix = 2
    iy = 2
  []
  [cmbn]
    type = CombinerGenerator
    inputs = 'big_one cut_one'
  []
  [coarsen]
    type = CoarsenBlockGenerator
    input = cmbn
    block = 0
    coarsening = 1
    starting_point = '0.25 0.25 0'
  []

  # Stitch now as the coarsening does not stitch
  [stitch]
    type = MeshRepairGenerator
    input = coarsen
    fix_node_overlap = true
  []
[]

[Outputs]
  exodus = true
[]
