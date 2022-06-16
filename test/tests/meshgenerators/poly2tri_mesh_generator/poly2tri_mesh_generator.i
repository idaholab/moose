[Mesh]
  [./outer_bdy]
    type = PolyLineMeshGenerator
    points = '-1.0 0.0 0.0
              0.0 -1.0 0.0
              1.0 0.0 0.0
              0.0 2.0 0.0'
    loop = true
  []
  [./triang]
    type = Poly2TriMeshGenerator
    boundary = 'outer_bdy'
    add_nodes_per_boundary_segment = 4
    refine_boundary = false
    desired_area = 0.05
  []

  parallel_type = replicated  # libMesh bug workaround
[]

[Outputs]
  exodus = true
[]
