[Mesh]
  [shape]
    type = PolyLineMeshGenerator
    points = '0.0 0.0 0.0
              1.0 1.0 0.0
              0.5 1.5 0.0
              2.0 1.5 0.0
              2.0 -1.0 0.0
              0.5 -1.0 0.0
              0.0 -2.0 0.0
              -0.5 -1.0 0.0
              -2.0 -1.0 0.0
              -2.0 1.5 0.0
              -0.5 1.5 0.0
              -1.0 1.0 0.0'
    loop = true
  []
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = -3.0
    xmax = 3.0
    ymin = -3.0
    ymax = 3.0
    elem_type = QUAD4
  []
  [xyd]
    type = XYDelaunayGenerator
    boundary = 'gmg'
    holes = 'shape'
    refine_boundary = false
    refine_holes = false
  []
  [gap]
    type = GapLineMeshGenerator
    input = 'xyd'
    thickness = 0.1
    boundary_ids = '1'
    gap_direction = INWARD
  []
[]
