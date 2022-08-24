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
  [left_2d]
    type = Poly2TriMeshGenerator
    boundary = 'left_bdy'
  []
  [right_2d]
    type = Poly2TriMeshGenerator
    boundary = 'right_bdy'
    output_boundary = 'right_outer'
  []
  [both_2d]
    type = CombinerGenerator
    inputs = 'left_2d right_2d'
  []
  [triang]
    type = Poly2TriMeshGenerator
    boundary = 'both_2d'
    input_boundary_names = 'right_outer' # only the right half
    refine_boundary = true
    desired_area = 0.2
  []
[]
