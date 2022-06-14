[Mesh]
  [./outer_bdy]
    type = PolyLineMeshGenerator
    points = '-1.0 0.0 0.0
              0.0 -1.0 0.0
              1.0 0.0 0.0
              0.0 2.0 0.0'
    loop = true
  []
  [./hole_1]
    type = PolyLineMeshGenerator
    points = '-0.5 -0.1 0.0
              -0.3 -0.1 0.0
              -0.3 0.1 0.0
              -0.5 0.1 0.0'
    loop = true
  []
  [./hole_2]
    type = PolyLineMeshGenerator
    points = '0.3 -0.1 0.0
              0.5 -0.1 0.0
              0.5 0.1 0.0
              0.3 0.1 0.0'
    loop = true
  []
  [./triang]
    type = Poly2TriMeshGenerator
    boundary = 'outer_bdy'
    holes = 'hole_1
             hole_2'
    interpolate_boundary = 4
    refine_boundary = false
    desired_area = 0.05
  []
[]

[Outputs]
  exodus = true
[]
