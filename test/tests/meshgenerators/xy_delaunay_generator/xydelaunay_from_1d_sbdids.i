[Mesh]
  [left_bdy]
    type = PolyLineMeshGenerator
    points = '-3.0 0.0 0.0
              -2.0 -1.0 0.0
              -1.0 0.0 0.0
              -2.0 2.0 0.0'
    loop = true
  []
  [right_bdy]
    type = PolyLineMeshGenerator
    points = '3.0 0.0 0.0
              2.0 -1.0 0.0
              1.0 0.0 0.0
              2.0 2.0 0.0'
    loop = true
  []
  [right_sbd1]
    type = SubdomainIDGenerator
    input = right_bdy
    subdomain_id = 1
  []
  [both_bdy]
    type = CombinerGenerator
    inputs = 'left_bdy right_sbd1'
  []
  [triang]
    type = XYDelaunayGenerator
    boundary = 'both_bdy'
    input_subdomain_names = 1 # only the right half
    refine_boundary = true
    desired_area = 0.2
  []
[]
