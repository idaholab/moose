[Mesh]
  [outer_bdy]
    type = PolyLineMeshGenerator
    points = '0.0 0.0 0.0
              2.0 0.0 0.0
              2.0 1.0 0.0
              1.0 1.0 0.0
              1.0 2.0 0.0
              0.0 2.0 0.0'
    loop = true
  []
  [triang]
    type = XYFrontalDelaunayGenerator
    boundary = 'outer_bdy'
    refine_boundary = true
    desired_area = 0.02
    output_subdomain_name = 'triangles'
  []
[]
