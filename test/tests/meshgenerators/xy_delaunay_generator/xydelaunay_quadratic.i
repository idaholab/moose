[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = accg_one_layer_quadratic.e
  []
  [ext]
    type = PolyLineMeshGenerator
    points = '-4.0 0.0 0.0
              0.0 -4.0 0.0
              4.0 0.0 0.0
              0.0 4.0 0.0'
    loop = true
  []
  [xyd]
    type = XYDelaunayGenerator
    boundary = 'ext'
    holes = 'fmg'
    stitch_holes = 'true'
    refine_holes = 'false'
    verify_holes = 'false'
    add_nodes_per_boundary_segment = 2
    refine_boundary = true
    desired_area = 1.0
    tri_element_type = TRI6
  []
[]
