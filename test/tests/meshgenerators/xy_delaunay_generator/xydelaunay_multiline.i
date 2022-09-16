[Mesh]
  [line1]
    type = PolyLineMeshGenerator
    points = '0.0 0.0 0.0
              1.0 0.0 0.0'
  []
  [line2]
    type = PolyLineMeshGenerator
    points = '1.0 0.0 0.0
              0.0 1.0 0.0'
  []
  [outer_bdy]
    type = StitchedMeshGenerator
    inputs = "line1 line2"
    stitch_boundaries_pairs = '2 1'
  []
  [triang]
    type = XYDelaunayGenerator
    boundary = 'outer_bdy'
    add_nodes_per_boundary_segment = 4
    refine_boundary = false
    desired_area = 0.05
  []
[]
