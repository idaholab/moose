[Mesh]
  # A healthy triangle ...
  [healthy]
    type = ElementGenerator
    nodal_positions = '0 0 0
                       2 0 0
                       1 1 0'
    element_connectivity = '0 1 2'
    elem_type = TRI3
  []
  # ... and a sliver triangle sharing its base edge (0,0)-(2,0): a thin flap whose apex (1,-0.01)
  # sits just below the long edge. The duplicated base nodes are merged by 'fix_node_overlap'.
  [sliver]
    type = ElementGenerator
    input = healthy
    nodal_positions = '0 0 0
                       1 -0.01 0
                       2 0 0'
    element_connectivity = '0 1 2'
    elem_type = TRI3
  []
  [repair]
    type = MeshRepairGenerator
    input = sliver
    fix_node_overlap = true
    fix_sliver_triangles = true
  []
[]
